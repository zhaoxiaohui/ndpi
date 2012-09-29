/*
 * armagetron.c
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



/* include files */
#include "ipq_protocols.h"
#ifdef NDPI_PROTOCOL_ARMAGETRON


static void ndpi_int_armagetron_add_connection(struct ndpi_detection_module_struct
												 *ndpi_struct)
{

	ndpi_int_add_connection(ndpi_struct, NDPI_PROTOCOL_ARMAGETRON, NDPI_REAL_PROTOCOL);
}

void ndpi_search_armagetron_udp(struct ndpi_detection_module_struct
								  *ndpi_struct)
{
	struct ndpi_packet_struct *packet = &ndpi_struct->packet;
	struct ndpi_flow_struct *flow = ndpi_struct->flow;
//      struct ndpi_id_struct         *src=ndpi_struct->src;
//      struct ndpi_id_struct         *dst=ndpi_struct->dst;

	NDPI_LOG(NDPI_PROTOCOL_ARMAGETRON, ndpi_struct, NDPI_LOG_DEBUG, "search armagetron.\n");


	if (packet->payload_packet_len > 10) {
		/* login request */
		if (get_u32(packet->payload, 0) == htonl(0x000b0000)) {
			const u16 dataLength = ntohs(get_u16(packet->payload, 4));
			if (dataLength == 0 || dataLength * 2 + 8 != packet->payload_packet_len)
				goto exclude;
			if (get_u16(packet->payload, 6) == htons(0x0008)
				&& get_u16(packet->payload, packet->payload_packet_len - 2) == 0) {
				NDPI_LOG(NDPI_PROTOCOL_ARMAGETRON, ndpi_struct, NDPI_LOG_DEBUG, "detected armagetron.\n");
				ndpi_int_armagetron_add_connection(ndpi_struct);
				return;
			}
		}
		/* sync_msg */
		if (packet->payload_packet_len == 16 && get_u16(packet->payload, 0) == htons(0x001c)
			&& get_u16(packet->payload, 2) != 0) {
			const u16 dataLength = ntohs(get_u16(packet->payload, 4));
			if (dataLength != 4)
				goto exclude;
			if (get_u32(packet->payload, 6) == htonl(0x00000500) && get_u32(packet->payload, 6 + 4) == htonl(0x00010000)
				&& get_u16(packet->payload, packet->payload_packet_len - 2) == 0) {
				NDPI_LOG(NDPI_PROTOCOL_ARMAGETRON, ndpi_struct, NDPI_LOG_DEBUG, "detected armagetron.\n");
				ndpi_int_armagetron_add_connection(ndpi_struct);
				return;
			}
		}

		/* net_sync combination */
		if (packet->payload_packet_len > 50 && get_u16(packet->payload, 0) == htons(0x0018)
			&& get_u16(packet->payload, 2) != 0) {
			u16 val;
			const u16 dataLength = ntohs(get_u16(packet->payload, 4));
			if (dataLength == 0 || dataLength * 2 + 8 > packet->payload_packet_len)
				goto exclude;
			val = get_u16(packet->payload, 6 + 2);
			if (val == get_u16(packet->payload, 6 + 6)) {
				val = ntohs(get_u16(packet->payload, 6 + 8));
				if ((6 + 10 + val + 4) < packet->payload_packet_len
					&& (get_u32(packet->payload, 6 + 10 + val) == htonl(0x00010000)
						|| get_u32(packet->payload, 6 + 10 + val) == htonl(0x00000001))
					&& get_u16(packet->payload, packet->payload_packet_len - 2) == 0) {
					NDPI_LOG(NDPI_PROTOCOL_ARMAGETRON, ndpi_struct, NDPI_LOG_DEBUG, "detected armagetron.\n");
					ndpi_int_armagetron_add_connection(ndpi_struct);
					return;
				}
			}
		}
	}

  exclude:
	NDPI_LOG(NDPI_PROTOCOL_ARMAGETRON, ndpi_struct, NDPI_LOG_DEBUG, "exclude armagetron.\n");
	NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_ARMAGETRON);
}

#endif
