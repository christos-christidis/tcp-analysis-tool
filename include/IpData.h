#pragma once

#include <string>
#include "ip.h"

struct IpData {
    u8 family = 0;  // NOT the same as IP version, this is AF_INET etc
    u16 datagram_len = 0;
    u16 id = 0;
    u8 flags = 0;
    u16 frag_offset = 0;
    u16 offset_to_tcp = 0;
    u8 ttl = 0;
    std::string src_addr, dst_addr;

    IpData(const u8 *);

    bool is_fragment() const;
    bool MF() const;
    bool DF() const;

private:
    void extract_ip_fields(const ip4_hdr *);
};

// used for setting IpData flags
#define IP_DATA_MF 0x01
#define IP_DATA_DF 0x02
