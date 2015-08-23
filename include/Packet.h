#pragma once

#include <iostream>
#include <string>

#include "IpData.h"
#include "types.h"
#include "TcpData.h"
#include "TimeOffset.h"

struct FiveTuple;

class Packet {
public:
    TcpData tcp;
    IpData ip;
    u16 tcp_data_len;
    TimeOffset t_off;

    Packet(const IpData &ip_data, const TcpData &tcp_data, const TimeOffset &t_off) :
            tcp(tcp_data), ip(ip_data), t_off(t_off) {
        tcp_data_len = ip.datagram_len - ip.offset_to_tcp - tcp.data_offset;
    }

    bool FIN() const;
    bool SYN() const;
    bool RST() const;
    bool PUSH() const;
    bool ACK() const;
    bool URG() const;
    bool ECE() const;
    bool CWR() const;

    bool has_timestamps() const;
    TimeStamp TS_val() const;
    TimeStamp TS_ecr() const;

    bool has_data() const;
    bool is_pure_ACK() const;

    FiveTuple get_tuple() const;
    bool has_dirA() const;
    bool has_dirB() const;

    /*
     * Packet printing methods
     */
    std::string time_to_str() const;
    std::string flags_to_str() const;
    std::string tcp_options_to_str() const;
};

/*
 * now I can do cout << Packet << endl;
 */
std::ofstream & operator<<(std::ofstream &out, const Packet &pkt);
