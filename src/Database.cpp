#include "../include/Database.h"

#include <asm-generic/errno-base.h>
#include <unistd.h>
#include <string>
#include <utility>

extern "C" {
#include "../include/sqlite/sqlite3.h"
}

#include "../include/common.h"

using namespace std;

void Database::open() {
    // I don't see how it could benefit to go multi-threaded but who knows?
    DB(sqlite3_config(SQLITE_CONFIG_SINGLETHREAD));

    DB(sqlite3_open(usr_opt.db.c_str(), &conn));

    DB(sqlite3_extended_result_codes(conn, 1));     // more specific errors

    DB(sqlite3_exec(conn, "PRAGMA foreign_keys = 1;", nullptr, nullptr, nullptr));

    const string s = "PRAGMA page_size = " + to_string(get_page_size());
    DB(sqlite3_exec(conn, s.c_str(), nullptr, nullptr, nullptr));

    DB(sqlite3_exec(conn, "PRAGMA journal_mode = OFF;", nullptr, nullptr, nullptr));

    // db_cache size can also be set...
//    const string cache_size_str = "PRAGMA cache_size = " + to_string(db_cache_size);
//    DB(sqlite3_exec(conn, cache_size_str.c_str(), nullptr, nullptr, nullptr));

    create_tables();
    init_caches();
    writer.prepare_stmts(conn);
    reader.prepare_stmts(conn);
}

/*
 * If I don't finalize stmts, db returns SQLITE_BUSY
 */
void Database::close() {
    writer.finalize_stmts(conn);
    reader.finalize_stmts(conn);

    DB(sqlite3_close(conn));
}

/*
 * In Unix, sqlite sets default page size to 512 sometimes even if it's bigger!
 */
int Database::get_page_size() {
    long ret, page_size = 512;  // default value

    if ((ret = sysconf(_SC_PAGESIZE)) == EINVAL)
        exit_with("ERR: sysconf() failed!");
    else if (ret != -1)     // -1 would be unavailable
        page_size = ret;

    return page_size;
}

/*
 * The data stored are those needed to plot various things like rtt etc. Not everything
 * is stored, but if one wants to, one can modify the tables to include more columns
 */
void Database::create_tables() {
    const string stmt = "DROP TABLE IF EXISTS packet;"
            "CREATE TABLE packet ("
            "dir_id INTEGER,"
            "time_offset INTEGER,"
            "ip_id INTEGER,"
            "seq INTEGER,"
            "ack INTEGER,"
            "tcp_flags INTEGER);"

            "DROP TABLE IF EXISTS rates;"
            "CREATE TABLE rates ("
            "dir_id INTEGER,"
            "time_offset INTEGER,"
            "throughput INTEGER,"
            "goodput INTEGER);"

            "DROP TABLE IF EXISTS inter_arrival_time;"
            "CREATE TABLE inter_arrival_time ("
            "dir_id INTEGER,"
            "time INTEGER);"

            "DROP TABLE IF EXISTS rtt;"
            "CREATE TABLE rtt ("
            "dir_id INTEGER,"
            "time_offset INTEGER,"
            "time INTEGER);";

    // Create all tables with above multi-statement string
    DB(sqlite3_exec(conn, stmt.c_str(), nullptr, nullptr, nullptr));
}

void Database::create_indexes() {
    const string stmt = "CREATE INDEX packet_idx ON packet(dir_id);"
            "CREATE INDEX rates_idx ON rates(dir_id);"
            "CREATE INDEX inter_arrival_time_idx ON inter_arrival_time(dir_id);"
            "CREATE INDEX rtt_idx ON rtt(dir_id);";

    // Create all indexes with above multi-statement string
    DB(sqlite3_exec(conn, stmt.c_str(), nullptr, nullptr, nullptr));

    indexes_created = true;
}

void Database::init_caches() {
    const int page_size = get_page_size();
    packet_cache.reserve(packet_cache_size * page_size / sizeof(DB_Packet));
    rates_cache.reserve(rates_cache_size * page_size / sizeof(DB_RatePair));
    inter_arrival_time_cache.reserve(
            inter_arrival_time_cache_size * page_size / sizeof(pair<Dir_id, TimeOffset> ));
    RTT_cache.reserve(RTT_cache_size * page_size / sizeof(DB_RTT));
}

void Database::flush_caches() {
    writer.write_cache(packet_cache, conn);
    writer.write_cache(rates_cache, conn);
    writer.write_cache(RTT_cache, conn);
    writer.write_cache(inter_arrival_time_cache, conn);
}

/*
 * These methods use the cache and when full instruct writer to flush to db
 */
void Database::store_packet(const DB_Packet &db_pkt) {
    if (is_full(packet_cache))
        writer.write_cache(packet_cache, conn);
    packet_cache.push_back(db_pkt);
}

void Database::store_rates(const DB_RatePair &db_rates) {
    if (is_full(rates_cache))
        writer.write_cache(rates_cache, conn);
    rates_cache.push_back(db_rates);
}

void Database::store_inter_arrival_times(Dir_id id, const vector<TimeOffset> &times) {
    for (auto &time : times) {
        if (is_full(inter_arrival_time_cache))
            writer.write_cache(inter_arrival_time_cache, conn);
        inter_arrival_time_cache.push_back(pair<Dir_id, TimeOffset>(id, time));
    }
}

void Database::store_RTT(const DB_RTT &db_rtt) {
    if (is_full(RTT_cache))
        writer.write_cache(RTT_cache, conn);
    RTT_cache.push_back(db_rtt);
}

void Database::fetch_last_n(Dir_id dir_id, u32 n, vector<TimeOffset> &times) {
    if (!indexes_created)
        create_indexes();
    if (!inter_arrival_time_cache.empty())
        writer.write_cache(inter_arrival_time_cache, conn);
    reader.get_inter_arrival_times(dir_id, n, times, conn);
}

/*
 * These methods flush the cache (if needed) before reading and returning stuff
 */
void Database::get_packets(Dir_id dir_id, vector<DB_Packet> &packets) {
    if (!indexes_created)
        create_indexes();
    if (!packet_cache.empty())
        writer.write_cache(packet_cache, conn);
    reader.get_packets(dir_id, packets, conn);
}

void Database::get_rates(Dir_id dir_id, vector<DB_RatePair> &rates) {
    if (!indexes_created)
        create_indexes();
    if (!rates_cache.empty())
        writer.write_cache(rates_cache, conn);
    reader.get_rates(dir_id, rates, conn);
}

void Database::get_RTTs(Dir_id dir_id, vector<DB_RTT> &RTTs) {
    if (!indexes_created)
        create_indexes();
    if (!RTT_cache.empty())
        writer.write_cache(RTT_cache, conn);
    reader.get_RTTs(dir_id, RTTs, conn);
}

void Database::get_inter_arrival_times(Dir_id dir_id, vector<TimeOffset> &times) {
    if (!indexes_created)
        create_indexes();
    if (!inter_arrival_time_cache.empty())
        writer.write_cache(inter_arrival_time_cache, conn);
    reader.get_inter_arrival_times(dir_id, -1, times, conn);
}
