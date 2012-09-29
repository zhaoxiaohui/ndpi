/*
 * thunder.c
 * Copyright (C) 2009-2011 by ipoque GmbH
 * 
 * This file is part of OpenDPI, an open source deep packet inspection
 * library based on the PACE technology by ipoque GmbH
 * 
 * OpenDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * OpenDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenDPI.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */


#include "ipq_protocols.h"
#ifdef NDPI_PROTOCOL_THUNDER

static void ndpi_int_thunder_add_connection(struct ndpi_detection_module_struct
											  *ndpi_struct, ndpi_protocol_type_t protocol_type)
{
	struct ndpi_packet_struct *packet = &ndpi_struct->packet;
	struct ndpi_id_struct *src = ndpi_struct->src;
	struct ndpi_id_struct *dst = ndpi_struct->dst;

	ndpi_int_add_connection(ndpi_struct, NDPI_PROTOCOL_THUNDER, protocol_type);

	if (src != NULL) {
		src->thunder_ts = packet->tick_timestamp;
	}
	if (dst != NULL) {
		dst->thunder_ts = packet->tick_timestamp;
	}
}


	
#if !(defined(HAVE_NTOP) && defined(WIN32))
 static inline
#else
__forceinline static
#endif
	 void ndpi_int_search_thunder_udp(struct ndpi_detection_module_struct
												 *ndpi_struct)
{
	struct ndpi_packet_struct *packet = &ndpi_struct->packet;
	struct ndpi_flow_struct *flow = ndpi_struct->flow;
//      struct ndpi_id_struct         *src=ndpi_struct->src;
//      struct ndpi_id_struct         *dst=ndpi_struct->dst;

	if (packet->payload_packet_len > 8 && packet->payload[0] >= 0x30
		&& packet->payload[0] < 0x40 && packet->payload[1] == 0 && packet->payload[2] == 0 && packet->payload[3] == 0) {
		if (flow->thunder_stage == 3) {
			NDPI_LOG(NDPI_PROTOCOL_THUNDER, ndpi_struct, NDPI_LOG_DEBUG, "THUNDER udp detected\n");
			ndpi_int_thunder_add_connection(ndpi_struct, NDPI_REAL_PROTOCOL);
			return;
		}

		flow->thunder_stage++;
		NDPI_LOG(NDPI_PROTOCOL_THUNDER, ndpi_struct, NDPI_LOG_DEBUG,
				"maybe thunder udp packet detected, stage increased to %u\n", flow->thunder_stage);
		return;
	}

	NDPI_LOG(NDPI_PROTOCOL_THUNDER, ndpi_struct, NDPI_LOG_DEBUG,
			"excluding thunder udp at stage %u\n", flow->thunder_stage);

	NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_THUNDER);
}

	
#if !(defined(HAVE_NTOP) && defined(WIN32))
 static inline
#else
__forceinline static
#endif
	 void ndpi_int_search_thunder_tcp(struct ndpi_detection_module_struct
												 *ndpi_struct)
{
	struct ndpi_packet_struct *packet = &ndpi_struct->packet;
	struct ndpi_flow_struct *flow = ndpi_struct->flow;
//      struct ndpi_id_struct         *src=ndpi_struct->src;
//      struct ndpi_id_struct         *dst=ndpi_struct->dst;

	if (packet->payload_packet_len > 8 && packet->payload[0] >= 0x30
		&& packet->payload[0] < 0x40 && packet->payload[1] == 0 && packet->payload[2] == 0 && packet->payload[3] == 0) {
		if (flow->thunder_stage == 3) {
			NDPI_LOG(NDPI_PROTOCOL_THUNDER, ndpi_struct, NDPI_LOG_DEBUG, "THUNDER tcp detected\n");
			ndpi_int_thunder_add_connection(ndpi_struct, NDPI_REAL_PROTOCOL);
			return;
		}

		flow->thunder_stage++;
		NDPI_LOG(NDPI_PROTOCOL_THUNDER, ndpi_struct, NDPI_LOG_DEBUG,
				"maybe thunder tcp packet detected, stage increased to %u\n", flow->thunder_stage);
		return;
	}

	if (flow->thunder_stage == 0 && packet->payload_packet_len > 17
		&& ipq_mem_cmp(packet->payload, "POST / HTTP/1.1\r\n", 17) == 0) {
		ipq_parse_packet_line_info(ndpi_struct);

		NDPI_LOG(NDPI_PROTOCOL_THUNDER, ndpi_struct, NDPI_LOG_DEBUG,
				"maybe thunder http POST packet detected, parsed packet lines: %u, empty line set %u (at: %u)\n",
				packet->parsed_lines, packet->empty_line_position_set, packet->empty_line_position);

		if (packet->empty_line_position_set != 0 &&
			packet->content_line.ptr != NULL &&
			packet->content_line.len == 24 &&
			ipq_mem_cmp(packet->content_line.ptr, "application/octet-stream",
						24) == 0 && packet->empty_line_position_set < (packet->payload_packet_len - 8)
			&& packet->payload[packet->empty_line_position + 2] >= 0x30
			&& packet->payload[packet->empty_line_position + 2] < 0x40
			&& packet->payload[packet->empty_line_position + 3] == 0x00
			&& packet->payload[packet->empty_line_position + 4] == 0x00
			&& packet->payload[packet->empty_line_position + 5] == 0x00) {
			NDPI_LOG(NDPI_PROTOCOL_THUNDER, ndpi_struct, NDPI_LOG_DEBUG,
					"maybe thunder http POST packet application does match\n");
			ndpi_int_thunder_add_connection(ndpi_struct, NDPI_CORRELATED_PROTOCOL);
			return;
		}
	}
	NDPI_LOG(NDPI_PROTOCOL_THUNDER, ndpi_struct, NDPI_LOG_DEBUG,
			"excluding thunder tcp at stage %u\n", flow->thunder_stage);

	NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_THUNDER);
}

	
#if !(defined(HAVE_NTOP) && defined(WIN32))
 static inline
