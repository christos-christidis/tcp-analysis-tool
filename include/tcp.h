#pragma once

#include <netinet/in.h>
#include "types.h"

/*
 * Copyright (c) 1982, 1986, 1993
 *        The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *        @(#)tcp.h        8.1 (Berkeley) 6/10/93
 */

struct tcp_hdr {
    u16 source;
    u16 dest;
    u32 seq;
    u32 ack_seq;
    u8 th_x2 :4;   // (unused)
    u8 doff :4;    // data offset
    u8 flags;
    u16 window;
    u16 check;
    u16 urg_ptr;
};

#define TCP_FIN   0x01
#define TCP_SYN   0x02
#define TCP_RST   0x04
#define TCP_PUSH  0x08
#define TCP_ACK   0x10
#define TCP_URG   0x20
#define TCP_ECE   0x40
#define TCP_CWR   0x80

enum {
    TCP_ESTABLISHED = 1,
    TCP_SYN_SENT,
    TCP_SYN_RECV,
    TCP_FIN_WAIT1,
    TCP_FIN_WAIT2,
    TCP_TIME_WAIT,
    TCP_CLOSE,
    TCP_CLOSE_WAIT,
    TCP_LAST_ACK,
    TCP_LISTEN,
    TCP_CLOSING
};

#define TCP_EOL            0
#define TCP_NOP            1
#define TCP_MSS            2
#define TCP_WSCALE         3
#define TCP_SACK_PERM      4
#define TCP_SACK           5
#define TCP_TIMESTAMP      8
#define TCP_MD5            19

#define TCPI_OPT_TIMESTAMPS     1
#define TCPI_OPT_SACK           2
#define TCPI_OPT_WSCALE         4
#define TCPI_OPT_ECN            8  /* ECN was negotiated at TCP session init */
#define TCPI_OPT_ECN_SEEN       16 /* we received at least one packet with ECT */
#define TCPI_OPT_SYN_DATA       32 /* SYN-ACK acked data in SYN sent or rcvd */

// Values for tcpi_state.
enum tcp_ca_state {
    TCP_CA_Open = 0,
    TCP_CA_Disorder = 1,
    TCP_CA_CWR = 2,
    TCP_CA_Recovery = 3,
    TCP_CA_Loss = 4
};

struct tcp_info {
    u8 tcpi_state;
    u8 tcpi_ca_state;
    u8 tcpi_retransmits;
    u8 tcpi_probes;
    u8 tcpi_backoff;
    u8 tcpi_options;
    u8 tcpi_snd_wscale :4, tcpi_rcv_wscale :4;

    u32 tcpi_snd_mss;
    u32 tcpi_rcv_mss;

    u32 tcpi_unacked;
    u32 tcpi_sacked;
    u32 tcpi_lost;
    u32 tcpi_retrans;
    u32 tcpi_fackets;

    /* Times. */
    u32 tcpi_last_data_sent;
    u32 tcpi_last_ack_sent; /* Not remembered, sorry.  */
    u32 tcpi_last_data_recv;
    u32 tcpi_last_ack_recv;

    /* Metrics. */
    u32 tcpi_pmtu;
    u32 tcpi_rcv_ssthresh;
    u32 tcpi_rtt;
    u32 tcpi_rttvar;
    u32 tcpi_snd_ssthresh;
    u32 tcpi_snd_cwnd;
    u32 tcpi_advmss;
    u32 tcpi_reordering;
};
