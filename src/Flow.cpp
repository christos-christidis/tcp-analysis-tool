#include "../include/Flow.h"

#include <memory>
#include <utility>

#include "../include/common.h"
#include "../include/Packet.h"
#include "../include/Stats.h"

using namespace std;

Flow::Flow() :
        additional_state() {
}

void Flow::set_state(State_ptr s) {
    state = s;
}

pkt_decision Flow::handle_packet(const Packet &pkt) {
    if (num_packets() > 0 && usr_opt.timeout > Duration(0) &&
            pkt.t_off - last_packet_time_offset > usr_opt.timeout) {
        state = State_ptr(new Closed);
        return PKT_NOT_FOR_ME;
    }

    return state->handle_packet(*this, pkt);
}

void Flow::add_packet(const Packet &pkt) {
    if (num_packets() == 0)
        UPDATE_STATS(num_flows, 1);

    if (pkt.has_dirA())
        dirA.add_packet(pkt);
    else
        dirB.add_packet(pkt);

    last_packet_time_offset = pkt.t_off;
}

bool Flow::dirA_SYN() const {
    return additional_state & DIR_A_SYN;
}
bool Flow::dirA_SYNACK() const {
    return additional_state & DIR_A_SYNACK;
}
bool Flow::dirB_SYN() const {
    return additional_state & DIR_B_SYN;
}
bool Flow::dirB_SYNACK() const {
    return additional_state & DIR_B_SYNACK;
}

/*
 * I lose SYN information on simultaneous opens this way :( Oh well, at least it's manageable
 */
bool Flow::incompatible_SYN(const Packet &pkt) {
    if (!pkt.ACK())
        return true;
    if (pkt.has_dirA() && (dirA_SYN() || dirA_SYNACK()))
        return true;
    if (pkt.has_dirB() && (dirB_SYN() || dirB_SYNACK()))
        return true;
    return false;
}

u64 Flow::num_packets() const {
    return dirA.num_packets + dirB.num_packets;
}
