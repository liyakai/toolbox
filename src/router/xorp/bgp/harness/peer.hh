// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-

// Copyright (c) 2001-2009 XORP, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, Version 2, June
// 1991 as published by the Free Software Foundation. Redistribution
// and/or modification of this program under the terms of any other
// version of the GNU General Public License is not permitted.
// 
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For more details,
// see the GNU General Public License, Version 2, a copy of which can be
// found in the XORP LICENSE.gpl file.
// 
// XORP Inc, 2953 Bunker Hill Lane, Suite 204, Santa Clara, CA 95054, USA;
// http://xorp.net

// $XORP: xorp/bgp/harness/peer.hh,v 1.26 2008/10/02 21:56:26 bms Exp $

#ifndef __BGP_HARNESS_PEER_HH__
#define __BGP_HARNESS_PEER_HH__

#include "trie.hh"
#include "libxorp/callback.hh"
#include "libxorp/tokenize.hh"
#include "bgp/peer.hh"

class EventLoop;
class TimeVal;

class Peer {
public:

    Peer(EventLoop    *eventloop,
	 XrlStdRouter *xrlrouter,
	 const string& peer_name,
	 const uint32_t genid,
	 const string& target_hostname,
	 const string& target_port);

    Peer(const Peer& rhs);
    Peer& operator=(const Peer& rhs);
    void copy(const Peer& rhs);

    ~Peer();
    
    enum PeerState {YES_ITS_ME, NO_ITS_NOT_ME, PLEASE_DELETE_ME};

    PeerState is_this_you(const string& peer_name) const;

    PeerState is_this_you(const string& peer_name, const uint32_t genid) const;

    void shutdown();

    bool up() const;

    void status(string& status);

    bool pending();

    void listen(const string& line, const vector<string>& words)
	throw(InvalidString);

    void connect(const string& line, const vector<string>& words)
	throw(InvalidString);

    void disconnect(const string& line, const vector<string>& words)
	throw(InvalidString);

    void establish(const string& line, const vector<string>& words)
	throw(InvalidString);

    void send(const string& line, const vector<string>& words)
	throw(InvalidString);

    void send_packet(const string& line, const vector<string>& words)
	throw(InvalidString);

    void send_dump(const string& line, const vector<string>& words)
	throw(InvalidString);

    void trie(const string& line, const vector<string>& words)
	throw(InvalidString);

    void expect(const string& line, const vector<string>& words)
	throw(InvalidString);

    void assertX(const string& line, const vector<string>& words)
	throw(InvalidString);

    void dump(const string& line, const vector<string>& words)
	throw(InvalidString);

    void check_expect(BGPPacket *rec);

    void xrl_callback(const XrlError& error, const char *comment);
    void xrl_callback_connected(const XrlError& error, const char *comment);

    void datain(const bool& status, const TimeVal& tv,
		const vector<uint8_t>&  data);
    void datain_error(const string& reason);
    void datain_closed();

    PathAttribute *path_attribute(const char *)	const throw(InvalidString);
    const BGPPacket *packet(const string& line, const vector<string>& words,
			    int index) const throw(InvalidString);
protected:
    typedef XorpCallback1<void, const XrlError&>::RefPtr SMCB;
    SMCB _smcb;
    void send_message(const uint8_t *buf, const size_t len, SMCB cb);
    void send_dump_callback(const XrlError& error, FILE *fp,
			    const size_t packet_number,
			    const size_t packets_to_send,
			    const char *comment);
    void send_open();
    void send_keepalive();
    
private:
    EventLoop   *_eventloop;
    XrlStdRouter *_xrlrouter;

    /* we need these because the attribute encode methods reference them */
    LocalData *_localdata;
    BGPPeerData *_peerdata;

    string _peername;
    uint32_t _genid;
    string _target_hostname;
    string _target_port;

    bool _up;		// True if this peer has not been shutdown
    TimeVal _shutdown_time;	// Time this peer was shutdown

    uint32_t _busy;	// Count of outstanding transactions.

    bool _connected;	// True if we believe a TCP connection session.
    bool _session;	// We are attempting to form a BGP session.
    bool _passive;	// We are passively trying to create a session.
    bool _established;	// True if we believe a session has been formed.
    bool _keepalive;	// If true echo all received keepalives.

    AsNum _as;		// Our AS number.
    int _holdtime;	// The holdtime sent in the BGP open message.
    IPv4 _id;		// The ID sent in the open message.
    bool _ipv6;		// Should we announce an ipv6 capability in
			// the open message.

    Trie _trie_recv;	// Update messages received.
    Trie _trie_sent;	// Update messages sent.

    // Number of update messages received or sent since last
    // disconnect, used to compute the number of messages in the
    // current session.
    uint32_t _last_recv;
    uint32_t _last_sent;

    typedef XorpCallback3<void, const uint8_t*, size_t, TimeVal>::RefPtr Dumper;
    Dumper _traffic_recv;
    Dumper _traffic_sent;
    
    struct expect {
	expect() : _ok(true), _bad(0) {
	}
	~expect() {
	    list<const BGPPacket *>::iterator i;
	    for(i = _list.begin(); i != _list.end(); i++)
		delete *i;
	    delete _bad;
	}
	
	list<const BGPPacket *> _list; 	// Messages that we are expecting.
	bool _ok;			// True while expected messages arrive.
	const BGPPacket *_bad;	       	// The mismatched packet.
    } _expect;
};

#endif // __BGP_HARNESS_PEER_HH__
