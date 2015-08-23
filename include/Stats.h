#pragma once

#include "types.h"

#define UPDATE_STATS(x, num) stats.x += (num)

struct Stats {
    u64 num_bytes;
    u64 num_pkts;
    u64 num_flows;
    u64 num_frag_pkts;

    Stats();

    void print_stats();
};
