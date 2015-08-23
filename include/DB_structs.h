#pragma once

#include "FlowDirection.h"
#include "types.h"
#include "TimeOffset.h"

class Packet;

struct DB_Packet {
    Dir_id dir_id;
    TimeOffset time;
    u16 ip_id;
    u32 seq, ack;
    u8 tcp_flags;

    DB_Packet();
    DB_Packet(Dir_id dir_id, TimeOffset &time, u16 ip_id, u32 seq, u32 ack, u8 tcp_flags);
    DB_Packet(Dir_id dir_id, const Packet &pkt);
};

struct DB_RatePair {
    Dir_id dir_id;
    TimeOffset time;
    u32 throughput;
    u32 goodput;

    DB_RatePair();
    DB_RatePair(Dir_id dir_id, const TimeOffset &time, u32 tput, u32 goodput);
};

struct DB_RTT {
    Dir_id dir_id;
    TimeOffset time;
    u32 rtt;

    DB_RTT();
    DB_RTT(Dir_id dir_id, const TimeOffset &time, u32 rtt);
};

bool operator==(const DB_Packet &p1, const DB_Packet &p2);
bool operator==(const DB_RatePair &rp1, const DB_RatePair &rp2);
bool operator==(const DB_RTT &r1, const DB_RTT &r2);
