#pragma once

#include "FlowDirection.h"
#include "FlowState.h"
#include "types.h"
#include "TimeOffset.h"

class Packet;

// bits to set in additional_state
enum state_flag_values {
    DIR_A_SYN = 0x01,
    DIR_A_SYNACK = 0x02,
    DIR_B_SYN = 0x04,
    DIR_B_SYNACK = 0x08,
};

/*
 * When a pkt with a new tuple is seen, the tuple is stored in the database and the direction of
 * the packet gives the dirA to the flow. A pkt with some tuple already seen will be added to
 * dirA or dirB depending on how the tuple has been stored already in map.
 */
class Flow {
public:
    State_ptr state = UNKNOWN_STATE;
    u8 additional_state;
    FlowDirection dirA, dirB;   // dirA = direction of tuple in map
    TimeOffset last_packet_time_offset;

    Flow();

    pkt_decision handle_packet(const Packet &pkt);
    u64 num_packets() const;

    /*
     * These should be private for FlowState only but friendship cannot be inherited so...
     */
    void set_state(State_ptr);
    void add_packet(const Packet &pkt);

    bool dirA_SYN() const;
    bool dirA_SYNACK() const;
    bool dirB_SYN() const;
    bool dirB_SYNACK() const;
    bool incompatible_SYN(const Packet &pkt);
};
