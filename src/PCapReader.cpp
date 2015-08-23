#include "../include/PCapReader.h"

#include <sys/time.h>
#include <iostream>

#include "../include/common_debug.h"
#include "../include/Stats.h"

extern "C" {
#include <pcap/pcap.h>
#include <pcap/bpf.h>
}

#include <cstring>
#include <ctime>

#include "FlowMap.h"
#include "TimeOffset.h"
#include "Packet.h"
#include "common.h"
#include "FiveTuple.h"
#include "IpData.h"
#include "lowlevel_pcap.h"
#include "TcpData.h"
#include "types.h"
#include "timeval_ops.h"

using namespace std;

/*
 * pcap_dispatch() requires this signature, so it can't be member of PCapReader
 */
static void process_packet(u8 *, const pcap_pkthdr *, const u8 *);

void PCapReader::read_trace(const string &file, u64 &num_pkts_to_read) {
    static char errbuf[PCAP_ERRBUF_SIZE];   // used for error messages

    cout << "Reading '" << file << "'" << endl;

    pcap_t *handle = pcap_open_offline(file.c_str(), errbuf);
    if (handle == nullptr)
        exit_with(errbuf);

    // compile filter and attach to handle
    bpf_program bpf;
    if (pcap_compile(handle, &bpf, usr_opt.filter.c_str(), 1, PCAP_NETMASK_UNKNOWN)
            == -1)
        exit_with(pcap_geterr(handle));

    if (pcap_setfilter(handle, &bpf) == -1)
        exit_with(pcap_geterr(handle));

    pcap_freecode(&bpf);    // free memory used by compiled filter

    // raw data is always in network order, but pcap headers (per packet and global header)
    // are in host order (whoever made the capture)
    change_hdr_byte_order = pcap_is_swapped(handle);

    clock_t t = clock();

    int n = 0;
    if ((n = pcap_dispatch(handle, num_pkts_to_read, process_packet, (u8 *) handle)) < 0)
        exit_with(pcap_geterr(handle));

    num_pkts_to_read -= n;

    const double sec = (clock() - t) / (double) CLOCKS_PER_SEC;
    cout << "INFO: Processing rate: " << n / sec << " pkts/s." << endl;
}

/*
 * This function is called from pcap_dispatch() every time a packet is matched by filter
 */
static void process_packet(u8 *user_info, const pcap_pkthdr *per_pkt_hdr, const u8 *raw_data) {
    int pkt_bytes_left = bswap32(per_pkt_hdr->caplen);

    pcap_t *handle = (pcap_t *) user_info;
    const int dl_hdr_size = datalink_header_size(handle);
    raw_data += dl_hdr_size, pkt_bytes_left -= dl_hdr_size;

    UPDATE_STATS(num_pkts, 1);

    // dropping packet due to incomplete IP header
    if (pkt_bytes_left < 20)
        return;

    IpData ip_data(raw_data);

    // IPv4 fragments are almost non-existent for TCP, so should have only minimal effect
    // on rate, average pkt size, rtt etc. Only keep 1st fragment of each TCP pkt
    if (ip_data.frag_offset > 0) {
        UPDATE_STATS(num_frag_pkts, 1);
        return;
    }

    if (ip_data.MF())
        UPDATE_STATS(num_frag_pkts, 1);

    raw_data += ip_data.offset_to_tcp, pkt_bytes_left -= ip_data.offset_to_tcp;

    // dropping packet due to incomplete basic TCP header (options may be captured or not)
    if (pkt_bytes_left < 20)
        return;

    TcpData tcp_data(raw_data, pkt_bytes_left);

    // get capture time
    timeval packet_time = {
            bswap32(per_pkt_hdr->ts.tv_sec),
            bswap32(per_pkt_hdr->ts.tv_usec)
    };

    if (!timerisset(&capture_start_time))
        capture_start_time = packet_time;

    TimeOffset from_start = packet_time - capture_start_time;

#ifdef DEBUG
    test_file << Packet(ip_data, tcp_data, from_start);
#endif  // DEBUG

    flow_map.map_to_flow(Packet(ip_data, tcp_data, from_start));

    PCapReader::print_progress();
}

void PCapReader::print_progress() {
    static u64 num_read = 0;
    static const int one_hundred_k = 100000;

    if (++num_read % one_hundred_k == 0) {
        cout << num_read / one_hundred_k / 10. << "M packets read ("
                << num_read * 100. / usr_opt.count << "%)" << endl;
    }
}
