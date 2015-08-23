#pragma once

#include <vector>

#include "DB_structs.h"
#include "FlowDirection.h"
#include "types.h"

extern "C" {
#include "sqlite/sqlite3.h"
}

class DatabaseReader {
public:
    friend class Database;

private:
    sqlite3_stmt *get_packets_stmt, *get_rates_stmt,
            *get_inter_arrival_times_stmt, *get_RTTs_stmt;

    void prepare_stmts(sqlite3 *conn);
    void finalize_stmts(sqlite3 *conn);

    void get_packets(Dir_id dir_id, std::vector<DB_Packet> &packets, sqlite3 *conn);
    void get_rates(Dir_id dir_id, std::vector<DB_RatePair> &rates, sqlite3 *conn);
    void get_inter_arrival_times(Dir_id dir_id, int num_fetch,
            std::vector<TimeOffset> &times, sqlite3 *conn);
    void get_RTTs(Dir_id dir_id, std::vector<DB_RTT> &RTTs, sqlite3 *conn);
};
