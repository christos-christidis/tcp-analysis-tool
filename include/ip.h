#pragma once

#include "types.h"

struct ip4_hdr {
    u8 ihl :4;
    u8 version :4;
    u8 tos;
    u16 tot_len;
    u16 id;
    u16 frag_off;
    u8 ttl;
    u8 protocol;
    u16 check;
    u32 saddr;
    u32 daddr;
    // Options start here
};

#define IP_FRAG_DF      0x4000  // DF mask
#define IP_FRAG_MF      0x2000  // MF mask
#define IP_FRAG_OFFSET  0x1fff  // offset mask
