#include "node-info.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#if IOTLAB_SITE == IOTLAB_LYON_2
uint16_t iotlab_nodes[NODE_NUM][3] = {
  /* {host name, uid, rx count} */
  {7, 0xa371, 0}, /* root */
  {3, 0x8676, 0}
};
#elif IOTLAB_SITE == IOTLAB_LYON_3
uint16_t iotlab_nodes[NODE_NUM][3] = {
  /* {host name, uid, rx count} */
  {7, 0xa371, 0}, /* root */
  {2, 0x8867, 0},
  {3, 0x8676, 0}
};
#elif IOTLAB_SITE == IOTLAB_LYON_10
uint16_t iotlab_nodes[NODE_NUM][3] = {
  /* {host name, uid, rx count} */
  {18, 0x3261, 0}, /* root */
  {1, 0x9768, 0},
  {2, 0x8867, 0},
  {3, 0x8676, 0},
  {4, 0xb181, 0},
  {5, 0x8968, 0},
  {6, 0xc279, 0},
  {7, 0xa371, 0},
  {8, 0xa683, 0},
  {10, 0x8976, 0}
};
#elif IOTLAB_SITE == IOTLAB_LYON_17
uint16_t iotlab_nodes[NODE_NUM][3] = {
  /* {host name, uid, rx count} */
  {18, 0x3261, 0}, /* root */
  {1, 0x9768, 0},
  {2, 0x8867, 0},
  {3, 0x8676, 0},
  {4, 0xb181, 0},
  {5, 0x8968, 0},
  {6, 0xc279, 0},
  {7, 0xa371, 0},
  {8, 0xa683, 0},
  {10, 0x8976, 0},
  {11, 0x8467, 0},
  {12, 0xb682, 0},
  {13, 0xb176, 0},
  {14, 0x2860, 0},
  {15, 0xa377, 0},
  {16, 0xb978, 0},
  {17, 0xa168, 0}
};
#elif IOTLAB_SITE == IOTLAB_LILLE_24
uint16_t iotlab_nodes[NODE_NUM][3] = {
  /* {host name, uid, rx count} */
  {250, 0x2659, 0}, /* root */
  {152, 0xa173, 0},
  {154, 0xb071, 0},
  {156, 0x3759, 0},
  {158, 0x2154, 0},
  {170, 0xb671, 0},
  {172, 0x3554, 0},
  {174, 0x9273, 0},
  {191, 0x8473, 0},
  {193, 0x3558, 0},
  {195, 0xb388, 0},
  {206, 0x2850, 0},
  {208, 0x2350, 0},
  {210, 0x1855, 0},
  {225, 0xa573, 0},
  {227, 0x2559, 0},
  {229, 0x9770, 0},
  {231, 0x2052, 0},
  {239, 0xc270, 0},
  {241, 0xb070, 0},
  {243, 0xb073, 0},
  {252, 0x2458, 0},
  {254, 0x2358, 0},
  {256, 0x2554, 0}
};
#elif IOTLAB_SITE == IOTLAB_LILLE_32
uint16_t iotlab_nodes[NODE_NUM][3] = {
  /* {host name, uid, rx count} */
  {89, 0xa990, 0}, /* root */
  {46, 0xb772, 0},
  {47, 0x1854, 0},
  {48, 0x956, 0},
  {51, 0xb973, 0},
  {52, 0x3658, 0},
  {53, 0x1651, 0},
  {54, 0x3151, 0},
  {55, 0xb771, 0},
  {56, 0xa273, 0},
  {57, 0x1256, 0},
  {58, 0x8674, 0},
  {59, 0xb271, 0},
  {60, 0x761, 0},
  {61, 0x3254, 0},
  {62, 0x2359, 0},
  {64, 0x1758, 0},
  {65, 0xb473, 0},
  {66, 0x1857, 0},
  {67, 0x1957, 0},
  {68, 0x2855, 0},
  {71, 0x1654, 0},
  {73, 0x1956, 0},
  {77, 0x1756, 0},
  {78, 0x2250, 0},
  {80, 0x2750, 0},
  {81, 0x1556, 0},
  {82, 0x2156, 0},
  {83, 0xa489, 0},
  {85, 0x9773, 0},
  {86, 0xb471, 0},
  {87, 0x2155, 0}
};
#elif IOTLAB_SITE == IOTLAB_LILLE_46
uint16_t iotlab_nodes[NODE_NUM][3] = {
  /* {host name, uid, rx count} */
  {250, 0x2659, 0}, /* root */
  {152, 0xa173, 0},
  {153, 0xb572, 0},
  {154, 0xb071, 0},
  {155, 0xb372, 0},
  {156, 0x3759, 0},
  {157, 0x2755, 0},
  {158, 0x2154, 0},
  {169, 0x1459, 0},
  {170, 0xb671, 0},
  {171, 0x2258, 0},
  {172, 0x3554, 0},
  {173, 0xc170, 0},
  {174, 0x9273, 0},
  {175, 0x2459, 0},
  {191, 0x8473, 0},
  {193, 0x3558, 0},
  {194, 0x1159, 0},
  {195, 0xb388, 0},
  {196, 0x2451, 0},
  {205, 0xb173, 0},
  {206, 0x2850, 0},
  {207, 0x3359, 0},
  {208, 0x2350, 0},
  {209, 0x2050, 0},
  {210, 0x1855, 0},
  {225, 0xa573, 0},
  {226, 0x9573, 0},
  {227, 0x2559, 0},
  {228, 0x1455, 0},
  {229, 0x9770, 0},
  {230, 0x2751, 0},
  {231, 0x2052, 0},
  {238, 0x9373, 0},
  {239, 0xc270, 0},
  {240, 0xb288, 0},
  {241, 0xb070, 0},
  {242, 0x2450, 0},
  {243, 0xb073, 0},
  {244, 0x1258, 0},
  {251, 0x2454, 0},
  {252, 0x2458, 0},
  {253, 0xb871, 0},
  {254, 0x2358, 0},
  {255, 0xb371, 0},
  {256, 0x2554, 0}
};
#endif

uint16_t
iotlab_node_id_from_uid(uint16_t uid)
{
  uint16_t i = 0;
  for(i = 0; i < NODE_NUM; i++) {
    if(iotlab_nodes[i][1] == uid) {
      return i + 1;
    }
  }
  return 0; /* no matching index */
}
/*---------------------------------------------------------------------------*/
void
print_node_info()
{
  LOG_INFO("HCK-NODE root %u %x (%u %x)\n", IOTLAB_ROOT_ID, IOTLAB_ROOT_ID, iotlab_nodes[0][0], iotlab_nodes[0][1]);
  uint8_t i = 1;
  for(i = 1; i < NODE_NUM; i++) {
    LOG_INFO("HCK-NODE non_root %u %x (%u %x)\n", i + 1, i + 1, iotlab_nodes[i][0], iotlab_nodes[i][1]);
  }
  LOG_INFO("HCK-NODE end\n");
}
/*---------------------------------------------------------------------------*/
#if WITH_OST_OID
uint16_t
ost_node_id_from_ipaddr(const uip_ipaddr_t *addr)
{
  uint16_t id = ((addr->u8[14]) << 8) + addr->u8[15];
  return id;
}
/*---------------------------------------------------------------------------*/
uint16_t
ost_node_id_from_linkaddr(const linkaddr_t *addr)
{
  uint16_t id = ((addr->u8[6]) << 8) + addr->u8[7];
  return id;
}
#endif