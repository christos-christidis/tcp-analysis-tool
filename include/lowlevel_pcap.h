#pragma once

extern "C" {
#include <pcap/pcap.h>
}

#include "types.h"

// see impl file for details on this global
extern bool change_hdr_byte_order;

// change byte order of fields
u16 bswap16(u16 x);
u32 bswap32(u32 x);

int datalink_header_size(pcap_t *handle);
