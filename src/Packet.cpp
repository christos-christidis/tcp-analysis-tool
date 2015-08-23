#include "../include/Packet.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <cstddef>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

#include "../include/common.h"
#include "../include/FiveTuple.h"
#include "../include/tcp.h"
#include "../include/timeval_ops.h"

struct TimeStamp;

using namespace std;

bool Packet::FIN() const {
    return tcp.flags & TCP_FIN;
}
bool Packet::SYN() const {
    return tcp.flags & TCP_SYN;
}
bool Packet::RST() const {
    return tcp.flags & TCP_RST;
}
bool Packet::PUSH() const {
    return tcp.flags & TCP_PUSH;
}
bool Packet::ACK() const {
    return tcp.flags & TCP_ACK;
}
bool Packet::URG() const {
    return tcp.flags & TCP_URG;
}
bool Packet::ECE() const {
    return tcp.flags & TCP_ECE;
}
bool Packet::CWR() const {
    return tcp.flags & TCP_CWR;
}

bool Packet::has_timestamps() const {
    const TcpOptions &opts = tcp.options;

    size_t idx = opts.get_idx_in_signature(TCP_TIMESTAMP);

    if (idx != opts.signature.size() && opts.lengths[idx] != 0)
        return true;

    return false;
}

TimeStamp Packet::TS_val() const {
    if (has_timestamps())
        return tcp.options.timestamps[0];

    exit_with("ERR: check before taking TS_val()");
    return 0;
}

TimeStamp Packet::TS_ecr() const {
    if (has_timestamps())
        return tcp.options.timestamps[1];

    exit_with("ERR: check before taking TS_ecr()");
    return 0;
}

bool Packet::has_data() const {
    return tcp_data_len;
}

// TODO: fix this..
bool Packet::is_pure_ACK() const {
    return !has_data() && !SYN() && !FIN();
}

FiveTuple Packet::get_tuple() const {
    return FiveTuple(ip.family, ip.src_addr, ip.dst_addr, tcp.src_port, tcp.dst_port);
}

/*
 * Since each tuple is included only once in map, it follows that all packets seen
 * will have either the direction of the tuple (dirA) or the opposite (dirB)
 */
bool Packet::has_dirA() const {
    return this->get_tuple() == currentTuple;
}
bool Packet::has_dirB() const {
    return !has_dirA();
}

/*
 * Packet printing methods start here
 */
std::string Packet::time_to_str() const {
    char time_buf[10];
    int chars_written;

    timeval real_time = capture_start_time + t_off;
    tm time = *localtime((const time_t *) &real_time.tv_sec);
    if ((chars_written = strftime(time_buf, 10, "%T.", &time)) == 0)
        exit_with("too small buffer when writing Packet");

    ostringstream ss;
    ss << time_buf << setw(6) << setfill('0') << real_time.tv_usec;
    return ss.str();
}

std::string Packet::flags_to_str() const {
    static const string flag_letters = "FSRP.UEW";
    u8 tcp_flags = tcp.flags;

    string s;
    for (int i = 0; i < 8; i++) {
        if (tcp_flags & 0x01)
            s += flag_letters[i];
        tcp_flags >>= 1;
    }

    return s;
}

string Packet::tcp_options_to_str() const {
    const TcpOptions &options = tcp.options;
    const vector<u8> &signature = options.signature;

    string s;
    if (signature.size()) {
        s = ", options [";
        int idx = 0;    // used for unknown options
        for (u8 i = 0; i < signature.size(); i++) {
            if (i != 0)
                s += ",";
            switch (signature[i]) {
                case TCP_EOL:
                    s += "eol";
                    break;
                case TCP_NOP:
                    s += "nop";
                    break;
                case TCP_MSS:
                    s += "mss";
                    if (options.lengths[i])
                        s += " " + to_string(options.mss);
                    break;
                case TCP_WSCALE:
                    s += "wscale";
                    if (options.lengths[i])
                        s += " " + to_string(options.wscale);
                    break;
                case TCP_SACK_PERM:
                    s += "sackOK";
                    break;
                case TCP_SACK:
                    s += "sack " + to_string(options.sacks_optlen) + " ";
                    for (u8 j = 0; j < options.lengths[i]; j++) {
                        s += "{" + to_string(options.sacks[2 * j]) + ":"
                                + to_string(options.sacks[2 * j + 1]) + "}";
                    }
                    break;
                case TCP_TIMESTAMP:
                    s += "TS";
                    if (options.lengths[i]) {
                        s += " val " + to_string(options.timestamps[0].val) +=
                                " ecr " + to_string(options.timestamps[1].val);
                    }
                    break;
                case TCP_MD5:
                    s += "md5 - ";
                    {
                        ostringstream ss;
                        for (int len = options.lengths[i]; len > 0; len--) {
                            ss << hex << setfill('0') << setw(2)
                                    << (int) options.unknown_opt_vals[idx++];
                        }
                        s += ss.str();
                    }
                    break;
                default:
                    s += "unknown-" + to_string(signature[i]) + " 0x";
                    {
                        ostringstream ss;
                        for (int len = options.lengths[i]; len > 0; len--) {
                            ss << hex << setfill('0') << setw(2)
                                    << (int) options.unknown_opt_vals[idx++];
                        }
                        s += ss.str();
                    }
                    break;
            }
        }
    }

    if (tcp.options_truncated)
        s += "[|tcp]>";
    else if (signature.size())
        s += "]";

    return s;
}

ofstream & operator<<(ofstream &out, const Packet &pkt) {
    ostringstream ss;

// for clarity
    const IpData &ip = pkt.ip;
    const TcpData &tcp = pkt.tcp;

    ss << pkt.time_to_str();
    ss << (ip.family == AF_INET ? " IP " : " IP6 ");

    char ip_str_buf[INET6_ADDRSTRLEN];
    ss << inet_ntop(ip.family, ip.src_addr.c_str(), ip_str_buf, INET6_ADDRSTRLEN)
            << "." << tcp.src_port << " > ";
    ss << inet_ntop(ip.family, ip.dst_addr.c_str(), ip_str_buf, INET6_ADDRSTRLEN)
            << "." << tcp.dst_port;

    ss << ": Flags [" << pkt.flags_to_str() << "]";

    if (pkt.has_data() || pkt.FIN() || pkt.SYN() || pkt.RST()) {
        ss << ", seq " << tcp.seq;
        if (pkt.has_data())
            ss << ":" << (tcp.seq + pkt.tcp_data_len);
    }

    if (pkt.ACK())
        ss << ", ack " << tcp.ack;

    ss << ", win " << tcp.window;

    ss << pkt.tcp_options_to_str();

    if (!tcp.options_truncated)
        ss << ", length " << pkt.tcp_data_len;

    out << ss.str() << endl;

    return out;
}
