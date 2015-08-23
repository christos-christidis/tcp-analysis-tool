#pragma once

#include <string>

#include "../include/types.h"

struct PCapReader {
    static void read_trace(const std::string &file, u64 &num_pkts_to_read);
    static void print_progress();
};
