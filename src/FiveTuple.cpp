#include "../include/FiveTuple.h"

#include <sstream>

using namespace std;

u32 next_tuple_id = 1;

FiveTuple FiveTuple::reverse() const {
    return FiveTuple(family, dst_addr, src_addr, dst_port, src_port);
}

string FiveTuple::to_string() const {
    ostringstream ss;
    ss << family << src_addr << dst_addr << src_port << dst_port;
    return ss.str();
}

int operator<(const FiveTuple &t1, const FiveTuple &t2) {
    return t1.to_string() < t2.to_string();
}

bool operator==(const FiveTuple &t1, const FiveTuple &t2) {
    return t1.family == t2.family && t1.src_addr == t2.src_addr && t1.dst_addr == t2.dst_addr
            && t1.src_port == t2.src_port && t1.dst_port == t2.dst_port;
}
