#include "../include/CmdLineParser.h"

#include <cctype>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <vector>

#include "../include/common.h"
#include "../include/tclap/ArgException.h"
#include "../include/tclap/CmdLine.h"
#include "../include/tclap/MultiArg.h"
#include "../include/tclap/UnlabeledMultiArg.h"
#include "../include/tclap/ValueArg.h"

using namespace std;

void CmdLineParser::parse_cmdline(int argc, char** argv) {
    try {
        TCLAP::CmdLine cmd("Analyse TCP connections from passive traces", ' ', "0.1");

//        TCLAP::ValueArg<int> num_flows_to_plot("n", "num-flows", "number of flows to plot",
//                false, 1, "num_flows", cmd);
//        TCLAP::ValueArg<int> min_pkts_to_plot("m", "min-pkts", "plot flows with >= m packets",
//                false, 1000, "min-pkts", cmd);
        TCLAP::ValueArg<string> count("c", "count", "max packets to read",
                false, "0", "pkt-count", cmd);
        TCLAP::ValueArg<string> timeout("t", "timeout", "hang up after this time",
                false, "15s", "timeout", cmd);
        TCLAP::ValueArg<int> sampling_rate("s", "sampling-rate", "sample every X ms",
                false, 200, "sample-rate", cmd);
        TCLAP::ValueArg<string> db_file("", "db", "database file for storing data",
                true, "", "db_file", cmd);
        TCLAP::UnlabeledMultiArg<string> traces("traces", "Internet packet traces",
                false, "passive_traces", cmd, false);

        cmd.parse(argc, argv);

//        usr_options.num_flows_to_plot = num_flows_to_plot.getValue();
//        usr_options.min_pkts_to_plot = min_pkts_to_plot.getValue();
        usr_opt.filter = set_filter();
        usr_opt.count = parse_count(count.getValue());
        usr_opt.timeout = parse_timeout(timeout.getValue());
        usr_opt.sampling_rate = Duration::from_msec(sampling_rate.getValue());
        usr_opt.db = db_file.getValue();
        usr_opt.traces = traces.getValue();

        check_args();
    } catch (TCLAP::ArgException &e) {
        exit_with("ERROR: " + e.error() + " for arg " + e.argId());
    }
}

/*
 * TODO: This may be used to set filter via flags later. Now only captures IPv4 pkts with TCP
 * in their proto field. IPv4 tunneling and IPv6 in general require following the protocol chain
 * (protochain). Now all fragments are caught (and ignored except the 1st), in the case of IPv6
 * the fragments would have to be caught with special mask as TCP would be contained only in 1st
 */
string CmdLineParser::set_filter() {
    return "ip proto 6";
}

/*
 * This technique may be slower but it's more compact than the equivalent switch
 */
u64 CmdLineParser::parse_count(std::string &s) {
    u64 count = 0;
    size_t idx;

    try {
        count = std::stoul(s, &idx);
    } catch (exception &e) {
        exit_with(string("Error reading -c count: ") + e.what());
    }

    // if there's a letter following
    if (idx < s.size()) {
        idx = string("kmgt").find(std::tolower(s[idx]));

        if (idx == string::npos)
            exit_with(string("ERR: Wrong character read while parsing -c count: ") + s[idx]);

        for (size_t i = 0; i < idx + 1; i++)
            count *= 1e3;
    }

    return count;
}

/*
 * Up to 65535 secs (1092 minutes or 18hrs), if larger overflows
 */
u16 CmdLineParser::parse_timeout(std::string &s) {
    u16 secs = 0;
    size_t idx;

    try {
        secs = std::stoi(s, &idx);
    } catch (exception &e) {
        exit_with(string("Error reading -t timeout: ") + e.what());
    }

    // if there's a letter following
    if (idx < s.size()) {
        idx = string("smh").find(s[idx]);

        if (idx == string::npos)
            exit_with(string("ERR: Wrong character read while parsing -t timeout: ") + s[idx]);

        for (size_t i = 0; i < idx; i++)
            secs *= 60;
    }

    return secs;
}

/*
 * Files are huge. We wouldn't want error to happen after we've read a bunch!
 */
void CmdLineParser::check_args() {
    check_traces();

    if (usr_opt.traces.size() == 0) {
        usr_opt.read_from_db = true;
        check_that_db_exists();
    }
}

void CmdLineParser::check_traces() {
    for (auto iter = usr_opt.traces.begin(); iter != usr_opt.traces.end(); iter++) {
        if (!file_exists(*iter)) {
            exit_with("Error when checking file '" + *iter + "': " + strerror(last_errno));
        }
    }
}

void CmdLineParser::check_that_db_exists() {
    if (!file_exists(usr_opt.db)) {
        exit_with("Error when checking db file '" + usr_opt.db + "': " + strerror(last_errno));
    }
// this should also check for existence of actual db in file
}
