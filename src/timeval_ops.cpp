#include "timeval_ops.h"

#include <sys/time.h>

#include "../include/TimeOffset.h"

/*
 * Easier to implement +=/-= first
 */
timeval& operator+=(timeval &t, const TimeOffset &to) {
    timeval t2, t_to = { to.sec(), to.usec() };   // treat as timepoint...
    timeradd(&t, &t_to, &t2);
    t = t2;
    return t;
}

timeval operator+(const timeval &t, const TimeOffset &to) {
    timeval t2 = t;
    t2 += to;
    return t2;
}

timeval& operator-=(timeval &t, const TimeOffset &to) {
    timeval t2, t_to = { to.sec(), to.usec() };   // treat as timepoint...
    timersub(&t, &t_to, &t2);
    t = t2;
    return t;
}

timeval operator-(const timeval &t, const TimeOffset &to) {
    timeval t2 = t;
    t2 -= to;
    return t2;
}

/*
 * Beware! Subtracting points in time makes sense only if t1 > t2, returning a positive Duration
 * (see "TimeOffset.h" for typedef of Duration)
 */
Duration operator-(const timeval &t1, const timeval &t2) {
    timeval t3;
    timersub(&t1, &t2, &t3);
    return Duration(t3.tv_sec, t3.tv_usec);
}

/*
 * Comparing points in time makes sense
 */
bool operator<(const timeval &t1, const timeval &t2) {
    return t1.tv_sec < t2.tv_sec || (t1.tv_sec == t2.tv_sec && t1.tv_usec < t2.tv_usec);
}

bool operator==(const timeval &t1, const timeval &t2) {
    return t1.tv_sec == t2.tv_sec && t1.tv_usec == t2.tv_usec;
}