#else
__forceinline static
#endif
	 void ndpi_int_search_thunder_http(struct ndpi_detection_module_struct
												  *ndpi_struct)
{
	struct ndpi_packet_struct *packet = &ndpi_struct->packet;
	struct ndpi_id_struct *src = ndpi_struct->src;
	struct ndpi_id_struct *dst = ndpi_struct->dst;


	if (packet->detected_protocol_stack[0] == NDPI_PROTOCOL_THUNDER) {
		if (src != NULL && ((NDPI_TIMESTAMP_COUNTER_SIZE)
							(packet->tick_timestamp - src->thunder_ts) < ndpi_struct->thunder_timeout)) {
			NDPI_LOG(NDPI_PROTOCOL_THUNDER, ndpi_struct, NDPI_LOG_DEBUG,
					"thunder : save src connection packet detected\n");
			src->thunder_ts = packet->tick_timestamp;
		} else if (dst != NULL && ((NDPI_TIMESTAMP_COUNTER_SIZE)
								   (packet->tick_timestamp - dst->thunder_ts) < ndpi_struct->thunder_timeout)) {
			NDPI_LOG(NDPI_PROTOCOL_THUNDER, ndpi_struct, NDPI_LOG_DEBUG,
					"thunder : save dst connection packet detected\n");
			dst->thunder_ts = packet->tick_timestamp;
		}
		return;
	}

	if (packet->payload_packet_len > 5
		&& memcmp(packet->payload, "GET /", 5) == 0 && NDPI_SRC_OR_DST_HAS_PROTOCOL(src, dst, NDPI_PROTOCOL_THUNDER)) {
		NDPI_LOG(NDPI_PROTOCOL_THUNDER, ndpi_struct, NDPI_LOG_DEBUG, "HTTP packet detected.\n");
		ipq_parse_packet_line_info(ndpi_struct);

		if (packet->parsed_lines > 7
			&& packet->parsed_lines < 11
			&& packet->line[1].len > 10
			&& ipq_mem_cmp(packet->line[1].ptr, "Accept: */*", 11) == 0
			&& packet->line[2].len > 22
			&& ipq_mem_cmp(packet->line[2].ptr, "Cache-Control: no-cache",
						   23) == 0 && packet->line[3].len > 16
			&& ipq_mem_cmp(packet->line[3].ptr, "Connection: close", 17) == 0
			&& packet->line[4].len > 6
			&& ipq_mem_cmp(packet->line[4].ptr, "Host: ", 6) == 0
			&& packet->line[5].len > 15
			&& ipq_mem_cmp(packet->line[5].ptr, "Pragma: no-cache", 16) == 0
			&& packet->user_agent_line.ptr != NULL
			&& packet->user_agent_line.len > 49
			&& ipq_mem_cmp(packet->user_agent_line.ptr,
						   "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)", 50) == 0) {
			NDPI_LOG(NDPI_PROTOCOL_THUNDER, ndpi_struct, NDPI_LOG_DEBUG,
					"Thunder HTTP download detected, adding flow.\n");
			ndpi_int_thunder_add_connection(ndpi_struct, NDPI_CORRELATED_PROTOCOL);
		}
	}
}

void ndpi_search_thunder(struct ndpi_detection_module_struct *ndpi_struct)
{
	struct ndpi_packet_struct *packet = &ndpi_struct->packet;
	//struct ndpi_flow_struct *flow = ndpi_struct->flow;
	//struct ndpi_id_struct *src = ndpi_struct->src;
	//struct ndpi_id_struct *dst = ndpi_struct->dst;

	if (packet->tcp != NULL) {
		ndpi_int_search_thunder_http(ndpi_struct);
		ndpi_int_search_thunder_tcp(ndpi_struct);
	} else if (packet->udp != NULL) {
		ndpi_int_search_thunder_udp(ndpi_struct);
	}
}

#endif
