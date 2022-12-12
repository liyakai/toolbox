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



#include "bgp_module.h"

#include "libxorp/xorp.h"
#include "libxorp/debug.h"
#include "libxorp/xlog.h"

#include "peer_list.hh"


BGPPeerList::BGPPeerList()
    : _next_token(0)
{
}

BGPPeerList::~BGPPeerList()
{
    list<BGPPeer *>::iterator i;

    for(i = _peers.begin(); i != _peers.end(); i++) {
	(*i)->event_stop();
	delete (*i);
 	*i = 0;
    }
    _peers.clear();
}

void 
BGPPeerList::all_stop(bool restart) 
{

    list<BGPPeer *>::iterator i;
    for(i = _peers.begin(); i != _peers.end(); i++) {
	(*i)->event_stop(restart);
	if (restart) {
	    if ((*i)->get_current_peer_state() && STATEIDLE == (*i)->state())
		(*i)->event_start();
	}
    }
    /* We now need to drop back to the EventLoop - the peers will only
       move to idle and cleanly tear down their state when the EventLoop
       calls their transmit complete callbacks */
    debug_msg("RETURNING FROM ALL_STOP\n");
}

bool
BGPPeerList::not_all_idle()
{
    list<BGPPeer *>::iterator i;
    for(i = _peers.begin(); i != _peers.end(); i++)
	if(STATEIDLE != (*i)->state())
	    return true;
    
    return false;
}

void
BGPPeerList::add_peer(BGPPeer *p)
{
    list<BGPPeer *>::iterator i;
     
    if (_peers.empty() || p->remote_ip_ge_than(*(_peers.back()))) {
	_peers.push_back(p);
	return;
    }
    
    for(i = _peers.begin(); i != _peers.end(); i++) {
	if((*i)->remote_ip_ge_than(*p)) {
	    _peers.insert(i,p);
	    return;
	}
    }
    XLOG_UNREACHABLE();
}

void
BGPPeerList::remove_peer(BGPPeer *p)
{
    detach_peer(p);
    delete p;
}

void
BGPPeerList::detach_peer(BGPPeer *p)
{
    //Before we remove a peer from the peer list, we need to check
    //whether there are any readers that point to this peer.  As the
    //number of readers is unlikely to be large, and this is not a
    //really frequest operation, we don't mind iterating through the
    //whole map of readers.
    map <uint32_t, list<BGPPeer *>::iterator>::iterator mi;
    for (mi = _readers.begin(); mi != _readers.end(); ) {
	uint32_t token = mi->first;
	list<BGPPeer *>::iterator pli = mi->second;
	++mi;
	if (*pli == p) {
	    pli++;
	    _readers.erase(token);
	    _readers.insert(make_pair(token, pli));
	}
    }

    //Now it's safe to do the deletion.
    list<BGPPeer *>::iterator i;
    for (i = _peers.begin(); i != _peers.end(); i++) {
	if (p == *i) {
	    _peers.erase(i);
	    return;
	}
    }
    XLOG_FATAL("Peer %s not found in peerlist", p->str().c_str());
}

void 
BGPPeerList::dump_list()
{
    list<BGPPeer *>::iterator i;
    for(i = _peers.begin(); i != _peers.end(); i++)
	debug_msg("%s\n", (*i)->str().c_str());
}

bool BGPPeerList::get_peer_list_start(uint32_t& token)
{
    list<BGPPeer *>::iterator i = _peers.begin();
    if (i == _peers.end())
	return false;
    _readers.insert(make_pair(_next_token, i));
    token = _next_token;
    _next_token++;
    return true;
}

bool 
BGPPeerList::get_peer_list_next(const uint32_t& token, 
				string& local_ip, 
				uint32_t& local_port, 
				string& peer_ip, 
				uint32_t& peer_port)
{
    map <uint32_t, list<BGPPeer *>::iterator>::iterator mi;
    mi = _readers.find(token);
    if (mi == _readers.end())
	return false;
    list<BGPPeer *>::iterator i = mi->second;

    if (i == _peers.end()) {
	//this can happen only if the iterator pointed to the last
	//peer, and it was deleted since we were last here.
	local_ip = "0.0.0.0";
	local_port = 0;
	peer_ip = "0.0.0.0";
	peer_port = 0;
    } else {
	BGPPeer *peer = *i;
	local_ip = peer->peerdata()->iptuple().get_local_addr();
	local_port = peer->peerdata()->iptuple().get_local_port();
	peer_ip = peer->peerdata()->iptuple().get_peer_addr();
	peer_port = peer->peerdata()->iptuple().get_peer_port();
	i++;
    }
    if (i == _peers.end()) {
	_readers.erase(mi);
	return false;
    }
    _readers.erase(token);
    _readers.insert(make_pair(token, i));
    return true;
}
