#pragma once

#include <map>
#include <vector>

#include "FiveTuple.h"
#include "Flow.h"

class Packet;

class FlowMap {
public:
    void map_to_flow(const Packet &p);
    void add_to_forward_dir(Flow &f, const Packet &p);
    void add_to_reverse_dir(Flow &f, const Packet &p);

    std::map<FiveTuple, std::vector<Flow>>::iterator begin();
    std::map<FiveTuple, std::vector<Flow>>::const_iterator begin() const;
    std::map<FiveTuple, std::vector<Flow>>::iterator end();
    std::map<FiveTuple, std::vector<Flow>>::const_iterator end() const;

private:
    std::map<FiveTuple, std::vector<Flow>> internal_map;

    void try_new_flow(const Packet &pkt);
    void sample_flow_rates(const timeval &t);
};
