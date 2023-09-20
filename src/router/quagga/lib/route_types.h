/* Auto-generated from route_types.txt by . */
/* Do not edit! */

#ifndef _FRR_ROUTE_TYPES_H
#define _FRR_ROUTE_TYPES_H

/* Zebra route's' types. */
#define ZEBRA_ROUTE_SYSTEM               0
#define ZEBRA_ROUTE_KERNEL               1
#define ZEBRA_ROUTE_CONNECT              2
#define ZEBRA_ROUTE_STATIC               3
#define ZEBRA_ROUTE_RIP                  4
#define ZEBRA_ROUTE_RIPNG                5
#define ZEBRA_ROUTE_OSPF                 6
#define ZEBRA_ROUTE_OSPF6                7
#define ZEBRA_ROUTE_ISIS                 8
#define ZEBRA_ROUTE_BGP                  9
#define ZEBRA_ROUTE_PIM                  10
#define ZEBRA_ROUTE_EIGRP                11
#define ZEBRA_ROUTE_NHRP                 12
#define ZEBRA_ROUTE_HSLS                 13
#define ZEBRA_ROUTE_OLSR                 14
#define ZEBRA_ROUTE_TABLE                15
#define ZEBRA_ROUTE_LDP                  16
#define ZEBRA_ROUTE_VNC                  17
#define ZEBRA_ROUTE_VNC_DIRECT           18
#define ZEBRA_ROUTE_VNC_DIRECT_RH        19
#define ZEBRA_ROUTE_BGP_DIRECT           20
#define ZEBRA_ROUTE_BGP_DIRECT_EXT       21
#define ZEBRA_ROUTE_BABEL                22
#define ZEBRA_ROUTE_SHARP                23
#define ZEBRA_ROUTE_PBR                  24
#define ZEBRA_ROUTE_BFD                  25
#define ZEBRA_ROUTE_OPENFABRIC           26
#define ZEBRA_ROUTE_VRRP                 27
#define ZEBRA_ROUTE_NHG                  28
#define ZEBRA_ROUTE_SRTE                 29
#define ZEBRA_ROUTE_ALL                  30
#define ZEBRA_ROUTE_MAX                  31

#define SHOW_ROUTE_V4_HEADER \
  "Codes: K - kernel route, C - connected, S - static, R - RIP,\n" \
  "       O - OSPF, I - IS-IS, B - BGP, E - EIGRP, N - NHRP,\n" \
  "       T - Table, v - VNC, V - VNC-Direct, A - Babel, F - PBR,\n" \
  "       f - OpenFabric,\n" \
  "       > - selected route, * - FIB route, q - queued, r - rejected, b - backup\n"  "       t - trapped, o - offload failure\n\n"
#define SHOW_ROUTE_V6_HEADER \
  "Codes: K - kernel route, C - connected, S - static, R - RIPng,\n" \
  "       O - OSPFv3, I - IS-IS, B - BGP, N - NHRP, T - Table,\n" \
  "       v - VNC, V - VNC-Direct, A - Babel, F - PBR,\n" \
  "       f - OpenFabric,\n" \
  "       > - selected route, * - FIB route, q - queued, r - rejected, b - backup\n"  "       t - trapped, o - offload failure\n\n"

/* babeld */
#define FRR_REDIST_STR_BABELD \
  "<kernel|connected|static|rip|ripng|ospf|ospf6|isis|bgp|eigrp|nhrp|table|vnc|openfabric>"
#define FRR_REDIST_HELP_STR_BABELD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Enhanced Interior Gateway Routing Protocol (EIGRP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "OpenFabric Routing Protocol\n"
#define FRR_IP_REDIST_STR_BABELD \
  "<kernel|connected|static|rip|ospf|isis|bgp|eigrp|nhrp|table|vnc|openfabric>"
#define FRR_IP_REDIST_HELP_STR_BABELD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Enhanced Interior Gateway Routing Protocol (EIGRP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "OpenFabric Routing Protocol\n"
#define FRR_IP6_REDIST_STR_BABELD \
  "<kernel|connected|static|ripng|ospf6|isis|bgp|nhrp|table|vnc|openfabric>"
#define FRR_IP6_REDIST_HELP_STR_BABELD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "OpenFabric Routing Protocol\n"

