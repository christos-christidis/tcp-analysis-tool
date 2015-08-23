#pragma once

#include <vector>

#include "DatabaseReader.h"
#include "DatabaseWriter.h"
#include "DB_structs.h"
#include "FlowDirection.h"
#include "sqlite/sqlite3.h"
#include "types.h"
#include "TimeOffset.h"

class Database {
public:
    DatabaseWriter writer;
    DatabaseReader reader;

    void open();
    void close();

    /*
     * The following are forwarded to reader and writer respectively to deal with
     */
    void store_packet(const DB_Packet &db_pkt);
    void store_rates(const DB_RatePair &db_rates);
    void store_inter_arrival_times(Dir_id id, const std::vector<TimeOffset> &times);
    void store_RTT(const DB_RTT &db_rtt);

    void get_packets(Dir_id id, std::vector<DB_Packet> &packets);
    void get_rates(Dir_id id, std::vector<DB_RatePair> &rates);
    void get_inter_arrival_times(Dir_id dir_id, std::vector<TimeOffset> &times);
    void get_RTTs(Dir_id id, std::vector<DB_RTT> &RTTs);
    void fetch_last_n(Dir_id id, u32 num_fetch, std::vector<TimeOffset> &times);

private:
    sqlite3 *conn;
    bool indexes_created;

    static int get_page_size();

    // this is the db disk cache size, it can be set..
//    int db_cache_size;

    /*
     * The purpose of these caches is to group SQL inserts together to improve performance
     * Their size is measured in pages (set via usr_opt???)
     */

    template<typename T>
    bool is_full(T &t) {
        return t.size() == t.capacity();
    }

    std::vector<DB_RatePair> rates_cache;
    std::vector<DB_Packet> packet_cache;
    std::vector<DB_RTT> RTT_cache;
    std::vector<std::pair<Dir_id, TimeOffset>> inter_arrival_time_cache;

    int rates_cache_size = 1;
    int packet_cache_size = 1;
    int RTT_cache_size = 1;
    int inter_arrival_time_cache_size = 1;

    void create_tables();
    void create_indexes();
    void init_caches();
    void flush_caches();
};
