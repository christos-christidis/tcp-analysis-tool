#pragma once

// NOTE: automatic import organization always throws away <utility> needed for rel_ops!

#include "TimeOffset.h"
#include <utility>

struct timeval;

timeval& operator+=(timeval &t, const TimeOffset &to);
timeval operator+(const timeval &t, const TimeOffset &to);
timeval& operator-=(timeval &t, const TimeOffset &to);
timeval operator-(const timeval &t, const TimeOffset &to);

Duration operator-(const timeval &t1, const timeval &t2);

using namespace std::rel_ops;
bool operator<(const timeval &t1, const timeval &t2);
bool operator==(const timeval &t1, const timeval &t2);
