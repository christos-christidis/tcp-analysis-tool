#include "../include/DatabaseReader.h"

#include <string>

#include "../include/common.h"
#include "../include/TimeOffset.h"

using namespace std;

void DatabaseReader::prepare_stmts(sqlite3 *conn) {
    const char *stmt_tail = nullptr;

    // get_packets_stmt
    const char *stmt = "SELECT time_offset, ip_id, seq, ack, tcp_flags "
            "FROM packet "
            "WHERE dir_id = ? "
            "ORDER BY time_offset;";

    DB(sqlite3_prepare_v2(conn, stmt, -1, &get_packets_stmt, &stmt_tail));

    // get_rates_stmt
    stmt = "SELECT time_offset, throughput, goodput "
            "FROM rates "
            "WHERE dir_id = ? "
            "ORDER BY time_offset;";

    DB(sqlite3_prepare_v2(conn, stmt, -1, &get_rates_stmt, &stmt_tail));

    // get_inter_arrival_times_stmt
    stmt = "SELECT time "
            "FROM inter_arrival_time "
            "WHERE dir_id = ? "
            "ORDER BY _rowid_ DESC "
            "LIMIT ?;";

    DB(sqlite3_prepare_v2(conn, stmt, -1, &get_inter_arrival_times_stmt, &stmt_tail));

    // get_RTTs_stmt
    stmt = "SELECT time_offset, time "
            "FROM rtt "
            "WHERE dir_id = ? "
            "ORDER BY time_offset;";

    DB(sqlite3_prepare_v2(conn, stmt, -1, &get_RTTs_stmt, &stmt_tail));
}

void DatabaseReader::finalize_stmts(sqlite3 *conn) {
    DB(sqlite3_finalize(get_packets_stmt));
    DB(sqlite3_finalize(get_rates_stmt));
    DB(sqlite3_finalize(get_inter_arrival_times_stmt));
    DB(sqlite3_finalize(get_RTTs_stmt));
}

void DatabaseReader::get_packets(Dir_id dir_id, vector<DB_Packet> &packets, sqlite3 *conn) {
    DB(sqlite3_exec(conn, "BEGIN TRANSACTION", nullptr, nullptr, nullptr));
    DB(sqlite3_bind_int(get_packets_stmt, 1, dir_id));

    int ret;
    while ((ret = sqlite3_step(get_packets_stmt)) == SQLITE_ROW) {
        TimeOffset time = TimeOffset::from_val(sqlite3_column_int(get_packets_stmt, 0));
        u16 ip_id = sqlite3_column_int(get_packets_stmt, 1);
        u32 seq = sqlite3_column_int(get_packets_stmt, 2);
        u32 ack = sqlite3_column_int(get_packets_stmt, 3);
        u8 flags = sqlite3_column_int(get_packets_stmt, 4);

        packets.push_back(DB_Packet(dir_id, time, ip_id, seq, ack, flags));
    }

    if (ret != SQLITE_DONE)
        exit_with(string("DB_ERR: ") + sqlite3_errstr(ret));

    DB(sqlite3_reset(get_packets_stmt));
    DB(sqlite3_exec(conn, "END TRANSACTION", nullptr, nullptr, nullptr));
}

void DatabaseReader::get_rates(Dir_id dir_id, vector<DB_RatePair> &rates, sqlite3 *conn) {
    DB(sqlite3_exec(conn, "BEGIN TRANSACTION", nullptr, nullptr, nullptr));
    DB(sqlite3_bind_int(get_rates_stmt, 1, dir_id));

    int ret;
    while ((ret = sqlite3_step(get_rates_stmt)) == SQLITE_ROW) {
        TimeOffset time = TimeOffset::from_val(sqlite3_column_int(get_rates_stmt, 0));
        u32 throughput = sqlite3_column_int(get_rates_stmt, 1);
        u32 goodput = sqlite3_column_int(get_rates_stmt, 2);
        rates.push_back(DB_RatePair(dir_id, time, throughput, goodput));
    }

    if (ret != SQLITE_DONE)
        exit_with(string("DB_ERR: ") + sqlite3_errstr(ret));

    DB(sqlite3_reset(get_rates_stmt));
    DB(sqlite3_exec(conn, "END TRANSACTION", nullptr, nullptr, nullptr));
}

void DatabaseReader::get_inter_arrival_times(Dir_id dir_id, int num_fetch, vector<TimeOffset> &times, sqlite3 *conn) {
    DB(sqlite3_exec(conn, "BEGIN TRANSACTION", nullptr, nullptr, nullptr));
    DB(sqlite3_bind_int(get_inter_arrival_times_stmt, 1, dir_id));
    DB(sqlite3_bind_int(get_inter_arrival_times_stmt, 2, num_fetch));

    int ret;
    while ((ret = sqlite3_step(get_inter_arrival_times_stmt)) == SQLITE_ROW) {
        TimeOffset time = TimeOffset::from_val(sqlite3_column_int(get_inter_arrival_times_stmt, 0));
        times.push_back(time);
    }

    if (ret != SQLITE_DONE)
        exit_with(string("DB_ERR: ") + sqlite3_errstr(ret));

    DB(sqlite3_reset(get_inter_arrival_times_stmt));
    DB(sqlite3_exec(conn, "END TRANSACTION", nullptr, nullptr, nullptr));
}

void DatabaseReader::get_RTTs(Dir_id dir_id, std::vector<DB_RTT> &RTTs, sqlite3 *conn) {
    DB(sqlite3_exec(conn, "BEGIN TRANSACTION", nullptr, nullptr, nullptr));
    DB(sqlite3_bind_int(get_RTTs_stmt, 1, dir_id));

    int ret;
    while ((ret = sqlite3_step(get_RTTs_stmt)) == SQLITE_ROW) {
        TimeOffset time = TimeOffset::from_val(sqlite3_column_int(get_RTTs_stmt, 0));
        u32 rtt = sqlite3_column_int(get_RTTs_stmt, 1);
        RTTs.push_back(DB_RTT(dir_id, time, rtt));
    }

    if (ret != SQLITE_DONE)
        exit_with(string("DB_ERR: ") + sqlite3_errstr(ret));

    DB(sqlite3_reset(get_RTTs_stmt));
    DB(sqlite3_exec(conn, "END TRANSACTION", nullptr, nullptr, nullptr));
}
