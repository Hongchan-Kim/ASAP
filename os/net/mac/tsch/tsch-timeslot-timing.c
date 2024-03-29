/*
 * Copyright (c) 2018, RISE SICS.
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
 *         IEEE 802.15.4 TSCH timeslot timings
 * \author
 *         Simon Duquennoy <simon.duquennoy@ri.se>
 *
 */

/**
 * \addtogroup tsch
 * @{
*/

#include "contiki.h"
#include "net/mac/tsch/tsch.h"

/**
 * \brief TSCH timing attributes and description. All timings are in usec.
 *
 * CCAOffset   -> time between the beginning of timeslot and start of CCA
 * CCA         -> duration of CCA (CCA is NOT ENABLED by default)
 * TxOffset    -> time between beginning of the timeslot and start of frame TX (end of SFD)
 * RxOffset    -> beginning of the timeslot to when the receiver shall be listening
 * RxAckDelay  -> end of frame to when the transmitter shall listen for ACK
 * TxAckDelay  -> end of frame to the start of ACK tx
 * RxWait      -> time to wait for start of frame (Guard time)
 * AckWait     -> min time to wait for start of an ACK frame
 * RxTx        -> receive-to-transmit switch time (NOT USED)
 * MaxAck      -> TX time to send a max length ACK
 * MaxTx       -> TX time to send the max length frame
 *
 * The TSCH timeslot structure is described in the IEEE 802.15.4-2015 standard,
 * in particular in the Figure 6-30.
 *
 * The default timeslot timing in the standard is a guard time of
 * 2200 us, a Tx offset of 2120 us and a Rx offset of 1120 us.
 * As a result, the listening device has a guard time not centered
 * on the expected Tx time. This is to be fixed in the next iteration
 * of the standard. This can be enabled with:
 * TxOffset: 2120
 * RxOffset: 1120
 * RxWait:   2200
 *
 * Instead, we align the Rx guard time on expected Tx time. The Rx
 * guard time is user-configurable with TSCH_CONF_RX_WAIT.
 * (TxOffset - (RxWait / 2)) instead
 */

#ifndef HCK_TSCH_TIMESLOT_LENGTH
#define HCK_TSCH_TIMESLOT_LENGTH 10000
#endif

#ifndef HCK_TSCH_MAX_ACK
#define HCK_TSCH_MAX_ACK 2400
#endif

const tsch_timeslot_timing_usec tsch_timeslot_timing_us_10000 = {
#if TSCH_CONF_CCA_ENABLED
#if UPA_TRIPLE_CCA
    910, /* CCAOffset */
    150, /* CCA (radio-rf2xx requires 140 us or 5 ticks) */
   2950, /* TxOffset */
  (2950 - (TSCH_CONF_RX_WAIT / 2)), /* RxOffset */
#else /* UPA_TRIPLE_CCA */
   1600, /* CCAOffset (52 ticks) */
    150, /* CCA (radio-rf2xx requires 140 us or 5 ticks) */
   2120, /* TxOffset */
  (2120 - (TSCH_CONF_RX_WAIT / 2)), /* RxOffset */
#endif /* UPA_TRIPLE_CCA */
#else /* TSCH_CONF_CCA_ENABLED */
   1800, /* CCAOffset */
    128, /* CCA */
   2120, /* TxOffset */
  (2120 - (TSCH_CONF_RX_WAIT / 2)), /* RxOffset */
#endif /* TSCH_CONF_CCA_ENABLED */
#if WITH_UPA
   1100, /* RxAckDelay - 1000 */
   1300, /* TxAckDelay - 1200 */
#elif WITH_OST
   1000, /* RxAckDelay - 1000 */
   1200, /* TxAckDelay - 1200 */
#else
    800, /* RxAckDelay */
   1000, /* TxAckDelay */
#endif
  TSCH_CONF_RX_WAIT, /* RxWait */
    400, /* AckWait */
    192, /* RxTx */
  HCK_TSCH_MAX_ACK, //2400, /* MaxAck */
   4256, /* MaxTx */
  HCK_TSCH_TIMESLOT_LENGTH, //10000, /* TimeslotLength */
#if UPA_TRIPLE_CCA
   720, /* Inter CCA offset */
#endif
};

#if WITH_UPA
#define UPA_RX_WAIT 300
const upa_timeslot_timing_usec upa_timeslot_timing_us_10000 = {
   1250, /* upa_ts_tx_offset_1 (41 ticks required) */
   (1250 - (UPA_RX_WAIT / 2)), /* upa_ts_rx_offset_1 */
   1000, /* upa_ts_tx_offset_2 (30 ticks required, 1000 == 33 ticks) */
   (1000 - (UPA_RX_WAIT / 2)), /* upa_ts_rx_offset_2 */
   1050, /* upa_ts_rx_ack_delay */
   1250, /* upa_ts_tx_ack_delay */
  UPA_RX_WAIT, /* upa_ts_rx_wait */
    400, /* upa_ts_ack_wait */
#if WITH_OST
    900, /* upa_ts_max_ack (23 bytes) -> 832 us, 27 ticks -> 2 ticks for guard -> 900 us, 29 ticks */
#else
    810, /* upa_ts_max_ack (21 bytes) -> 768 us, 25 ticks -> 2 ticks for guard -> 810 us, 27 ticks */
#endif
   4256, /* upa_ts_max_tx */
    360, /* upa_ts_tx_process_b_ack (12 ticks) - time to read and process b-ack */
};
#endif

/** @} */
