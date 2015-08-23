#include "../include/FlowMap.h"

#include <utility>

#include "../include/common.h"
#include "../include/Packet.h"

using namespace std;

/*
 * The obvious direction of the tuple is called direction A or dirA. DirB is the direction going
 * the opposite way. So when we ask whether a packet has dirA, we're asking whether its tuple is
 * exactly equal, ie it needn't be reversed, to the tuple under consideration (in the map)
 */

void FlowMap::map_to_flow(const Packet &pkt) {
    const FiveTuple &pkt_tuple = pkt.get_tuple();

    auto iter = internal_map.find(pkt_tuple);

    if (iter != internal_map.end()) {
        currentTuple = iter->first;
        Flow &last_flow = internal_map[currentTuple].back();
        if (last_flow.handle_packet(pkt) == PKT_NOT_FOR_ME)
            try_new_flow(pkt);
    } else {
        iter = internal_map.find(pkt_tuple.reverse());
        if (iter != internal_map.end()) {
            // tuple of packet is included but packet is going "backwards"
            currentTuple = iter->first;
            Flow &last_flow = internal_map[currentTuple].back();
            if (last_flow.handle_packet(pkt) == PKT_NOT_FOR_ME)
                try_new_flow(pkt);
        } else {
            // Tuple of pkt is not included
            currentTuple = pkt.get_tuple();
            try_new_flow(pkt);
        }
    }
}

void FlowMap::try_new_flow(const Packet &pkt) {
    Flow f;
    if (f.handle_packet(pkt) == PKT_STORED)
        internal_map[currentTuple].push_back(f);
}

std::map<FiveTuple, std::vector<Flow>>::iterator FlowMap::begin() {
    return internal_map.begin();
}

std::map<FiveTuple, std::vector<Flow>>::const_iterator FlowMap::begin() const {
    return internal_map.begin();
}

std::map<FiveTuple, std::vector<Flow>>::iterator FlowMap::end() {
    return internal_map.end();
}

std::map<FiveTuple, std::vector<Flow>>::const_iterator FlowMap::end() const {
    return internal_map.end();
}
