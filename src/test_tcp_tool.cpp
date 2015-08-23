//============================================================================
// Name        : test_tcp_tool.cpp
// Author      : Christos Christidis
// Copyright   : Your copyright notice
// Description : I dunno
//============================================================================

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "common.h"
#include "common_debug.h"
#include "CmdLineParser.h"
#include "Database.h"
#include "Flow.h"
#include "FlowMap.h"
#include "PCapReader.h"
#include "Stats.h"
#include "types.h"

using namespace std;

int main(int argc, char **argv) {
    // usr_options used EVERYWHERE so made it global. Set in this function
    CmdLineParser::parse_cmdline(argc, argv);

    db.open();

    u64 num_pkts_to_read = usr_opt.count;
    for (auto iter = usr_opt.traces.begin(); iter != usr_opt.traces.end(); iter++) {
        if (num_pkts_to_read > 0) {
            const string tmp = make_temp("/tmp", test_file);
            PCapReader::read_trace(*iter, num_pkts_to_read);    // writes into test_file
            test_file.close();

            cout << "DEBUG: Comparing data with tcpdump output..." << endl;
            compare_with_tcpdump(tmp, *iter);
        }
    }

    cout << "DEBUG: Comparing stored db values with retrieved ones..." << endl;
    test_db_values();
    cout << "DEBUG: Database values are read and written correctly." << endl;
    db.close();

//    start_interactive_plot_session();
    cout << "INFO: Peak virtual memory used: " << peak_vmem_used() << endl;
    stats.print_stats();

    return 0;
}

void test_db_values() {
    for (auto &tuple_flow_vec_pair : flow_map) {
        auto &flows = tuple_flow_vec_pair.second;
        for (auto &flow : flows) {
            flow.dirA.test_db_values();
            flow.dirB.test_db_values();
        }
    }
}
