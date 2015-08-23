#include "../include/TimeStamp.h"

TimeStamp::TimeStamp(u32 val) :
        val(val) {
}

/*
 * Relationship straight from rfc1323, hex at the end = 2**31
 */
bool operator>(const TimeStamp &t1, const TimeStamp &t2) {
    return t1.val - t2.val > 0 && t1.val - t2.val < 0x80000000;
}

bool operator==(const TimeStamp &t1, const TimeStamp &t2) {
    return t1.val == t2.val;
}

bool operator<(const TimeStamp &t1, const TimeStamp &t2) {
    return !(t1 > t2) && t1 != t2;
}
