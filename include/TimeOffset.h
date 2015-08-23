#pragma once

// NOTE: automatic import organization always throws away <utility> needed for rel_ops!

#include <cstddef>
#include <iostream>
#include <utility>

#include "types.h"

class DatabaseWriter;

// NOTE: automatic import organization always throws away <utility> needed for rel_ops!
// NOTE: automatic import organization always throws away <utility> needed for rel_ops!
/*
 * usec need 20 bits (2**20 = 1,048,576) so in this form we have 12 bits left for sec, ie max
 * sec = 2**12 - 1 = 4095 = 1hr 8min 15s. That easily covers MAWI files (15min) plus the continuous
 * time range of CAIDA (~1hr 2min 40s for 2015), if we store packet time relative to start.
 */
class TimeOffset {
public:
    u32 sec() const;
    u32 usec() const;

    TimeOffset();
    TimeOffset(u32 sec, u32 usec = 0);

    TimeOffset& operator+=(const TimeOffset &t2);

    double to_sec() const;
    u64 to_usec() const;

    static TimeOffset from_msec(u64 ms);
    static TimeOffset from_usec(u64 us);
    static TimeOffset from_val(u32 val);

    friend DatabaseWriter;

private:
    u32 val;
};

/*
 * This class can easily double as a duration
 */
typedef TimeOffset Duration;

/*
 * Note that this type kind of doubles as a duration too, which explains the following operators.
 * I don't really like it but I'm loathe to create a new class just for that
 */
//TimeOffset operator+(const TimeOffset &t1, const TimeOffset &t2);
TimeOffset operator+(const TimeOffset &t1, const Duration &d);
TimeOffset operator-(const TimeOffset &t1, const TimeOffset &t2);

using namespace std::rel_ops;
bool operator<(const TimeOffset &t1, const TimeOffset &t2);
bool operator==(const TimeOffset &t1, const TimeOffset &t2);

size_t operator/(const Duration &d1, const Duration &d2);
Duration operator*(double n, const Duration &d);

std::ostream & operator<<(std::ostream &out, const TimeOffset &t);
