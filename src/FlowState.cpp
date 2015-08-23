#include "../include/FlowState.h"

#include "../include/Flow.h"
#include "../include/Packet.h"

using namespace std;

/*
 * These are all the basic states
 */
State_ptr UNKNOWN_STATE(new Unknown_State());
State_ptr OPENING(new Opening());
State_ptr ESTABLISHED(new Established());
State_ptr DIR_A_CLOSED(new Dir_A_Closed());
State_ptr DIR_B_CLOSED(new Dir_B_Closed());
State_ptr CLOSED(new Closed());

/*
 * This will be default state for new flows. Beware: a new flow may have to handle packet with
 * opposite direction to currentTuple. That's when the tuple already exists and it's the reverse
 * of packet's tuple and the latest flow in tuple's vector says PKT_NOT_FOR_ME.
 */
pkt_decision Unknown_State::handle_packet(Flow &f, const Packet &pkt) const {
    if (pkt.FIN() || pkt.RST())
        return PKT_DROPPED;

    if (pkt.SYN()) {
        if (pkt.ACK()) {
            f.additional_state |= (pkt.has_dirA() ? DIR_A_SYNACK : DIR_B_SYNACK);
            f.set_state(ESTABLISHED);
        } else {
            f.additional_state |= (pkt.has_dirA() ? DIR_A_SYN : DIR_B_SYN);
            f.set_state(OPENING);
        }

        f.add_packet(pkt);
        return PKT_STORED;
    }

    // when seeing data/acks without seeing SYN first
    return handle_regular_pkts(f, pkt);
}

pkt_decision Opening::handle_packet(Flow &f, const Packet &pkt) const {
    if (pkt.FIN())
        return handle_FIN(f, pkt);

    if (pkt.SYN()) {
        if (!pkt.ACK() || (pkt.has_dirA() && f.dirA_SYN()) ||
                (pkt.has_dirB() && f.dirB_SYN())) {
            f.set_state(CLOSED);
            return PKT_NOT_FOR_ME;
        } else {
            f.additional_state |= (pkt.has_dirA() ? DIR_A_SYNACK : DIR_B_SYNACK);
            f.set_state(ESTABLISHED);

            f.add_packet(pkt);
            return PKT_STORED;
        }
    }

    if (pkt.RST())
        return handle_RST(f, pkt);

    return handle_regular_pkts(f, pkt);
}

/*
 * This is after we've seen a handshake or data/acks without any SYNs
 */
pkt_decision Established::handle_packet(Flow &f, const Packet &pkt) const {
    if (pkt.FIN())
        return handle_FIN(f, pkt);

    if (pkt.SYN()) {    // like I said, very simple handling...
        f.set_state(CLOSED);
        return PKT_NOT_FOR_ME;
    }

    if (pkt.RST())
        return handle_RST(f, pkt);

    return handle_regular_pkts(f, pkt);
}

/*
 * We know that forward direction is closed, but not whether same for reverse
 */
pkt_decision Dir_A_Closed::handle_packet(Flow &f, const Packet &pkt) const {
    if (pkt.FIN()) {
        if (pkt.has_dirA())
            return PKT_DROPPED;
        return handle_FIN(f, pkt);
    }

    if (pkt.SYN()) {
        f.set_state(CLOSED);
        return PKT_NOT_FOR_ME;
    }

    if (pkt.RST())
        return handle_RST(f, pkt);

    if (pkt.has_dirA() && pkt.has_data())
        return PKT_NOT_FOR_ME;

    return handle_regular_pkts(f, pkt);
}

/*
 * Same as Dir_A_Closed but for opposite direction
 */
pkt_decision Dir_B_Closed::handle_packet(Flow &f, const Packet &pkt) const {
    if (pkt.FIN()) {
        if (pkt.has_dirB())
            return PKT_DROPPED;
        return handle_FIN(f, pkt);
    }

    if (pkt.SYN()) {
        f.set_state(CLOSED);
        return PKT_NOT_FOR_ME;
    }

    if (pkt.RST())
        return handle_RST(f, pkt);

    if (pkt.has_dirB() && pkt.has_data())
        return PKT_NOT_FOR_ME;

    return handle_regular_pkts(f, pkt);
}

pkt_decision Closed::handle_packet(Flow &f, const Packet &pkt) const {
    (void) pkt, (void) f;   // to avoid error due to unused params
    return PKT_NOT_FOR_ME;
}

/*
 * Basic handling of FIN, assume it's not old duplicate..
 */
pkt_decision Base_State::handle_FIN(Flow &f, const Packet &pkt) const {
    if (!pkt.ACK())
        return PKT_DROPPED;

    if (pkt.has_dirA()) {
        if (f.state == DIR_B_CLOSED)
            f.set_state(CLOSED);
        else
            f.set_state(DIR_A_CLOSED);
    } else {
        if (f.state == DIR_A_CLOSED)
            f.set_state(CLOSED);
        else
            f.set_state(DIR_B_CLOSED);
    }

    f.add_packet(pkt);
    return PKT_STORED;
}

/*
 * As above, naive handling of RST
 */
pkt_decision Base_State::handle_RST(Flow &f, const Packet &pkt) const {
    if (!pkt.ACK())
        return PKT_DROPPED;

    f.set_state(CLOSED);
    f.add_packet(pkt);
    return PKT_STORED;
}

/*
 * Any packet other than FIN, SYN, RST.
 */
pkt_decision Base_State::handle_regular_pkts(Flow &f, const Packet &pkt) const {
    if (!pkt.ACK())
        return PKT_DROPPED;  // all pkts must have ACK bit set

    f.set_state(ESTABLISHED);   // either way

    f.add_packet(pkt);
    return PKT_STORED;
}