/* bgpd */
#define FRR_REDIST_STR_BGPD \
  "<kernel|connected|static|rip|ripng|ospf|ospf6|isis|eigrp|nhrp|table|vnc|vnc-direct|babel|openfabric>"
#define FRR_REDIST_HELP_STR_BGPD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Enhanced Interior Gateway Routing Protocol (EIGRP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "VNC direct (not via zebra) routes\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"
#define FRR_IP_REDIST_STR_BGPD \
  "<kernel|connected|static|rip|ospf|isis|eigrp|nhrp|table|vnc|vnc-direct|babel|openfabric>"
#define FRR_IP_REDIST_HELP_STR_BGPD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Enhanced Interior Gateway Routing Protocol (EIGRP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "VNC direct (not via zebra) routes\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"
#define FRR_IP6_REDIST_STR_BGPD \
  "<kernel|connected|static|ripng|ospf6|isis|nhrp|table|vnc|vnc-direct|babel|openfabric>"
#define FRR_IP6_REDIST_HELP_STR_BGPD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "VNC direct (not via zebra) routes\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"

/* eigrpd */
#define FRR_REDIST_STR_EIGRPD \
  "<kernel|connected|static|rip|ospf|isis|bgp|nhrp|table|vnc|babel|openfabric>"
#define FRR_REDIST_HELP_STR_EIGRPD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"

/* fabricd */
#define FRR_REDIST_STR_FABRICD \
  "<kernel|connected|static|rip|ripng|ospf|ospf6|isis|bgp|eigrp|nhrp|table|vnc|babel>"
#define FRR_REDIST_HELP_STR_FABRICD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Enhanced Interior Gateway Routing Protocol (EIGRP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n"
#define FRR_IP_REDIST_STR_FABRICD \
  "<kernel|connected|static|rip|ospf|isis|bgp|eigrp|nhrp|table|vnc|babel>"
#define FRR_IP_REDIST_HELP_STR_FABRICD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Enhanced Interior Gateway Routing Protocol (EIGRP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n"
#define FRR_IP6_REDIST_STR_FABRICD \
  "<kernel|connected|static|ripng|ospf6|isis|bgp|nhrp|table|vnc|babel>"
#define FRR_IP6_REDIST_HELP_STR_FABRICD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n"

/* isisd */
#define FRR_REDIST_STR_ISISD \
  "<kernel|connected|static|rip|ripng|ospf|ospf6|bgp|eigrp|nhrp|table|vnc|babel|openfabric>"
#define FRR_REDIST_HELP_STR_ISISD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Enhanced Interior Gateway Routing Protocol (EIGRP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"
#define FRR_IP_REDIST_STR_ISISD \
  "<kernel|connected|static|rip|ospf|bgp|eigrp|nhrp|table|vnc|babel|openfabric>"
#define FRR_IP_REDIST_HELP_STR_ISISD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Enhanced Interior Gateway Routing Protocol (EIGRP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"
#define FRR_IP6_REDIST_STR_ISISD \
  "<kernel|connected|static|ripng|ospf6|bgp|nhrp|table|vnc|babel|openfabric>"
#define FRR_IP6_REDIST_HELP_STR_ISISD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"

/* nhrpd */
#define FRR_REDIST_STR_NHRPD \
  "<kernel|connected|static|rip|ripng|ospf|ospf6|isis|bgp|eigrp|table|vnc|babel|openfabric>"
#define FRR_REDIST_HELP_STR_NHRPD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Enhanced Interior Gateway Routing Protocol (EIGRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"
#define FRR_IP_REDIST_STR_NHRPD \
  "<kernel|connected|static|rip|ospf|isis|bgp|eigrp|table|vnc|babel|openfabric>"
#define FRR_IP_REDIST_HELP_STR_NHRPD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Enhanced Interior Gateway Routing Protocol (EIGRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"
#define FRR_IP6_REDIST_STR_NHRPD \
  "<kernel|connected|static|ripng|ospf6|isis|bgp|table|vnc|babel|openfabric>"
#define FRR_IP6_REDIST_HELP_STR_NHRPD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"

/* ospf6d */
#define FRR_REDIST_STR_OSPF6D \
  "<kernel|connected|static|ripng|isis|bgp|nhrp|table|vnc|babel|openfabric>"
#define FRR_REDIST_HELP_STR_OSPF6D \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"

