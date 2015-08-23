#include "../include/CacheManager.h"

#include <algorithm>
#include <iterator>

#include "../include/common.h"
#include "../include/Database.h"

#define ONE_GiB (1024 * 1024 * 1024)
#define SIZE_AFTER_FLUSH (0.90 * ONE_GiB)

using namespace std;

/*
 * The n-th inter-arrival time needs n previous times to compute new ones
 */
void CacheManager::store(Dir_id id, u32 num_needed, const TimeOffset &new_time) {
    update_age_byte(INTER_ARRIVAL_TIMES_MAP);

    if (real_size() > ONE_GiB)
        flush_to_db(INTER_ARRIVAL_TIMES_MAP);

    vector<TimeOffset> &times_vector = inter_arrival_times[id].second;
    bool fetched_from_db = false;
    if (num_needed > times_vector.size()) {
        size_t num_fetch = num_needed - times_vector.size();
        db.fetch_last_n(id, num_fetch, times_vector);
        fetched_from_db = true;
    }

    u32 n = num_needed; // I need num_needed later
    for (auto i = times_vector.size() - n; n--; i++)
        times_vector.push_back(new_time + times_vector[i]);

    times_vector.push_back(new_time);

    // must delete those fetched as I don't need them and they already exist in db,
    // otherwise they'll be store AGAIN when vector has to be flushed
    if (fetched_from_db) {
        copy(times_vector.begin() + num_needed, times_vector.end(), times_vector.begin());
        times_vector.resize(num_needed + 1);
    }
}

size_t CacheManager::real_size() const {
    size_t total_size = 0;

    for (auto &p : inter_arrival_times) {
        const vector<TimeOffset> &times_vector = p.second.second;
        total_size += sizeof(times_vector) + times_vector.capacity() * sizeof(TimeOffset);
    }
    total_size += sizeof(inter_arrival_times);

    return total_size;
}

void CacheManager::flush_to_db(map_id id) {
    u8 oldest_age = 255;

    while (real_size() > SIZE_AFTER_FLUSH) {
        switch (id) {
            case INTER_ARRIVAL_TIMES_MAP:
                for (auto &p : inter_arrival_times) {
                    Dir_id dir_id = p.first;
                    u8 age_byte = p.second.first;
                    if (age_byte == oldest_age) {
                        vector<TimeOffset> &times_vector = p.second.second;
                        db.store_inter_arrival_times(dir_id, times_vector);
                        shrink_vector(times_vector);
                    }
                }
                break;
            default:
                exit_with("wrong map id in flush_to_db()");
        }

        oldest_age--;
    }
}

void CacheManager::update_age_byte(map_id id) {
    switch (id) {
        case INTER_ARRIVAL_TIMES_MAP:
            for (auto &p : inter_arrival_times) {
                u8 &age_byte = p.second.first;
                if (age_byte != 255)
                    age_byte++;
            }
            break;
        default:
            exit_with("wrong map id in update_age_byte()");
    }
}
