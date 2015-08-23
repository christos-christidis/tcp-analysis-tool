#pragma once

#include <utility>
#include <vector>

#include "FlowDirection.h"
#include "TimeOffset.h"

extern "C" {
#include "sqlite/sqlite3.h"
}

#include "DB_structs.h"

class DatabaseWriter {
public:
    friend class Database;

private:
    sqlite3_stmt *store_packets_stmt, *store_rates_stmt, *store_inter_arrival_times_stmt, *store_rtt_stmt;

    void prepare_stmts(sqlite3 *conn);
    void finalize_stmts(sqlite3 *conn);

    void write_cache(std::vector<DB_Packet> &packet_cache, sqlite3 *conn);
    void write_cache(std::vector<DB_RatePair> &rates_cache, sqlite3 *conn);
    void write_cache(std::vector<DB_RTT> &rtt_cache, sqlite3 *conn);
    void write_cache(std::vector<std::pair<Dir_id, TimeOffset>> &inter_arrival_time_cache, sqlite3 *conn);
};
