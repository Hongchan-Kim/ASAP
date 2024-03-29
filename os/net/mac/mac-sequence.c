/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Copyright (c) 2013, ADVANSEE - http://www.advansee.com/
 * Benoît Thébaudeau <benoit.thebaudeau@advansee.com>
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
 *         MAC sequence numbers management
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Benoît Thébaudeau <benoit.thebaudeau@advansee.com>
 */

#include <string.h>

#include "contiki-net.h"
#include "net/mac/mac-sequence.h"
#include "net/packetbuf.h"

#if HCK_MODIFIED_MAC_SEQNO_DUPLICATE_CHECK

#ifdef NETSTACK_CONF_MAC_SEQNO_MAX_AGE
#define SEQNO_MAX_AGE NETSTACK_CONF_MAC_SEQNO_MAX_AGE
#else /* NETSTACK_CONF_MAC_SEQNO_MAX_AGE */
#define SEQNO_MAX_AGE (20 * CLOCK_SECOND)
#endif /* NETSTACK_CONF_MAC_SEQNO_MAX_AGE */

#ifdef NETSTACK_CONF_MAC_SEQNO_HISTORY
#define MAX_SEQNOS NETSTACK_CONF_MAC_SEQNO_HISTORY
#else /* NETSTACK_CONF_MAC_SEQNO_HISTORY */
#define MAX_SEQNOS 16
#endif /* NETSTACK_CONF_MAC_SEQNO_HISTORY */

struct seqno {
  clock_time_t timestamp;
  uint8_t seqno;
};
struct seqnos_from_sender {
  struct seqno seqno_array[MAX_SEQNOS];
};

static struct seqnos_from_sender received_seqnos[NODE_NUM];

/*---------------------------------------------------------------------------*/
int
mac_sequence_is_duplicate(void)
{
  int i;
  clock_time_t now = clock_time();

  /*
   * Check for duplicate packet by comparing the sequence number of the incoming
   * packet with the last few ones we saw.
   */
  int sender_id = HCK_GET_NODE_ID_FROM_LINKADDR(packetbuf_addr(PACKETBUF_ADDR_SENDER));
  uint16_t sender_index = 0;
  if(0 < sender_id && sender_id <= NODE_NUM) {
    sender_index = sender_id - 1; //valid node id
  } else {
    return 1; //invalid node id
  }

  for(i = 0; i < MAX_SEQNOS; ++i) {
    if(packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO) == received_seqnos[sender_index].seqno_array[i].seqno) {
#if SEQNO_MAX_AGE > 0
      if(now - received_seqnos[sender_index].seqno_array[i].timestamp <= SEQNO_MAX_AGE) {
        /* Duplicate packet. */
        return 1;
      }
      break;
#else /* SEQNO_MAX_AGE > 0 */
      return 1;
#endif /* SEQNO_MAX_AGE > 0 */
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
mac_sequence_register_seqno(void)
{
  int i, j;

  int sender_id = HCK_GET_NODE_ID_FROM_LINKADDR(packetbuf_addr(PACKETBUF_ADDR_SENDER));
  uint16_t sender_index = 0;
  if(0 < sender_id && sender_id <= NODE_NUM) {
    sender_index = sender_id - 1; //valid node id
  } else {
    return; //invalid node id
  }

  /* Locate possible previous sequence number for this address. */
  for(i = 0; i < MAX_SEQNOS; ++i) {
    if(packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO) == received_seqnos[sender_index].seqno_array[i].seqno) {
      i++;
      break;
    }
  }


  /* Keep the last sequence number for each address as per 802.15.4e. */
  for(j = i - 1; j > 0; --j) {
    memcpy(&received_seqnos[sender_index].seqno_array[j], &received_seqnos[sender_index].seqno_array[j - 1], sizeof(struct seqno));
  }
  received_seqnos[sender_index].seqno_array[0].seqno = packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO);
  received_seqnos[sender_index].seqno_array[0].timestamp = clock_time();
}
/*---------------------------------------------------------------------------*/

#else /* HCK_MODIFIED_MAC_SEQNO_DUPLICATE_CHECK */

struct seqno {
  linkaddr_t sender;
  clock_time_t timestamp;
  uint8_t seqno;
};

#ifdef NETSTACK_CONF_MAC_SEQNO_MAX_AGE
#define SEQNO_MAX_AGE NETSTACK_CONF_MAC_SEQNO_MAX_AGE
#else /* NETSTACK_CONF_MAC_SEQNO_MAX_AGE */
#define SEQNO_MAX_AGE (20 * CLOCK_SECOND)
#endif /* NETSTACK_CONF_MAC_SEQNO_MAX_AGE */

#ifdef NETSTACK_CONF_MAC_SEQNO_HISTORY
#define MAX_SEQNOS NETSTACK_CONF_MAC_SEQNO_HISTORY
#else /* NETSTACK_CONF_MAC_SEQNO_HISTORY */
#define MAX_SEQNOS 16
#endif /* NETSTACK_CONF_MAC_SEQNO_HISTORY */
static struct seqno received_seqnos[MAX_SEQNOS];

/*---------------------------------------------------------------------------*/
int
mac_sequence_is_duplicate(void)
{
  int i;
  clock_time_t now = clock_time();

  /*
   * Check for duplicate packet by comparing the sequence number of the incoming
   * packet with the last few ones we saw.
   */
  for(i = 0; i < MAX_SEQNOS; ++i) {
    if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_SENDER),
                    &received_seqnos[i].sender)) {
      if(packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO) == received_seqnos[i].seqno) {
#if SEQNO_MAX_AGE > 0
        if(now - received_seqnos[i].timestamp <= SEQNO_MAX_AGE) {
          /* Duplicate packet. */
          return 1;
        }
#else /* SEQNO_MAX_AGE > 0 */
        return 1;
#endif /* SEQNO_MAX_AGE > 0 */
      }
      break;
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
mac_sequence_register_seqno(void)
{
  int i, j;

  /* Locate possible previous sequence number for this address. */
  for(i = 0; i < MAX_SEQNOS; ++i) {
    if(linkaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_SENDER),
                    &received_seqnos[i].sender)) {
      i++;
      break;
    }
  }

  /* Keep the last sequence number for each address as per 802.15.4e. */
  for(j = i - 1; j > 0; --j) {
    memcpy(&received_seqnos[j], &received_seqnos[j - 1], sizeof(struct seqno));
  }
  received_seqnos[0].seqno = packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO);
  received_seqnos[0].timestamp = clock_time();
  linkaddr_copy(&received_seqnos[0].sender,
                packetbuf_addr(PACKETBUF_ADDR_SENDER));
}
/*---------------------------------------------------------------------------*/
#endif /* HCK_MODIFIED_MAC_SEQNO_DUPLICATE_CHECK */