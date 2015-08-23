#pragma once

#include <stddef.h>
#include <vector>

#include "types.h"
#include "TimeStamp.h"

struct tcp_hdr;

struct TcpOptions {
    u16 mss;
    u8 wscale;
    u32 sacks[8];
    TimeStamp timestamps[2];

    std::vector<u8> signature;

    /*
     * Lengths needed for options with value; sackOK is valid iff it's included in the signature,
     * but MSS may not have its value (eg truncated due to snaplen). Lengths are intuitive, ie
     * mss/wscale len = 1 if value exists, sacks' len = 2 if 2 segments (ie 4 edges were seen) etc.
     * HOWEVER for compatibility with tcpdump, timestamps' = 1 if both TS_val and TS_ecr, else 0.
     */
    std::vector<u8> lengths;

    u8 sacks_optlen;    // optlen seen on the wire, needed for diff with tcpdump output
    std::vector<u8> unknown_opt_vals;   // again needed for diff :(

    size_t get_idx_in_signature(u8 code) const;
};

struct TcpData {
    u16 src_port, dst_port;
    u32 seq, ack;
    u8 data_offset;
    u8 flags;
    u16 window;

    TcpOptions options;
    bool options_truncated; // needed only for diff with tcpdump output

    TcpData(const u8 *raw_data, int bytes_left);

    void extract_tcp_options(const u8 *raw_data, int bytes_left);

private:
//    add_option();
    void extract_tcp_fields(const tcp_hdr *);
    void add_option(u8, u8, const u8 *);
};
