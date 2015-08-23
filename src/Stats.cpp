#include "../include/Stats.h"

#include <iostream>

using namespace std;

Stats::Stats() :
        num_bytes(), num_pkts(), num_flows(), num_frag_pkts() {
}

void Stats::print_stats() {
    cout << endl << "Printing stats..." << endl
            << num_bytes << " bytes" << endl
            << num_pkts << " packets, of which:" << endl
            << "  " << num_frag_pkts << " fragmented ("
            << num_frag_pkts * 100. / num_pkts << "%) " << endl
            << num_flows << " flows" << endl;
}
