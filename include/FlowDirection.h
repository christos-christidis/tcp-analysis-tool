#pragma once

#include <cstddef>

#include "types.h"
#include "TimeOffset.h"

class Packet;

typedef u32 Dir_id;

struct FlowDirection {
    Dir_id id;
    u64 bytes;
    u64 tcp_bytes;
    u32 num_packets;
    TimeOffset latest_arrival_t;

    // needed for rate samples
    TimeOffset last_sample_t;
    u32 sample_bytes;
    u32 sample_tcp_bytes;

    FlowDirection();

    void add_packet(const Packet &pkt);
#ifdef DEBUG
    void test_db_values() const;
#endif // DEBUG

private:
    void check_if_time_for_rate_sample(const Packet &pkt);
    void take_sample(const Packet &pkt, size_t n_samples);
};
