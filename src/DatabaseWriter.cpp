#include "../include/DatabaseWriter.h"

#include <iterator>
#include <string>

#include "../include/common.h"

using namespace std;

void DatabaseWriter::prepare_stmts(sqlite3 *conn) {
    const char *stmt_tail = nullptr;

    const char *stmt = "INSERT INTO packet("
            "dir_id, time_offset, ip_id, seq, ack, tcp_flags) "
            "VALUES(?, ?, ?, ?, ?, ?);";

    sqlite3_prepare_v2(conn, stmt, -1, &store_packets_stmt, &stmt_tail);

    stmt = "INSERT INTO rates ("
            "dir_id, time_offset, throughput, goodput) "
            "VALUES(?, ?, ?, ?);";

    DB(sqlite3_prepare_v2(conn, stmt, -1, &store_rates_stmt, &stmt_tail));

    stmt = "INSERT INTO inter_arrival_time ("
            "dir_id, time)"
            " VALUES(?, ?);";

    DB(sqlite3_prepare_v2(conn, stmt, -1, &store_inter_arrival_times_stmt, &stmt_tail));

    stmt = "INSERT INTO rtt("
            "dir_id, time_offset, time) "
            "VALUES(?, ?, ?);";

    DB(sqlite3_prepare_v2(conn, stmt, -1, &store_rtt_stmt, &stmt_tail));
}

void DatabaseWriter::finalize_stmts(sqlite3 *conn) {
    DB(sqlite3_finalize(store_packets_stmt));
    DB(sqlite3_finalize(store_rates_stmt));
    DB(sqlite3_finalize(store_inter_arrival_times_stmt));
    DB(sqlite3_finalize(store_rtt_stmt));
}

void DatabaseWriter::write_cache(vector<DB_Packet> &packet_cache, sqlite3 *conn) {
    DB(sqlite3_exec(conn, "BEGIN TRANSACTION", nullptr, nullptr, nullptr));

    for (auto iter = packet_cache.begin(); iter != packet_cache.end(); iter++) {
        DB(sqlite3_bind_int(store_packets_stmt, 1, iter->dir_id));
        DB(sqlite3_bind_int(store_packets_stmt, 2, iter->time.val));
        DB(sqlite3_bind_int(store_packets_stmt, 3, iter->ip_id));
        DB(sqlite3_bind_int(store_packets_stmt, 4, iter->seq));
        DB(sqlite3_bind_int(store_packets_stmt, 5, iter->ack));
        DB(sqlite3_bind_int(store_packets_stmt, 6, iter->tcp_flags));

        if (sqlite3_step(store_packets_stmt) != SQLITE_DONE)
            exit_with(string("DB_ERR: ") + sqlite3_errmsg(conn));

        DB(sqlite3_reset(store_packets_stmt));
    }

    DB(sqlite3_exec(conn, "END TRANSACTION", nullptr, nullptr, nullptr));

    packet_cache.clear();
}

void DatabaseWriter::write_cache(vector<DB_RatePair> &rates_cache, sqlite3 *conn) {
    DB(sqlite3_exec(conn, "BEGIN TRANSACTION", nullptr, nullptr, nullptr));

    for (auto iter = rates_cache.begin(); iter != rates_cache.end(); iter++) {
        DB(sqlite3_bind_int(store_rates_stmt, 1, iter->dir_id));
        DB(sqlite3_bind_int(store_rates_stmt, 2, iter->time.val));
        DB(sqlite3_bind_int(store_rates_stmt, 3, iter->throughput));
        DB(sqlite3_bind_int(store_rates_stmt, 4, iter->goodput));

        if (sqlite3_step(store_rates_stmt) != SQLITE_DONE)
            exit_with(string("DB_ERR: ") + sqlite3_errmsg(conn));

        DB(sqlite3_reset(store_rates_stmt));
    }

    DB(sqlite3_exec(conn, "END TRANSACTION", nullptr, nullptr, nullptr));

    rates_cache.clear();
}

void DatabaseWriter::write_cache(vector<pair<Dir_id, TimeOffset> > &inter_arrival_time_cache,
        sqlite3 *conn) {
    DB(sqlite3_exec(conn, "BEGIN TRANSACTION", nullptr, nullptr, nullptr));

    for (auto &p : inter_arrival_time_cache) {
        DB(sqlite3_bind_int(store_inter_arrival_times_stmt, 1, p.first));
        DB(sqlite3_bind_int(store_inter_arrival_times_stmt, 2, p.second.val));

        if (sqlite3_step(store_inter_arrival_times_stmt) != SQLITE_DONE)
            exit_with(string("DB_ERR: ") + sqlite3_errmsg(conn));

        DB(sqlite3_reset(store_inter_arrival_times_stmt));
    }

    DB(sqlite3_exec(conn, "END TRANSACTION", nullptr, nullptr, nullptr));

    inter_arrival_time_cache.clear();
}

void DatabaseWriter::write_cache(vector<DB_RTT> &RTT_cache, sqlite3 *conn) {
    DB(sqlite3_exec(conn, "BEGIN TRANSACTION", nullptr, nullptr, nullptr));

    for (auto iter = RTT_cache.begin(); iter != RTT_cache.end(); iter++) {
        DB(sqlite3_bind_int(store_rtt_stmt, 1, iter->dir_id));
        DB(sqlite3_bind_int(store_rtt_stmt, 2, iter->time.val));
        DB(sqlite3_bind_int(store_rtt_stmt, 3, iter->rtt));

        if (sqlite3_step(store_rtt_stmt) != SQLITE_DONE)
            exit_with(string("DB_ERR: ") + sqlite3_errmsg(conn));

        DB(sqlite3_reset(store_rtt_stmt));
    }

    DB(sqlite3_exec(conn, "END TRANSACTION", nullptr, nullptr, nullptr));

    RTT_cache.clear();
}
