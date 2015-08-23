#pragma once

#include <string>

#include "../include/types.h"

class CmdLineParser {
public:
    static void parse_cmdline(int argc, char **argv);
    static void check_args();

private:
    static std::string set_filter();
    static u64 parse_count(std::string &count);
    static u16 parse_timeout(std::string &timeout);
    static void check_traces();
    static void check_that_db_exists();
};
