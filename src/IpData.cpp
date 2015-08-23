#include "IpData.h"

#include <netinet/in.h>
#include <sys/socket.h>

#include "common.h"
#include "Stats.h"
#include "types.h"

using namespace std;

IpData::IpData(const u8 *data) {
    const ip4_hdr *ip = (const ip4_hdr *) data;

    if (ip->version == 4) {
        extract_ip_fields(ip);
        UPDATE_STATS(num_bytes, datagram_len);
    } else if (ip->version == 6)
        exit_with("Sorry, IPv6 not implemented yet!");
    else
        exit_with("Unknown network protocol in IpData constructor!");
}

void IpData::extract_ip_fields(const ip4_hdr *ip) {
    family = AF_INET;
    datagram_len = ntohs(ip->tot_len);
    id = ntohs(ip->id);
    frag_offset = ntohs(ip->frag_off);
    if (frag_offset & IP_FRAG_DF)
        flags |= IP_DATA_DF;
    if (frag_offset & IP_FRAG_MF)
        flags |= IP_DATA_MF;
    frag_offset &= IP_FRAG_OFFSET;
    offset_to_tcp = frag_offset ? 0 : ip->ihl * 4;
    ttl = ip->ttl;
    src_addr = string((const char *) &(ip->saddr), 4);
    dst_addr = string((const char *) &(ip->daddr), 4);
}

bool IpData::is_fragment() const {
    return (flags & IP_DATA_MF) || frag_offset;
}

bool IpData::MF() const {
    return flags & IP_DATA_MF;
}

bool IpData::DF() const {
    return flags & IP_DATA_DF;
}
