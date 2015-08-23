#include "../include/common_debug.h"

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "../include/common.h"
#include "../include/Database.h"
#include "../include/DB_structs.h"
#include "../include/FlowDirection.h"
#include "../include/TimeOffset.h"

using namespace std;

ofstream test_file;     // program writes to this to compare with tcpdump output
map<Dir_id, vector<DB_RatePair> > test_rates;
map<Dir_id, vector<DB_Packet> > test_packets;
map<Dir_id, vector<TimeOffset> > test_inter_arrival_times;
map<Dir_id, vector<DB_RTT> > test_RTTs;

void compare_with_tcpdump(const string &tmp1, const string &trace) {
    ofstream fout;
    const string tmp2 = make_temp("/tmp", fout);
    fout.close();   // we just wanted to create the file and get the path...

    const string tmp3 = make_temp("/tmp", fout);
    fout.close();   // same

    execute_cmd("tcpdump -r " + trace + " 'ip proto 6' -nS -c " + to_string(usr_opt.count)
            + " 2>/dev/null > " + tmp2, EXIT_ON_FAIL);

    // diff the files
    int ret = execute_cmd("diff " + tmp1 + " " + tmp2 + " > " + tmp3);
    execute_cmd("rm " + tmp1 + " " + tmp2, EXIT_ON_FAIL);

    if (ret == 256) {
        execute_cmd("mv " + tmp3 + " ./diffs.txt", EXIT_ON_FAIL);
        cerr << "DEBUG: Differences found, please check './diffs.txt' afterwards." << endl;
    } else {
        cout << "DEBUG: File is read correctly." << endl;
    }
}

void FlowDirection::test_db_values() const {
    vector<DB_Packet> db_packets;
    vector<DB_RatePair> db_rates;
    vector<TimeOffset> db_times;
    vector<DB_RTT> db_RTTs;

    db.get_packets(id, db_packets);
    if (db_packets != test_packets[id])
        exit_with("DEBUG: Database values are not correct!");

    db.get_rates(id, db_rates);
    if (db_rates != test_rates[id])
        exit_with("DEBUG: Database values are not correct!");

    db.get_inter_arrival_times(id, db_times);
    if (db_times != test_inter_arrival_times[id])
        exit_with("debug: database values are not correct!");

    db.get_RTTs(id, db_RTTs);
    if (db_RTTs != test_RTTs[id])
        exit_with("DEBUG: Database values are not correct!");
}
