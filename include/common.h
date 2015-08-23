#pragma once

#include <sys/time.h>
#include <iostream>
#include <string>
#include <vector>

#include "CacheManager.h"
#include "Database.h"
#include "FiveTuple.h"
#include "FlowMap.h"
#include "Stats.h"
#include "types.h"
#include "TimeOffset.h"

#ifdef DEBUG
#include "common_debug.h"
#endif  // DEBUG

// Global options used to pass info around
extern struct Options {
//    int num_flows_to_plot = 0;
//    int min_pkts_to_plot = 0;
    std::string filter;
    u64 count;
    Duration timeout;
    Duration sampling_rate;
    std::string db;
    std::vector<std::string> traces;
    bool read_from_db = false;
} usr_opt;

/* A map from tuples to flows, each flow is bound to a tuple
 * but a tuple may be bound to many flows, ie it is reused.
 */
extern FlowMap flow_map;

/*
 * An easy way to remember current tuple as I'm not storing with flows..
 */
extern FiveTuple currentTuple;
extern Database db;
extern int last_errno;
extern CacheManager cache_manager;

/*
 *
 * Time of first packet of first trace read (obv traces must be continuous in time)
 * Used to transform packet times from absolute to relative ones (space optimization)
 */
extern timeval capture_start_time;
extern Stats stats;

enum exec_mode {
    NORMAL_EXEC = 1,
    EXIT_ON_FAIL,
};

void exit_with(const std::string &msg);
bool file_exists(const std::string &name);
std::streampos get_file_size(const std::string &file);
std::string make_temp(const std::string &dir, std::ofstream &fout);

void print_info(const std::string &s);
bool prompt_user(const std::string &question);
int execute_cmd(const std::string &cmd, exec_mode m = NORMAL_EXEC);
std::string peak_vmem_used();

#define DB(call) \
    if (call != SQLITE_OK) \
        exit_with(string("DB_ERR: ") + sqlite3_errmsg(conn))

#define UPDATE_STATS(x, num) stats.x += (num)
