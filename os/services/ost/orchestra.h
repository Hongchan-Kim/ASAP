/*
 * Copyright (c) 2015, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/**
 * \file
 *         Orchestra header file
 *
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#ifndef __ORCHESTRA_H__
#define __ORCHESTRA_H__

#include "net/mac/tsch/tsch.h"
#include "orchestra-conf.h"

#if WITH_OST
#include "net/ipv6/uip-ds6-nbr.h"
#endif

/* The structure of an Orchestra rule */
struct orchestra_rule {
  void (* init)(uint16_t slotframe_handle);
  void (* new_time_source)(const struct tsch_neighbor *old, const struct tsch_neighbor *new);
  int  (* select_packet)(uint16_t *slotframe, uint16_t *timeslot, uint16_t *channel_offset);
  void (* child_added)(const linkaddr_t *addr);
  void (* child_removed)(const linkaddr_t *addr);
  const char *name;
};

extern struct orchestra_rule eb_per_time_source;
extern struct orchestra_rule unicast_per_neighbor_rpl_storing;
extern struct orchestra_rule unicast_per_neighbor_rpl_ns;
extern struct orchestra_rule unicast_per_neighbor_link_based;
extern struct orchestra_rule default_common;

extern linkaddr_t orchestra_parent_linkaddr;
extern int orchestra_parent_knows_us;

/* Call from application to start Orchestra */
void orchestra_init(void);
/* Callbacks requied for Orchestra to operate */
/* Set with #define TSCH_CALLBACK_PACKET_READY orchestra_callback_packet_ready */
int orchestra_callback_packet_ready(void);
/* Set with #define TSCH_CALLBACK_NEW_TIME_SOURCE orchestra_callback_new_time_source */
void orchestra_callback_new_time_source(const struct tsch_neighbor *old, const struct tsch_neighbor *new);
/* Set with #define NETSTACK_CONF_ROUTING_NEIGHBOR_ADDED_CALLBACK orchestra_callback_child_added */
void orchestra_callback_child_added(const linkaddr_t *addr);
/* Set with #define NETSTACK_CONF_ROUTING_NEIGHBOR_REMOVED_CALLBACK orchestra_callback_child_removed */
void orchestra_callback_child_removed(const linkaddr_t *addr);

#if WITH_OST
/* OST functions */
void ost_print_nbr();
void ost_reset_nbr(const linkaddr_t *addr, uint8_t new_add, uint8_t rx_no_path);
uint16_t ost_get_tx_sf_handle_from_id(const uint16_t id);
uint16_t ost_get_rx_sf_handle_from_id(const uint16_t id);
uint16_t ost_get_id_from_tx_sf_handle(const uint16_t handle);
uint16_t ost_get_id_from_rx_sf_handle(const uint16_t handle);
uint8_t ost_is_routing_nbr(uip_ds6_nbr_t *nbr);
void ost_remove_tx(linkaddr_t *nbr_lladdr);
void ost_remove_rx(uint16_t id);
void ost_change_queue_select_packet(linkaddr_t *nbr_lladdr, uint16_t handle, uint16_t timeslot);
int neighbor_has_uc_link(const linkaddr_t *linkaddr);
/* OST variables */
#if OST_ON_DEMAND_PROVISION
extern struct ost_ssq_schedule_t ost_ssq_schedule_list[16];
#endif
#endif

#endif /* __ORCHESTRA_H__ */
