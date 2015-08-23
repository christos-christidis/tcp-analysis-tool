#include "../include/TimeOffset.h"

#include <iomanip>

using namespace std;

#define USEC_MASK 0x0fffff

u32 TimeOffset::usec() const {
    return val & USEC_MASK;
}

u32 TimeOffset::sec() const {
    return (val & ~USEC_MASK) >> 20;
}

TimeOffset::TimeOffset() :
        val(0) {
}

TimeOffset::TimeOffset(u32 sec, u32 usec) {
    val = sec << 20;
    val |= usec;
}

double TimeOffset::to_sec() const {
    return sec() + usec() / 1000000.;
}

u64 TimeOffset::to_usec() const {
    return sec() * 1000000. + usec();
}

TimeOffset TimeOffset::from_msec(u64 ms) {
    u64 usec = ms * 1000;
    return TimeOffset(usec / 1000000, usec % 1000000);
}

TimeOffset TimeOffset::from_usec(u64 usec) {
    return TimeOffset(usec / 1000000, usec % 1000000);
}

TimeOffset TimeOffset::from_val(u32 val) {
    TimeOffset t;
    t.val = val;
    return t;
}

/*
 * Advance time offset t by duration d. Needed to advance time offset of last sample
 * when sampling for rate, rtt etc
 */
TimeOffset operator+(const TimeOffset &t1, const Duration &d) {
    u32 usec = t1.usec() + d.usec();
    u32 sec = t1.sec() + d.sec();

    if (usec & ~USEC_MASK) {
        usec %= 1000000;
        sec++;
    }

    return TimeOffset(sec, usec);
}

TimeOffset& TimeOffset::operator+=(const Duration &d) {
    *this = *this + d;
    return *this;
}

/*
 * Beware. This should not be called if t1 < t2! It has the meaning of subtracting two offsets
 * with respect to a point in time (eg a timeval) in order to get the duration by which they
 * differ eg, if (t1 - t2 > Duration(2)) do_sth();
 * I don't really like this double of the class but it works
 */
Duration operator-(const TimeOffset &t1, const TimeOffset &t2) {
    u32 usec;
    u32 sec = t1.sec() - t2.sec();

    if (t1.usec() < t2.usec()) {
        usec = t1.usec() + 1000000 - t2.usec();
        sec--;
    } else {
        usec = t1.usec() - t2.usec();
    }

    return Duration(sec, usec);
}

bool operator<(const Duration &d1, const Duration &d2) {
    return d1.sec() < d2.sec() || (d1.sec() == d2.sec() && d1.usec() < d2.usec());
}

bool operator==(const TimeOffset &d1, const TimeOffset &d2) {
    return d1.sec() == d2.sec() && d1.usec() == d2.usec();
}

/*
 * Whole number of times a duration is larger than the other
 */
size_t operator/(const Duration &d1, const Duration &d2) {
    return d1.to_usec() / d2.to_usec();
}

/*
 * Get a duration n times the original
 */
Duration operator*(double n, const Duration &d) {
    u64 usec = (size_t) n * d.to_usec();
    return Duration(usec / 1000000, usec % 1000000);
}

ostream & operator<<(ostream &out, const TimeOffset &t) {
    streamsize orig_width = out.width();
    out << t.sec() << '.' << setw(6) << setfill('0') << t.usec() << setw(orig_width);
    return out;
}
