#pragma once

#include "types.h"
#include <utility>

// just a wrapper so I can overload > operator etc
struct TimeStamp {
    u32 val;

    TimeStamp(u32 val = 0);
};

using namespace std::rel_ops;
bool operator>(const TimeStamp &t1, const TimeStamp &t2);
bool operator==(const TimeStamp &t1, const TimeStamp &t2);
bool operator<(const TimeStamp &t1, const TimeStamp &t2);