/* ospfd */
#define FRR_REDIST_STR_OSPFD \
  "<kernel|connected|static|rip|isis|bgp|eigrp|nhrp|table|vnc|babel|openfabric>"
#define FRR_REDIST_HELP_STR_OSPFD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Enhanced Interior Gateway Routing Protocol (EIGRP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"

/* pbrd */
#define FRR_REDIST_STR_PBRD \
  "<kernel|connected|static|rip|ripng|ospf|ospf6|isis|bgp|eigrp|nhrp|table|vnc|babel|openfabric>"
#define FRR_REDIST_HELP_STR_PBRD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Enhanced Interior Gateway Routing Protocol (EIGRP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"
#define FRR_IP_REDIST_STR_PBRD \
  "<kernel|connected|static|rip|ospf|isis|bgp|eigrp|nhrp|table|vnc|babel|openfabric>"
#define FRR_IP_REDIST_HELP_STR_PBRD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Enhanced Interior Gateway Routing Protocol (EIGRP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"
#define FRR_IP6_REDIST_STR_PBRD \
  "<kernel|connected|static|ripng|ospf6|isis|bgp|nhrp|table|vnc|babel|openfabric>"
#define FRR_IP6_REDIST_HELP_STR_PBRD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"

/* ripd */
#define FRR_REDIST_STR_RIPD \
  "<kernel|connected|static|ospf|isis|bgp|eigrp|nhrp|table|vnc|babel|openfabric>"
#define FRR_REDIST_HELP_STR_RIPD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Enhanced Interior Gateway Routing Protocol (EIGRP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"

/* ripngd */
#define FRR_REDIST_STR_RIPNGD \
  "<kernel|connected|static|ospf6|isis|bgp|nhrp|table|vnc|babel|openfabric>"
#define FRR_REDIST_HELP_STR_RIPNGD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"

/* sharpd */
#define FRR_REDIST_STR_SHARPD \
  "<kernel|connected|static|rip|ripng|ospf|ospf6|isis|bgp|eigrp|nhrp|table|vnc|babel|openfabric>"
#define FRR_REDIST_HELP_STR_SHARPD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Enhanced Interior Gateway Routing Protocol (EIGRP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"
#define FRR_IP_REDIST_STR_SHARPD \
  "<kernel|connected|static|rip|ospf|isis|bgp|eigrp|nhrp|table|vnc|babel|openfabric>"
#define FRR_IP_REDIST_HELP_STR_SHARPD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Enhanced Interior Gateway Routing Protocol (EIGRP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"
#define FRR_IP6_REDIST_STR_SHARPD \
  "<kernel|connected|static|ripng|ospf6|isis|bgp|nhrp|table|vnc|babel|openfabric>"
#define FRR_IP6_REDIST_HELP_STR_SHARPD \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"

/* zebra */
#define FRR_REDIST_STR_ZEBRA \
  "<kernel|connected|static|rip|ripng|ospf|ospf6|isis|bgp|eigrp|nhrp|table|vnc|babel|openfabric>"
#define FRR_REDIST_HELP_STR_ZEBRA \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Enhanced Interior Gateway Routing Protocol (EIGRP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"
#define FRR_IP_REDIST_STR_ZEBRA \
  "<kernel|connected|static|rip|ospf|isis|bgp|eigrp|nhrp|table|vnc|babel|openfabric>"
#define FRR_IP_REDIST_HELP_STR_ZEBRA \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Enhanced Interior Gateway Routing Protocol (EIGRP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"
#define FRR_IP6_REDIST_STR_ZEBRA \
  "<kernel|connected|static|ripng|ospf6|isis|bgp|nhrp|table|vnc|babel|openfabric>"
#define FRR_IP6_REDIST_HELP_STR_ZEBRA \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n"
#define FRR_IP_PROTOCOL_MAP_STR_ZEBRA \
  "<kernel|connected|static|rip|ospf|isis|bgp|eigrp|nhrp|table|vnc|babel|openfabric|any>"
#define FRR_IP_PROTOCOL_MAP_HELP_STR_ZEBRA \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Enhanced Interior Gateway Routing Protocol (EIGRP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n" \
  "Any of the above protocols\n"
#define FRR_IP6_PROTOCOL_MAP_STR_ZEBRA \
  "<kernel|connected|static|ripng|ospf6|isis|bgp|nhrp|table|vnc|babel|openfabric|any>"
