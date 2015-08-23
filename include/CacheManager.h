#pragma once

#include <cstddef>
#include <map>
#include <utility>
#include <vector>

#include "FlowDirection.h"
#include "types.h"
#include "TimeOffset.h"

template<typename T>
void shrink_vector(T &v) {
    T().swap(v);
}

class CacheManager {
public:
    enum map_id {
        INTER_ARRIVAL_TIMES_MAP = 1,
    };

    void store(Dir_id id, u32 num_times_needed, const TimeOffset &new_time);

private:
    std::map<Dir_id, std::pair<u8, std::vector<TimeOffset> > > inter_arrival_times;

    size_t real_size() const;
    void flush_to_db(map_id id);
    void update_age_byte(map_id id);
};
