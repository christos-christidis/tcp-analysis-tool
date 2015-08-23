#include "../include/FlowDirection.h"

#include <vector>

#include "../include/common.h"
#include "../include/common_debug.h"
#include "../include/CacheManager.h"
#include "../include/Database.h"
#include "../include/DB_structs.h"
#include "../include/Packet.h"

using namespace std;

Dir_id global_dir_id;

FlowDirection::FlowDirection() :
        id(global_dir_id++), bytes(), tcp_bytes(), num_packets(),
                sample_bytes(), sample_tcp_bytes() {
}

void FlowDirection::add_packet(const Packet &pkt) {
    if (num_packets != 0) {
        TimeOffset new_inter_arrival_time = pkt.t_off - latest_arrival_t;
        // we pass number of inter_arrival time, eg 2nd one will be when num_packets = 3
        cache_manager.store(id, num_packets - 1, new_inter_arrival_time);
    }

    latest_arrival_t = pkt.t_off;

    check_if_time_for_rate_sample(pkt);

    num_packets++;
    bytes += pkt.ip.datagram_len;
    tcp_bytes += pkt.tcp_data_len;

    sample_bytes += pkt.ip.datagram_len;
    sample_tcp_bytes += pkt.tcp_data_len;

    db.store_packet(DB_Packet(id, pkt));
#ifdef DEBUG
    test_packets[id].push_back(DB_Packet(id, pkt));
#endif // DEBUG
}

void FlowDirection::check_if_time_for_rate_sample(const Packet &pkt) {
    if (num_packets == 0)
        last_sample_t = pkt.t_off;

    size_t needed_samples = (pkt.t_off - last_sample_t) / usr_opt.sampling_rate;

    if (needed_samples > 0)
        take_sample(pkt, needed_samples);
}

void FlowDirection::take_sample(const Packet &pkt, size_t n_samples) {
    u32 throughput = sample_bytes / usr_opt.sampling_rate.to_sec();
    u32 goodput = sample_tcp_bytes / usr_opt.sampling_rate.to_sec();

    db.store_rates(DB_RatePair(id, pkt.t_off, throughput, goodput));
#ifdef DEBUG
    test_rates[id].push_back(DB_RatePair(id, pkt.t_off, throughput, goodput));
#endif // DEBUG

    sample_bytes = 0;
    sample_tcp_bytes = 0;

    last_sample_t += n_samples * usr_opt.sampling_rate;

    while (--n_samples) {
        db.store_rates(DB_RatePair(id, pkt.t_off, 0, 0));
#ifdef DEBUG
        test_rates[id].push_back(DB_RatePair(id, pkt.t_off, 0, 0));
#endif // DEBUG
    }
}