#define FRR_IP6_PROTOCOL_MAP_HELP_STR_ZEBRA \
  "Kernel routes (not installed via the zebra RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Next Hop Resolution Protocol (NHRP)\n" \
  "Non-main Kernel Routing Table\n" \
  "Virtual Network Control (VNC)\n" \
  "Babel routing protocol (Babel)\n" \
  "OpenFabric Routing Protocol\n" \
  "Any of the above protocols\n"


#ifdef FRR_DEFINE_DESC_TABLE

struct zebra_desc_table
{
    unsigned int type;
    const char* string;
    char chr;
};

#define DESC_ENTRY(T,S,C) [(T)] = { (T), (S), (C) }
static const struct zebra_desc_table route_types[] =
{
    DESC_ENTRY    (ZEBRA_ROUTE_SYSTEM,     "system",  'X' ),
    DESC_ENTRY    (ZEBRA_ROUTE_KERNEL,     "kernel",  'K' ),
    DESC_ENTRY    (ZEBRA_ROUTE_CONNECT,    "connected",   'C' ),
    DESC_ENTRY    (ZEBRA_ROUTE_STATIC,     "static",  'S' ),
    DESC_ENTRY    (ZEBRA_ROUTE_RIP,    "rip", 'R' ),
    DESC_ENTRY    (ZEBRA_ROUTE_RIPNG,  "ripng",   'R' ),
    DESC_ENTRY    (ZEBRA_ROUTE_OSPF,   "ospf",    'O' ),
    DESC_ENTRY    (ZEBRA_ROUTE_OSPF6,  "ospf6",   'O' ),
    DESC_ENTRY    (ZEBRA_ROUTE_ISIS,   "isis",    'I' ),
    DESC_ENTRY    (ZEBRA_ROUTE_BGP,    "bgp", 'B' ),
    DESC_ENTRY    (ZEBRA_ROUTE_PIM,    "pim", 'P' ),
    DESC_ENTRY    (ZEBRA_ROUTE_EIGRP,  "eigrp",   'E' ),
    DESC_ENTRY    (ZEBRA_ROUTE_NHRP,   "nhrp",    'N' ),
    DESC_ENTRY    (ZEBRA_ROUTE_HSLS,   "hsls",    'H' ),
    DESC_ENTRY    (ZEBRA_ROUTE_OLSR,   "olsr",    'o' ),
    DESC_ENTRY    (ZEBRA_ROUTE_TABLE,  "table",   'T' ),
    DESC_ENTRY    (ZEBRA_ROUTE_LDP,    "ldp", 'L' ),
    DESC_ENTRY    (ZEBRA_ROUTE_VNC,    "vnc", 'v' ),
    DESC_ENTRY    (ZEBRA_ROUTE_VNC_DIRECT,     "vnc-direct",  'V' ),
    DESC_ENTRY    (ZEBRA_ROUTE_VNC_DIRECT_RH,  "vnc-rn",  'V' ),
    DESC_ENTRY    (ZEBRA_ROUTE_BGP_DIRECT,     "bgp-direct",  'b' ),
    DESC_ENTRY    (ZEBRA_ROUTE_BGP_DIRECT_EXT,     "bgp-direct-to-nve-groups",    'e' ),
    DESC_ENTRY    (ZEBRA_ROUTE_BABEL,  "babel",   'A' ),
    DESC_ENTRY    (ZEBRA_ROUTE_SHARP,  "sharp",   'D' ),
    DESC_ENTRY    (ZEBRA_ROUTE_PBR,    "pbr", 'F' ),
    DESC_ENTRY    (ZEBRA_ROUTE_BFD,    "bfd", '-' ),
    DESC_ENTRY    (ZEBRA_ROUTE_OPENFABRIC,     "openfabric",  'f' ),
    DESC_ENTRY    (ZEBRA_ROUTE_VRRP,   "vrrp",    '-' ),
    DESC_ENTRY    (ZEBRA_ROUTE_NHG,    "zebra",   '-' ),
    DESC_ENTRY    (ZEBRA_ROUTE_SRTE,   "srte",    '-' ),
    DESC_ENTRY    (ZEBRA_ROUTE_ALL,    "wildcard",    '-' ),
};
#undef DESC_ENTRY

#endif /* FRR_DEFINE_DESC_TABLE */

#endif /* _FRR_ROUTE_TYPES_H */
