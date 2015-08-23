#include "../include/common.h"

#include <unistd.h>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iterator>

using namespace std;

Options usr_opt;
FlowMap flow_map;
FiveTuple currentTuple;
Database db;
CacheManager cache_manager;
int last_errno;
timeval capture_start_time;
Stats stats;

/*
 * exit with err msg
 */
void exit_with(const string &msg) {
    cerr << msg << endl;
    exit(EXIT_FAILURE);
}

/*
 * Light-weight test for file existence. Not much safety, would have to
 * use stat() for that, burden on the user not to pass directory for instance
 */
bool file_exists(const string &name) {
    bool ret = false;
    errno = 0;
    ifstream fin(name.c_str());
    last_errno = errno;

    if (fin.good()) {
        ret = true;
    }

    fin.close();
    return ret;
}

streampos get_file_size(const string &file) {
    ifstream fin(file, ios::binary | ios::ate);
    return fin.tellg();
}

string make_temp(const string &dir, ofstream &fout) {
    string path = dir + "/XXXXXX";
    // we can't write to s.c_str() which is why we use this
    vector<char> new_path{path.begin(), path.end()};
    new_path.push_back('\0');

    int fd;
    if ((fd = mkstemp((char *) &new_path[0])) != -1) {
        path.assign(new_path.begin(), new_path.end() - 1); // replace with new_path
        fout.open(path.c_str(), ios::trunc);
        close(fd);
    }

    return path;
}

bool prompt_user(const string &question) {
    cout << question << ": ";
    string ans;
    cin >> ans;
    if (ans.find('y') != string::npos || ans.find('Y') != string::npos)
        return true;

    return false;
}

int execute_cmd(const string &cmd, exec_mode mode) {
    static bool system_checked = false;

    if (!system_checked) {
        system_checked = true;
        if (!system(nullptr)) {
            exit_with("Command processor not available. Cannot use 'system()'!");
        }
    }

    int ret = system(cmd.c_str());
    if (ret && mode == EXIT_ON_FAIL) {
        exit_with(string("ERR: system(") + cmd + ") exited with code " + to_string(ret));
    }

    return ret;
}

string peak_vmem_used() {
    string ret = "<ERR: no vmem info found>";

    // most reliable way
    ifstream fin("/proc/self/status");
    if (fin.good()) {
        string s;
        while (getline(fin, s)) {
            if (s.find("VmPeak") != string::npos) {
                s.erase(0, 7);  // length of "VmPeak:"
                size_t idx = s.find_first_not_of(" \t");
                s.erase(s.begin(), s.begin() + idx);
                ret = s;
            }
        }
    }

    return ret;
}
