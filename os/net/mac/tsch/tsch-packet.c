/*
 * Copyright (c) 2014, SICS Swedish ICT.
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
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         TSCH packet format management
 * \author
 *         Simon Duquennoy <simonduq@sics.se>
 *         Beshr Al Nahas <beshr@sics.se>
 */

/**
 * \addtogroup tsch
 * @{
*/

#include "contiki.h"
#include "net/packetbuf.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/framer/frame802154.h"
#include "net/mac/framer/framer-802154.h"
#include "net/netstack.h"
#include "lib/ccm-star.h"
#include "lib/aes-128.h"

#if WITH_OST
#include "node-info.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/ipv6/uip-ds6-nbr.h"
#include "net/mac/tsch/tsch-slot-operation.h"
#include "orchestra.h"
#endif


/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "TSCH Pkt"
#define LOG_LEVEL LOG_LEVEL_MAC

/*
 * We use a local packetbuf_attr array to collect necessary frame settings to
 * create an EACK because EACK is generated in the interrupt context where
 * packetbuf and packetbuf_attrs[] may be in use for another purpose.
 *
 * We have accessors of eackbuf_attrs: tsch_packet_eackbuf_set_attr() and
 * tsch_packet_eackbuf_attr(). For some platform, they might need to be
 * implemented as inline functions. However, for now, we don't provide the
 * inline option. Such an optimization is left to the compiler for a target
 * platform.
 */
static struct packetbuf_attr eackbuf_attrs[PACKETBUF_NUM_ATTRS];

/* The offset of the frame pending bit flag within the first byte of FCF */
#define IEEE802154_FRAME_PENDING_BIT_OFFSET 4

/*---------------------------------------------------------------------------*/
void
tsch_packet_eackbuf_set_attr(uint8_t type, const packetbuf_attr_t val)
{
  eackbuf_attrs[type].val = val;
  return;
}
/*---------------------------------------------------------------------------*/
/* Return the value of a specified attribute */
packetbuf_attr_t
tsch_packet_eackbuf_attr(uint8_t type)
{
  return eackbuf_attrs[type].val;
}
/*---------------------------------------------------------------------------*/
/* Construct enhanced ACK packet and return ACK length */
#if !WITH_OST
#if WITH_UPA
int
tsch_packet_create_eack(uint8_t *buf, uint16_t buf_len,
                        const linkaddr_t *dest_addr, uint8_t seqno,
                        int16_t drift, int nack, 
                        uint16_t upa_pkts_to_receive)
#else /* Default burst transmission or no burst transmission */
int
tsch_packet_create_eack(uint8_t *buf, uint16_t buf_len,
                        const linkaddr_t *dest_addr, uint8_t seqno,
                        int16_t drift, int nack)
#endif
#else /* !WIT_OST */
#if OST_ON_DEMAND_PROVISION
int
tsch_packet_create_eack(uint8_t *buf, uint16_t buf_len,
                        const linkaddr_t *dest_addr, uint8_t seqno,
                        int16_t drift, int nack, 
                        struct input_packet *current_input, uint16_t matching_slot)
#elif WITH_UPA /* HEADER_IE_IN_DATA_AND_ACK */
int
tsch_packet_create_eack(uint8_t *buf, uint16_t buf_len,
                        const linkaddr_t *dest_addr, uint8_t seqno,
                        int16_t drift, int nack, 
                        struct input_packet *current_input, uint16_t upa_pkts_to_receive)
#else /* Default burst transmission or no burst transmission */
int
tsch_packet_create_eack(uint8_t *buf, uint16_t buf_len,
                        const linkaddr_t *dest_addr, uint8_t seqno,
                        int16_t drift, int nack,
                        struct input_packet *current_input)
