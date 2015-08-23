#include "../include/DB_structs.h"

#include "../include/Packet.h"

DB_Packet::DB_Packet() :
        dir_id(), time(), ip_id(), seq(), ack(), tcp_flags() {
}

DB_Packet::DB_Packet(Dir_id dir_id, TimeOffset &time, u16 ip_id, u32 seq, u32 ack, u8 flags) :
        dir_id(dir_id), time(time), ip_id(ip_id), seq(seq), ack(ack), tcp_flags(flags) {
}

DB_Packet::DB_Packet(Dir_id dir_id, const Packet &pkt) :
        dir_id(dir_id), time(pkt.t_off), ip_id(pkt.ip.id),
                seq(pkt.tcp.seq), ack(pkt.tcp.ack), tcp_flags(pkt.tcp.flags) {
}

bool operator==(const DB_Packet &p1, const DB_Packet &p2) {
    return p1.dir_id == p2.dir_id && p1.time == p2.time && p1.ip_id == p2.ip_id &&
            p1.seq == p2.seq && p1.ack == p2.ack && p1.tcp_flags == p2.tcp_flags;
}

DB_RatePair::DB_RatePair() :
        dir_id(), time(), throughput(), goodput() {
}

DB_RatePair::DB_RatePair(Dir_id dir_id, const TimeOffset &time, u32 th_put, u32 g_put) :
        dir_id(dir_id), time(time), throughput(th_put), goodput(g_put) {
}

bool operator==(const DB_RatePair &rp1, const DB_RatePair &rp2) {
    return rp1.dir_id == rp2.dir_id && rp1.time == rp2.time &&
            rp1.throughput == rp2.throughput && rp1.goodput == rp2.goodput;
}

DB_RTT::DB_RTT() :
        dir_id(), time(), rtt() {
}

DB_RTT::DB_RTT(Dir_id dir_id, const TimeOffset &time, u32 rtt) :
        dir_id(dir_id), time(time), rtt(rtt) {
}

bool operator==(const DB_RTT &r1, const DB_RTT &r2) {
    return r1.dir_id == r2.dir_id && r1.time == r2.time && r1.rtt == r2.rtt;
}
