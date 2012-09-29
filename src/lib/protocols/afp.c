/*
 * afp.c
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
#ifdef NDPI_PROTOCOL_AFP

static void ndpi_int_afp_add_connection(struct ndpi_detection_module_struct
										  *ndpi_struct)
{
	ndpi_int_add_connection(ndpi_struct, NDPI_PROTOCOL_AFP, NDPI_REAL_PROTOCOL);
}


void ndpi_search_afp(struct ndpi_detection_module_struct *ndpi_struct)
{
	struct ndpi_packet_struct *packet = &ndpi_struct->packet;
	struct ndpi_flow_struct *flow = ndpi_struct->flow;
//  struct ndpi_id_struct *src = ndpi_struct->src;
//  struct ndpi_id_struct *dst = ndpi_struct->dst;


	/*
	 * this will detect the OpenSession command of the Data Stream Interface (DSI) protocol
	 * which is exclusively used by the Apple Filing Protocol (AFP) on TCP/IP networks
	 */
	if (packet->payload_packet_len >= 22 && get_u16(packet->payload, 0) == htons(0x0004) &&
		get_u16(packet->payload, 2) == htons(0x0001) && get_u32(packet->payload, 4) == 0 &&
		get_u32(packet->payload, 8) == htonl(packet->payload_packet_len - 16) &&
		get_u32(packet->payload, 12) == 0 && get_u16(packet->payload, 16) == htons(0x0104)) {

		NDPI_LOG(NDPI_PROTOCOL_AFP, ndpi_struct, NDPI_LOG_DEBUG, "AFP: DSI OpenSession detected.\n");
		ndpi_int_afp_add_connection(ndpi_struct);
		return;
	}

	/*
	 * detection of GetStatus command of DSI protocl
	 */
	if (packet->payload_packet_len >= 18 && get_u16(packet->payload, 0) == htons(0x0003) &&
		get_u16(packet->payload, 2) == htons(0x0001) && get_u32(packet->payload, 4) == 0 &&
		get_u32(packet->payload, 8) == htonl(packet->payload_packet_len - 16) &&
		get_u32(packet->payload, 12) == 0 && get_u16(packet->payload, 16) == htons(0x0f00)) {

		NDPI_LOG(NDPI_PROTOCOL_AFP, ndpi_struct, NDPI_LOG_DEBUG, "AFP: DSI GetStatus detected.\n");
		ndpi_int_afp_add_connection(ndpi_struct);
		return;
	}


	NDPI_LOG(NDPI_PROTOCOL_AFP, ndpi_struct, NDPI_LOG_DEBUG, "AFP excluded.\n");
	NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_AFP);
}

#endif