#endif
#endif
{
  frame802154_t params;
  struct ieee802154_ies ies;
  int hdr_len;
  int ack_len;
#if WITH_UPA /* HEADER_IE_IN_DATA_AND_ACK */
  int ies_len;
  int current_ie_len;
#endif

  if(buf == NULL) {
    return -1;
  }

  memset(eackbuf_attrs, 0, sizeof(eackbuf_attrs));

  tsch_packet_eackbuf_set_attr(PACKETBUF_ATTR_FRAME_TYPE, FRAME802154_ACKFRAME);
  tsch_packet_eackbuf_set_attr(PACKETBUF_ATTR_MAC_METADATA, 1);
  tsch_packet_eackbuf_set_attr(PACKETBUF_ATTR_MAC_SEQNO, seqno);

  tsch_packet_eackbuf_set_attr(PACKETBUF_ATTR_MAC_NO_DEST_ADDR, 1);
#if TSCH_PACKET_EACK_WITH_DEST_ADDR
  if(dest_addr != NULL) {
    tsch_packet_eackbuf_set_attr(PACKETBUF_ATTR_MAC_NO_DEST_ADDR, 0);
    linkaddr_copy((linkaddr_t *)&params.dest_addr, dest_addr);
  }
#endif

  tsch_packet_eackbuf_set_attr(PACKETBUF_ATTR_MAC_NO_SRC_ADDR, 1);
#if TSCH_PACKET_EACK_WITH_SRC_ADDR
  tsch_packet_eackbuf_set_attr(PACKETBUF_ATTR_MAC_NO_SRC_ADDR, 0);
  linkaddr_copy((linkaddr_t *)&params.src_addr, &linkaddr_node_addr);
#endif

#if LLSEC802154_ENABLED
  tsch_security_set_packetbuf_attr(FRAME802154_ACKFRAME);
#endif /* LLSEC802154_ENABLED */

  framer_802154_setup_params(tsch_packet_eackbuf_attr, 0, &params);

#if WITH_OST /* Piggyback t_offset */
  if(current_input != NULL) { /* In b-ack of UPA, current_input is NULL */
    uip_ds6_nbr_t *ds6_nbr = uip_ds6_nbr_ll_lookup((uip_lladdr_t *)dest_addr);
    if(ds6_nbr != NULL
      && ost_is_routing_nbr(ds6_nbr) == 1
      && ds6_nbr->ost_rx_no_path == 0) {
      params.ost_pigg1 = current_input->ost_prN_new_t_offset;

      if(current_input->ost_flag_failed_to_select_t_offset == 1) {
        params.ost_pigg1 = OST_T_OFFSET_ALLOCATION_FAILURE;
      }

      if(current_input->ost_flag_respond_to_consec_new_tx_sched_req == 1) {
        params.ost_pigg1= OST_T_OFFSET_CONSECUTIVE_NEW_TX_REQUEST;
      }

    } else {
      /* Tx EACK: t_offset make 65535 (No ds6_nbr) */
      params.ost_pigg1 = 0xffff;
    }
  }

#if OST_ON_DEMAND_PROVISION
  params.ost_pigg2 = matching_slot; /* Tx EACK: matching slot */
#endif
#endif

  hdr_len = frame802154_hdrlen(&params);

  memset(buf, 0, buf_len);

  /* Setup IE timesync */
  memset(&ies, 0, sizeof(ies));
  ies.ie_time_correction = drift;
  ies.ie_is_nack = nack;

#if WITH_UPA /* HEADER_IE_IN_DATA_AND_ACK */
  ies.ie_upa_info = upa_pkts_to_receive;
  ies_len = 0;
  
  current_ie_len = 0;
  current_ie_len =
    frame80215e_create_ie_header_upa_info(buf + hdr_len + ies_len, 
                                                          buf_len - hdr_len - ies_len, &ies);
  if(current_ie_len < 0) {
    return -1;
  }
  ies_len += current_ie_len;

  current_ie_len = 0;
  current_ie_len =
    frame80215e_create_ie_header_ack_nack_time_correction(buf + hdr_len + ies_len,
                                                          buf_len - hdr_len - ies_len, &ies);
  if(current_ie_len < 0) {
    return -1;
  }
  ies_len += current_ie_len;

  ack_len = hdr_len + ies_len;

#else /* WITH_UPA */

  ack_len =
    frame80215e_create_ie_header_ack_nack_time_correction(buf + hdr_len,
                                                          buf_len - hdr_len, &ies);
  if(ack_len < 0) {
    return -1;
  }
  ack_len += hdr_len;

#endif

  frame802154_create(&params, buf);

  return ack_len;
}
/*---------------------------------------------------------------------------*/
/* Parse enhanced ACK packet, extract drift and nack */
int
tsch_packet_parse_eack(const uint8_t *buf, int buf_size,
                       uint8_t seqno, frame802154_t *frame, struct ieee802154_ies *ies, uint8_t *hdr_len)
{
  uint8_t curr_len = 0;
  int ret;
  linkaddr_t dest;

  if(frame == NULL || buf_size < 0) {
    return 0;
  }
  /* Parse 802.15.4-2006 frame, i.e. all fields before Information Elements */
  if((ret = frame802154_parse((uint8_t *)buf, buf_size, frame)) < 3) {
    return 0;
  }
  if(hdr_len != NULL) {
    *hdr_len = ret;
  }
  curr_len += ret;

  /* Check seqno */
  if(seqno != frame->seq) {
    return 0;
  }

  /* Check destination PAN ID */
  if(frame802154_check_dest_panid(frame) == 0) {
    return 0;
  }

  /* Check destination address (if any) */
  if(frame802154_extract_linkaddr(frame, NULL, &dest) == 0 ||
     (!linkaddr_cmp(&dest, &linkaddr_node_addr)
      && !linkaddr_cmp(&dest, &linkaddr_null))) {
    return 0;
  }

  if(ies != NULL) {
    memset(ies, 0, sizeof(struct ieee802154_ies));
  }

  if(frame->fcf.ie_list_present) {
    int mic_len = 0;
#if LLSEC802154_ENABLED
    /* Check if there is space for the security MIC (if any) */
    mic_len = tsch_security_mic_len(frame);
    if(buf_size < curr_len + mic_len) {
      return 0;
    }
#endif /* LLSEC802154_ENABLED */
    /* Parse information elements. We need to substract the MIC length, as the exact payload len is needed while parsing */
    if((ret = frame802154e_parse_information_elements(buf + curr_len, buf_size - curr_len - mic_len, ies)) == -1) {
      return 0;
    }
    curr_len += ret;
  }

  if(hdr_len != NULL) {
    *hdr_len += ies->ie_payload_ie_offset;
  }

  return curr_len;
}
/*---------------------------------------------------------------------------*/
/* Create an EB packet */
int
tsch_packet_create_eb(uint8_t *hdr_len, uint8_t *tsch_sync_ie_offset)
{
  struct ieee802154_ies ies;
  uint8_t *p;
  int ie_len;
  const uint16_t payload_ie_hdr_len = 2;

  packetbuf_clear();

  /* Prepare Information Elements for inclusion in the EB */
  memset(&ies, 0, sizeof(ies));

#if WITH_OST_TODO
  p.ost_pigg1 = 0xffff;
#endif

#if WITH_SLA /* Coordinator/non-coordinator: piggyback information to ies */
  ies.ie_sla_triggering_asn = sla_triggering_asn;
  ies.ie_sla_curr_timeslot_len = tsch_timing_us[tsch_ts_timeslot_length];
  ies.ie_sla_next_timeslot_len = sla_next_timeslot_length;
#endif

  /* Add TSCH timeslot timing IE. */
#if TSCH_PACKET_EB_WITH_TIMESLOT_TIMING
  {
    int i;
    ies.ie_tsch_timeslot_id = 1;
    for(i = 0; i < tsch_ts_elements_count; i++) {
      ies.ie_tsch_timeslot[i] = RTIMERTICKS_TO_US(tsch_timing[i]);
    }
  }
#endif /* TSCH_PACKET_EB_WITH_TIMESLOT_TIMING */

  /* Add TSCH hopping sequence IE */
#if TSCH_PACKET_EB_WITH_HOPPING_SEQUENCE
  if(tsch_hopping_sequence_length.val <= sizeof(ies.ie_hopping_sequence_list)) {
    ies.ie_channel_hopping_sequence_id = 1;
    ies.ie_hopping_sequence_len = tsch_hopping_sequence_length.val;
    memcpy(ies.ie_hopping_sequence_list, tsch_hopping_sequence,
           ies.ie_hopping_sequence_len);
  }
#endif /* TSCH_PACKET_EB_WITH_HOPPING_SEQUENCE */

  /* Add Slotframe and Link IE */
#if TSCH_PACKET_EB_WITH_SLOTFRAME_AND_LINK
  {
    /* Send slotframe 0 with link at timeslot 0 and channel offset 0 */
    struct tsch_slotframe *sf0 = tsch_schedule_get_slotframe_by_handle(0);
    struct tsch_link *link0 = tsch_schedule_get_link_by_timeslot(sf0, 0, 0);
    if(sf0 && link0) {
      ies.ie_tsch_slotframe_and_link.num_slotframes = 1;
      ies.ie_tsch_slotframe_and_link.slotframe_handle = sf0->handle;
      ies.ie_tsch_slotframe_and_link.slotframe_size = sf0->size.val;
      ies.ie_tsch_slotframe_and_link.num_links = 1;
      ies.ie_tsch_slotframe_and_link.links[0].timeslot = link0->timeslot;
      ies.ie_tsch_slotframe_and_link.links[0].channel_offset =
        link0->channel_offset;
      ies.ie_tsch_slotframe_and_link.links[0].link_options =
        link0->link_options;
    }
  }
#endif /* TSCH_PACKET_EB_WITH_SLOTFRAME_AND_LINK */

  p = packetbuf_dataptr();

  ie_len = frame80215e_create_ie_tsch_synchronization(p,
                                                      packetbuf_remaininglen(),
                                                      &ies);
  if(ie_len < 0) {
    return -1;
  }
  p += ie_len;
  packetbuf_set_datalen(packetbuf_datalen() + ie_len);

#if WITH_SLA /* Coordinator/non-coordinator: piggyback information to ies */
  ie_len = frame80215e_create_ie_tsch_sla_triggering_asn(p,
                                                      packetbuf_remaininglen(),
                                                      &ies);
  if(ie_len < 0) {
    return -1;
  }
  p += ie_len;
  packetbuf_set_datalen(packetbuf_datalen() + ie_len);

  ie_len = frame80215e_create_ie_tsch_sla_timeslot_len(p,
                                               packetbuf_remaininglen(),
                                               &ies);
  if(ie_len < 0) {
    return -1;
  }
  p += ie_len;
  packetbuf_set_datalen(packetbuf_datalen() + ie_len);
#endif

  ie_len = frame80215e_create_ie_tsch_timeslot(p,
                                               packetbuf_remaininglen(),
                                               &ies);
  if(ie_len < 0) {
    return -1;
  }
  p += ie_len;
  packetbuf_set_datalen(packetbuf_datalen() + ie_len);

  ie_len = frame80215e_create_ie_tsch_channel_hopping_sequence(p,
                                                               packetbuf_remaininglen(),
                                                               &ies);
  if(ie_len < 0) {
    return -1;
  }
  p += ie_len;
  packetbuf_set_datalen(packetbuf_datalen() + ie_len);

  ie_len = frame80215e_create_ie_tsch_slotframe_and_link(p,
                                                         packetbuf_remaininglen(),
                                                         &ies);
  if(ie_len < 0) {
    return -1;
  }
  p += ie_len;
  packetbuf_set_datalen(packetbuf_datalen() + ie_len);

#if 0
  /* Payload IE list termination: optional */
  ie_len = frame80215e_create_ie_payload_list_termination(p,
                                                          packetbuf_remaininglen(),
                                                          &ies);
  if(ie_len < 0) {
    return -1;
  }
  p += ie_len;
  packetbuf_set_datalen(packetbuf_datalen() + ie_len);
#endif

  ies.ie_mlme_len = packetbuf_datalen();

  /* make room for Payload IE header */
  memmove((uint8_t *)packetbuf_dataptr() + payload_ie_hdr_len,
          packetbuf_dataptr(), packetbuf_datalen());
  packetbuf_set_datalen(packetbuf_datalen() + payload_ie_hdr_len);
  ie_len = frame80215e_create_ie_mlme(packetbuf_dataptr(),
                                      packetbuf_remaininglen(),
                                      &ies);
  if(ie_len < 0) {
    return -1;
  }

  /* allocate space for Header Termination IE, the size of which is 2 octets */
  packetbuf_hdralloc(2);
  ie_len = frame80215e_create_ie_header_list_termination_1(packetbuf_hdrptr(),
                                                           packetbuf_remaininglen(),
                                                           &ies);
  if(ie_len < 0) {
    return -1;
  }

  packetbuf_set_attr(PACKETBUF_ATTR_FRAME_TYPE, FRAME802154_BEACONFRAME);
  packetbuf_set_attr(PACKETBUF_ATTR_MAC_METADATA, 1);

  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &tsch_eb_address);

