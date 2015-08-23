#include "lowlevel_pcap.h"

extern "C" {
#include <pcap/bpf.h>
#include <pcap/pcap.h>
}

#include "common.h"
#include "types.h"

using namespace std;

bool change_hdr_byte_order; // if true, pcap headers' bytes must be swapped

u16 bswap16(u16 x) {
    return change_hdr_byte_order ? __builtin_bswap16(x) : x;
}

u32 bswap32(u32 x) {
    return change_hdr_byte_order ? __builtin_bswap32(x) : x;
}

int datalink_header_size(pcap_t *handle) {
    int data_link_type, num_bytes = 0;

    if ((data_link_type = pcap_datalink(handle)) < 0)
        exit_with(pcap_geterr(handle));

    switch (data_link_type) {
        case DLT_NULL:
            num_bytes = 4;
            break;
        case DLT_EN10MB:
            num_bytes = 14;
            break;
        case 12:
        case 14:
            // Raw IP
            num_bytes = 0;
            break;
        default:
            exit_with("ERROR: link type not recognised in skip_datalink_hdr()");
    }
    return num_bytes;
}
