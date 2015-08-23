#include "../include/TcpData.h"

#include <netinet/in.h>
#include <algorithm>

#include "../include/tcp.h"

using namespace std;

TcpData::TcpData(const u8 *raw_data, int bytes_left) {
    extract_tcp_fields((const tcp_hdr *) raw_data);
    raw_data += 20, bytes_left -= 20;
    extract_tcp_options(raw_data, bytes_left);
}

void TcpData::extract_tcp_fields(const tcp_hdr *tcp) {
    src_port = ntohs(tcp->source);
    dst_port = ntohs(tcp->dest);
    seq = ntohl(tcp->seq);
    ack = ntohl(tcp->ack_seq);
    data_offset = tcp->doff * 4;
    flags = tcp->flags;
    window = ntohs(tcp->window);
}

void TcpData::extract_tcp_options(const u8 *raw_data, int bytes_remaining) {
    options_truncated = data_offset - 20 > bytes_remaining;
    int bytes_left = min(data_offset - 20, bytes_remaining);

    while (bytes_left > 0) {
        u8 kind = *raw_data++;
        bytes_left--;

        options.signature.push_back(kind);
        options.lengths.push_back(0);

        if (kind == TCP_EOL) {
            return;
        }

        // this and EOL have no optlen byte
        if (kind == TCP_NOP) {
            continue;
        }

        // consume optlen byte
        u8 optlen;
        if (bytes_left > 0) {
            optlen = *raw_data++;
            bytes_left--;
        } else
            return;

        switch (kind) {
            case TCP_MSS:
                if (bytes_left >= 2) {
                    options.mss = ntohs(*(u16 *) raw_data);
                    options.lengths.back() = 1;
                }
                raw_data += 2, bytes_left -= 2;
                break;
            case TCP_WSCALE:
                if (bytes_left >= 1) {
                    options.wscale = *raw_data;
                    options.lengths.back() = 1;
                }
                raw_data += 1, bytes_left -= 1;
                break;
            case TCP_SACK_PERM:
                break;
            case TCP_SACK:
                options.sacks_optlen = optlen / 8;
                for (int i = 0; i < optlen / 8; i++) {
                    if (bytes_left >= 8) {
                        options.sacks[2 * i] = ntohl(*((u32 *) raw_data));
                        options.sacks[2 * i + 1] = ntohl(*((u32 *) raw_data + 1));
                        options.lengths.back() += 1;
                    }
                    raw_data += 8, bytes_left -= 8;
                }
                break;
            case TCP_TIMESTAMP:
                if (bytes_left >= 8) {
                    options.timestamps[0] = ntohl(*((u32 *) raw_data));
                    options.timestamps[1] = ntohl(*((u32 *) raw_data + 1));
                    options.lengths.back() = 1;
                }
                raw_data += 8, bytes_left -= 8;
                break;
            default:
                for (int i = 0; i < min(optlen - 2, bytes_left); i++) {
                    options.unknown_opt_vals.push_back(raw_data[i]);
                    options.lengths.back() += 1;
                }
                raw_data += optlen - 2, bytes_left -= optlen - 2;
                break;
        }
    }
}

size_t TcpOptions::get_idx_in_signature(u8 code) const {
    size_t idx;
    for (idx = 0; idx < signature.size(); idx++) {
        if (signature[idx] == code)
            return idx;
    }

    return idx; // equal to size of signature
}