#if LLSEC802154_ENABLED
  tsch_security_set_packetbuf_attr(FRAME802154_BEACONFRAME);
#endif /* LLSEC802154_ENABLED */

  if(NETSTACK_FRAMER.create() < 0) {
    return -1;
  }

  if(hdr_len != NULL) {
    *hdr_len = packetbuf_hdrlen();
  }

  /*
   * Save the offset of the TSCH Synchronization IE, which is expected to be
   * located just after the Payload IE header, needed to update ASN and join
   * priority before sending.
   */
  if(tsch_sync_ie_offset != NULL) {
    *tsch_sync_ie_offset = packetbuf_hdrlen() + payload_ie_hdr_len;
  }

  return packetbuf_totlen();
}
/*---------------------------------------------------------------------------*/
/* Update ASN in EB packet */
int
tsch_packet_update_eb(uint8_t *buf, int buf_size, uint8_t tsch_sync_ie_offset)
{
  struct ieee802154_ies ies;
  ies.ie_asn = tsch_current_asn;
  ies.ie_join_priority = tsch_join_priority;
  return frame80215e_create_ie_tsch_synchronization(buf+tsch_sync_ie_offset, buf_size-tsch_sync_ie_offset, &ies) != -1;
}
/*---------------------------------------------------------------------------*/
#if WITH_SLA /* Coordinator/non-coordinator: update information in EB before transmission */
/* Update SLA info in EB packet */
int
sla_packet_update_eb(uint8_t *buf, int buf_size, uint8_t tsch_sync_ie_offset)
{
  struct ieee802154_ies ies;
  ies.ie_sla_triggering_asn = sla_triggering_asn;
  ies.ie_sla_curr_timeslot_len = tsch_timing_us[tsch_ts_timeslot_length];
  ies.ie_sla_next_timeslot_len = sla_next_timeslot_length;
  uint8_t result_triggering_asn = frame80215e_create_ie_tsch_sla_triggering_asn(buf+tsch_sync_ie_offset+8, buf_size-tsch_sync_ie_offset-8, &ies) != -1;
  uint8_t result_frame_ack_len = frame80215e_create_ie_tsch_sla_timeslot_len(buf+tsch_sync_ie_offset+8+7, buf_size-tsch_sync_ie_offset-8-7, &ies) != -1;
  return result_triggering_asn && result_frame_ack_len;
}
#endif
/*---------------------------------------------------------------------------*/
/* Parse a IEEE 802.15.4e TSCH Enhanced Beacon (EB) */
int
tsch_packet_parse_eb(const uint8_t *buf, int buf_size,
                     frame802154_t *frame, struct ieee802154_ies *ies, uint8_t *hdr_len, int frame_without_mic)
{
  uint8_t curr_len = 0;
  int ret;

  if(frame == NULL || buf_size < 0) {
    return 0;
  }

  /* Parse 802.15.4-2006 frame, i.e. all fields before Information Elements */
  if((ret = frame802154_parse((uint8_t *)buf, buf_size, frame)) == 0) {
    LOG_ERR("! parse_eb: failed to parse frame\n");
    return 0;
  }

  if(frame->fcf.frame_version < FRAME802154_IEEE802154_2015
     || frame->fcf.frame_type != FRAME802154_BEACONFRAME) {
    LOG_INFO("! parse_eb: frame is not a TSCH beacon." \
           " Frame version %u, type %u, FCF %02x %02x\n",
           frame->fcf.frame_version, frame->fcf.frame_type, buf[0], buf[1]);
    LOG_INFO("! parse_eb: frame was from 0x%x/", frame->src_pid);
    LOG_INFO_LLADDR((const linkaddr_t *)&frame->src_addr);
    LOG_INFO_(" to 0x%x/", frame->dest_pid);
    LOG_INFO_LLADDR((const linkaddr_t *)&frame->dest_addr);
    LOG_INFO_("\n");
    return 0;
  }

  if(hdr_len != NULL) {
    *hdr_len = ret;
  }
  curr_len += ret;

  if(ies != NULL) {
    memset(ies, 0, sizeof(struct ieee802154_ies));
    ies->ie_join_priority = 0xff; /* Use max value in case the Beacon does not include a join priority */
  }
  if(frame->fcf.ie_list_present) {
    /* Calculate space needed for the security MIC, if any, before attempting to parse IEs */
    int mic_len = 0;
#if LLSEC802154_ENABLED
    if(!frame_without_mic) {
      mic_len = tsch_security_mic_len(frame);
      if(buf_size < curr_len + mic_len) {
        return 0;
      }
    }
#endif /* LLSEC802154_ENABLED */

    /* Parse information elements. We need to substract the MIC length, as the exact payload len is needed while parsing */
    if((ret = frame802154e_parse_information_elements(buf + curr_len, buf_size - curr_len - mic_len, ies)) == -1) {
      LOG_ERR("! parse_eb: failed to parse IEs\n");
      return 0;
    }
    curr_len += ret;
  }

  if(hdr_len != NULL) {
    *hdr_len += ies->ie_payload_ie_offset;
  }

  return curr_len;
}
/*---------------------------------------------------------------------------*/
/* Set frame pending bit in a packet (whose header was already build) */
void
tsch_packet_set_frame_pending(uint8_t *buf, int buf_size)
{
  buf[0] |= (1 << IEEE802154_FRAME_PENDING_BIT_OFFSET);
}
/*---------------------------------------------------------------------------*/
/* Get frame pending bit from a packet */
int
tsch_packet_get_frame_pending(uint8_t *buf, int buf_size)
{
  return (buf[0] >> IEEE802154_FRAME_PENDING_BIT_OFFSET) & 1;
}
/*---------------------------------------------------------------------------*/
/** @} */
