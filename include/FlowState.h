#pragma once

#include <memory>

class Flow;
class Packet;

enum pkt_decision {
    PKT_STORED = 1,
    PKT_DROPPED,
    PKT_NOT_FOR_ME,
};

/*
 *  NOTE: I make the assumption that pkts that mark start/end of a conversation, ie SYN, FIN and RST
 *  do not suffer any of the usual internet "anomalies" (namely reordering and duplication). If we
 *  allow for those it gets really, really hard to reason about and the states become unmanageable.
 *  ... since I'm not von Neumann I can't do that so I'm leaving it for some other brave soul.
 *
 * This is the basic scheme:
 * SYN ... data ... RST done OR
 * SYN ... data ... FIN ... data ... FIN OR HANG_UP (if no new data for some time) OR
 * SYN ... data ... HANG_UP
 *
 * If data with ACK on but no SYN are seen we assume a connection is ongoing. But like said above
 * when SYN, FIN, RST is seen it's treated like a sync point... we don't wait to see if data has
 * been reordered past them. But what happens when we see a new SYN before FIN? Based on our
 * assumptions SYN can't be old duplicate so if it's not manifestly wrong (ie older timestamp) it
 * starts a new connection.
 */

/*
 * Abstract class for State of a flow wrt BOTH directions
 */
struct Base_State {
    // Children will implement this abstract method
    virtual pkt_decision handle_packet(Flow &f, const Packet &pkt) const = 0;
    virtual ~Base_State() = default;

    // Children states inherit these
    pkt_decision handle_FIN(Flow &f, const Packet &pkt) const;
    pkt_decision handle_RST(Flow &f, const Packet &pkt) const;
    pkt_decision handle_regular_pkts(Flow &f, const Packet &pkt) const;
};

struct Unknown_State: public Base_State {
    pkt_decision handle_packet(Flow &f, const Packet &pkt) const;
};

struct Opening: public Base_State {
    pkt_decision handle_packet(Flow &f, const Packet &pkt) const;
};

struct Established: public Base_State {
    pkt_decision handle_packet(Flow &f, const Packet &pkt) const;
};

struct Dir_A_Closed: public Base_State {
    pkt_decision handle_packet(Flow &f, const Packet &pkt) const;
};

struct Dir_B_Closed: public Base_State {
    pkt_decision handle_packet(Flow &f, const Packet &pkt) const;
};

struct Closed: public Base_State {
    pkt_decision handle_packet(Flow &f, const Packet &pkt) const;
};

/*
 * Since I have to use raw Base_State ptrs, I use shared_ptr to automatically manage them.
 * The globals below will be used to copy from and compare to to implement the state machine
 */
typedef std::shared_ptr<Base_State> State_ptr;

extern State_ptr UNKNOWN_STATE;
extern State_ptr OPENING;
extern State_ptr ESTABLISHED;
extern State_ptr DIR_A_CLOSED;
extern State_ptr DIR_B_CLOSED;
extern State_ptr CLOSED;
