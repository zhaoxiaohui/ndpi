/*
 * applejuice.c
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
#ifdef NDPI_PROTOCOL_APPLEJUICE


static void ndpi_int_applejuice_add_connection(struct ndpi_detection_module_struct
												 *ndpi_struct)
{
	ndpi_int_add_connection(ndpi_struct, NDPI_PROTOCOL_APPLEJUICE, NDPI_REAL_PROTOCOL);
}

void ndpi_search_applejuice_tcp(struct ndpi_detection_module_struct
								  *ndpi_struct)
{
	struct ndpi_packet_struct *packet = &ndpi_struct->packet;
	struct ndpi_flow_struct *flow = ndpi_struct->flow;
//      struct ndpi_id_struct         *src=ndpi_struct->src;
//      struct ndpi_id_struct         *dst=ndpi_struct->dst;

	NDPI_LOG(NDPI_PROTOCOL_APPLEJUICE, ndpi_struct, NDPI_LOG_DEBUG, "search applejuice.\n");

	if ((packet->payload_packet_len > 7) && (packet->payload[6] == 0x0d)
		&& (packet->payload[7] == 0x0a)
		&& (ipq_mem_cmp(packet->payload, "ajprot", 6) == 0)) {
		NDPI_LOG(NDPI_PROTOCOL_APPLEJUICE, ndpi_struct, NDPI_LOG_DEBUG, "detected applejuice.\n");
		ndpi_int_applejuice_add_connection(ndpi_struct);
		return;
	}

	NDPI_LOG(NDPI_PROTOCOL_APPLEJUICE, ndpi_struct, NDPI_LOG_DEBUG, "exclude applejuice.\n");
	NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_APPLEJUICE);
}

#endif
