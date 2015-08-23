#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "DB_structs.h"
#include "FlowDirection.h"

/*
 * Program writes to test_file and uses test_XXX to test against parsing and db respectively
 */
extern std::ofstream test_file;

extern std::map<Dir_id, std::vector<DB_RatePair> > test_rates;
extern std::map<Dir_id, std::vector<DB_Packet> > test_packets;
extern std::map<Dir_id, std::vector<TimeOffset> > test_inter_arrival_times;
extern std::map<Dir_id, std::vector<DB_RTT> > test_RTTs;

void compare_with_tcpdump(const std::string &tmp1, const std::string &trace);
void test_db_values();
