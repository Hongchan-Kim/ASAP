/*
 * Copyright (c) 2017, George Oikonomou - http://www.spd.gr
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
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup main
 * @{
 */
/*---------------------------------------------------------------------------*/
/**
 * \file
 *
 * Implementation of \os's main routine
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "contiki-net.h"
#include "sys/node-id.h"
#include "sys/platform.h"
#include "sys/energest.h"
#include "sys/stack-check.h"
#include "dev/watchdog.h"

#include "net/queuebuf.h"
#include "net/app-layer/coap/coap-engine.h"
#include "net/app-layer/snmp/snmp.h"
#include "services/rpl-border-router/rpl-border-router.h"

#if WITH_ALICE == 1
#include "services/alice/orchestra.h" //alice-implementation
#else /* default orchestra */
#include "services/orchestra/orchestra.h"
#endif

#include "services/shell/serial-shell.h"
#include "services/simple-energest/simple-energest.h"
#include "services/tsch-cs/tsch-cs.h"

#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "Main"
#define LOG_LEVEL LOG_LEVEL_MAIN
/*---------------------------------------------------------------------------*/
int
#if PLATFORM_MAIN_ACCEPTS_ARGS
main(int argc, char **argv)
{
  platform_process_args(argc, argv);
#else
main(void)
{
#endif
  platform_init_stage_one();

  clock_init();
  rtimer_init();
  process_init();
  process_start(&etimer_process, NULL);
  ctimer_init();
  watchdog_init();

  energest_init();

#if STACK_CHECK_ENABLED
  stack_check_init();
#endif

  platform_init_stage_two();

#if QUEUEBUF_ENABLED
  queuebuf_init();
#endif /* QUEUEBUF_ENABLED */
  netstack_init();
  node_id_init();

  LOG_INFO("Starting " CONTIKI_VERSION_STRING "\n");
  LOG_DBG("TARGET=%s", CONTIKI_TARGET_STRING);
#ifdef CONTIKI_BOARD_STRING
  LOG_DBG_(", BOARD=%s", CONTIKI_BOARD_STRING);
#endif
  LOG_DBG_("\n");
  LOG_INFO("- Routing: %s\n", NETSTACK_ROUTING.name);
  LOG_INFO("- Net: %s\n", NETSTACK_NETWORK.name);
  LOG_INFO("- MAC: %s\n", NETSTACK_MAC.name);
  LOG_INFO("- 802.15.4 PANID: 0x%04x\n", IEEE802154_PANID);
#if MAC_CONF_WITH_TSCH
  LOG_INFO("- 802.15.4 TSCH default hopping sequence length: %u\n", (unsigned)sizeof(TSCH_DEFAULT_HOPPING_SEQUENCE));
#else /* MAC_CONF_WITH_TSCH */
  LOG_INFO("- 802.15.4 Default channel: %u\n", IEEE802154_DEFAULT_CHANNEL);
#endif /* MAC_CONF_WITH_TSCH */

  LOG_INFO("Node ID: %u\n", node_id);
  LOG_INFO("Link-layer address: ");
  LOG_INFO_LLADDR(&linkaddr_node_addr);
  LOG_INFO_("\n");

#if NETSTACK_CONF_WITH_IPV6
  {
    uip_ds6_addr_t *lladdr;
    memcpy(&uip_lladdr.addr, &linkaddr_node_addr, sizeof(uip_lladdr.addr));
    process_start(&tcpip_process, NULL);

    lladdr = uip_ds6_get_link_local(-1);
    LOG_INFO("Tentative link-local IPv6 address: ");
    LOG_INFO_6ADDR(lladdr != NULL ? &lladdr->ipaddr : NULL);
    LOG_INFO_("\n");
  }
#endif /* NETSTACK_CONF_WITH_IPV6 */

  platform_init_stage_three();

#if BUILD_WITH_RPL_BORDER_ROUTER
  rpl_border_router_init();
  LOG_DBG("With RPL Border Router\n");
#endif /* BUILD_WITH_RPL_BORDER_ROUTER */

#if BUILD_WITH_ORCHESTRA
  orchestra_init();
  LOG_DBG("With Orchestra\n");
#endif /* BUILD_WITH_ORCHESTRA */

#if BUILD_WITH_SHELL
  serial_shell_init();
  LOG_DBG("With Shell\n");
#endif /* BUILD_WITH_SHELL */

#if BUILD_WITH_COAP
  coap_engine_init();
  LOG_DBG("With CoAP\n");
#endif /* BUILD_WITH_SHELL */

#if BUILD_WITH_SNMP
  snmp_init();
  LOG_DBG("With SNMP\n");
#endif /* BUILD_WITH_SNMP */

#if BUILD_WITH_SIMPLE_ENERGEST
  simple_energest_init();
#endif /* BUILD_WITH_SIMPLE_ENERGEST */

#if BUILD_WITH_TSCH_CS
  /* Initialize the channel selection module */
  tsch_cs_adaptations_init();
#endif /* BUILD_WITH_TSCH_CS */

  autostart_start(autostart_processes);

  watchdog_start();

#if PLATFORM_PROVIDES_MAIN_LOOP
  platform_main_loop();
#else
  while(1) {
    uint8_t r;
    do {
      r = process_run();
      watchdog_periodic();
    } while(r > 0);

    platform_idle();
  }
#endif

  return 0;
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 */
