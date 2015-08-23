//============================================================================
//============================================================================
// Name        : tcp_analysis_tool.cpp
// Author      : Christos Christidis
// Copyright   : Your copyright notice
// Description : I dunno
//============================================================================

#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "../include/common.h"
#include "../include/CmdLineParser.h"
#include "../include/Database.h"
#include "../include/PCapReader.h"
#include "../include/types.h"

using namespace std;

int main(int argc, char **argv) {
    // usr_options used EVERYWHERE so made it global. Set in this function
    CmdLineParser::parse_cmdline(argc, argv);

    db.open();

    u64 num_pkts_to_read = usr_opt.count;
    for (auto iter = usr_opt.traces.begin(); iter != usr_opt.traces.end(); iter++) {
        if (num_pkts_to_read > 0) {
            PCapReader::read_trace(*iter, num_pkts_to_read);    // writes into test_file
        }
    }

    db.close();

//    start_interactive_plot_session();
    cout << "INFO: Peak virtual memory used: " << peak_vmem_used() << endl;
    stats.print_stats();

    return 0;
}
