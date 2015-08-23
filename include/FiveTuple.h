#pragma once

#include <string>

#include "types.h"

// needed to match tuples to flows inside the db (and only there)
extern u32 next_tuple_id;

struct FiveTuple {
    u8 family = 0;
    std::string src_addr, dst_addr;
    u16 src_port = 0, dst_port = 0;

    FiveTuple() = default;

    FiveTuple(u8 family, const std::string &src_addr, const std::string &dst_addr,
            u16 src_port, u16 dst_port) :
            family(family), src_addr(src_addr), dst_addr(dst_addr),
            src_port(src_port), dst_port(dst_port) {
    }

    FiveTuple reverse() const;

    // used by '<' to order tuples in the map
    std::string to_string() const;
};

// called when inserting into map (I don't use unordered_map, takes up more memory)
int operator<(const FiveTuple &t1, const FiveTuple &t2);
bool operator==(const FiveTuple &t1, const FiveTuple &t2);
