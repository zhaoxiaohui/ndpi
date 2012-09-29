/*
 * flash.c
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
#ifdef NDPI_PROTOCOL_FLASH

static void ndpi_int_flash_add_connection(struct ndpi_detection_module_struct
											*ndpi_struct)
{
	ndpi_int_add_connection(ndpi_struct, NDPI_PROTOCOL_FLASH, NDPI_REAL_PROTOCOL);
}

void ndpi_search_flash(struct ndpi_detection_module_struct *ndpi_struct)
{
	struct ndpi_packet_struct *packet = &ndpi_struct->packet;
	struct ndpi_flow_struct *flow = ndpi_struct->flow;
//      struct ndpi_id_struct         *src=ndpi_struct->src;
//      struct ndpi_id_struct         *dst=ndpi_struct->dst;

	if (flow->l4.tcp.flash_stage == 0 && packet->payload_packet_len > 0
		&& (packet->payload[0] == 0x03 || packet->payload[0] == 0x06)) {
		flow->l4.tcp.flash_bytes = packet->payload_packet_len;
		if (packet->tcp->psh == 0) {
			NDPI_LOG(NDPI_PROTOCOL_FLASH, ndpi_struct, NDPI_LOG_DEBUG, "FLASH pass 1: \n");
			flow->l4.tcp.flash_stage = packet->packet_direction + 1;

			NDPI_LOG(NDPI_PROTOCOL_FLASH, ndpi_struct, NDPI_LOG_DEBUG,
					"FLASH pass 1: flash_stage: %u, flash_bytes: %u\n", flow->l4.tcp.flash_stage,
					flow->l4.tcp.flash_bytes);
			return;
		} else if (packet->tcp->psh != 0 && flow->l4.tcp.flash_bytes == 1537) {
			NDPI_LOG(NDPI_PROTOCOL_FLASH, ndpi_struct, NDPI_LOG_DEBUG,
					"FLASH hit: flash_stage: %u, flash_bytes: %u\n", flow->l4.tcp.flash_stage,
					flow->l4.tcp.flash_bytes);
			flow->l4.tcp.flash_stage = 3;
			ndpi_int_flash_add_connection(ndpi_struct);
			return;
		}
	} else if (flow->l4.tcp.flash_stage == 1 + packet->packet_direction) {
		flow->l4.tcp.flash_bytes += packet->payload_packet_len;
		if (packet->tcp->psh != 0 && flow->l4.tcp.flash_bytes == 1537) {
			NDPI_LOG(NDPI_PROTOCOL_FLASH, ndpi_struct, NDPI_LOG_DEBUG,
					"FLASH hit: flash_stage: %u, flash_bytes: %u\n", flow->l4.tcp.flash_stage,
					flow->l4.tcp.flash_bytes);
			flow->l4.tcp.flash_stage = 3;
			ndpi_int_flash_add_connection(ndpi_struct);
			return;
		} else if (packet->tcp->psh == 0 && flow->l4.tcp.flash_bytes < 1537) {
			NDPI_LOG(NDPI_PROTOCOL_FLASH, ndpi_struct, NDPI_LOG_DEBUG,
					"FLASH pass 2: flash_stage: %u, flash_bytes: %u\n", flow->l4.tcp.flash_stage,
					flow->l4.tcp.flash_bytes);
			return;
		}
	}

	NDPI_LOG(NDPI_PROTOCOL_FLASH, ndpi_struct, NDPI_LOG_DEBUG,
			"FLASH might be excluded: flash_stage: %u, flash_bytes: %u, packet_direction: %u\n",
			flow->l4.tcp.flash_stage, flow->l4.tcp.flash_bytes, packet->packet_direction);

#ifdef NDPI_PROTOCOL_HTTP
	if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_HTTP) != 0) {
#endif							/* NDPI_PROTOCOL_HTTP */
		NDPI_LOG(NDPI_PROTOCOL_FLASH, ndpi_struct, NDPI_LOG_DEBUG, "FLASH: exclude\n");
		NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_FLASH);
#ifdef NDPI_PROTOCOL_HTTP
	} else {
		NDPI_LOG(NDPI_PROTOCOL_FLASH, ndpi_struct, NDPI_LOG_DEBUG, "FLASH avoid early exclude from http\n");
	}
#endif							/* NDPI_PROTOCOL_HTTP */

}
#endif
