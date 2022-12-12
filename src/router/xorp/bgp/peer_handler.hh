// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-

// Copyright (c) 2001-2011 XORP, Inc and Others
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

// $XORP: xorp/bgp/peer_handler.hh,v 1.31 2008/11/08 06:14:37 mjh Exp $

#ifndef __BGP_PEER_HANDLER_HH__
#define __BGP_PEER_HANDLER_HH__

class BGPMain;
class BGPPeer;
class BGPPlumbing;

#include "libxorp/debug.h"
#include "internal_message.hh"
#include "packet.hh"
#include "peer.hh"
#include "parameter.hh"

/**
 * PeerHandler's job is primarily format conversion. But it also
 * servers as a handle to tie together the input and output sides of a
 * RIB.
 */
class PeerHandler {
public:
    PeerHandler(const string &peername, BGPPeer *peer,
		BGPPlumbing *plumbing_unicast,
		BGPPlumbing *plumbing_multicast);
    virtual ~PeerHandler();

    void stop();
    void peering_went_down();
    void peering_came_up();
    bool peering_is_up() const { return _peering_is_up; }

    /**
     * process_update_packet is called when an update packet has been
     * received by this peer
     */
    int process_update_packet(UpdatePacket *p);
    
    /**
     * Given an update packet find all the NLRIs with <AFI,SAFI>
     * specified and inject one by one into the plumbing.
     *
     * @param p - packet to tease apart
     * @param safi - Subsequent address family identifier
     *
     * @return true if an <AFI,SAFI> was found.
     */
    template <typename A> 
    bool add(const UpdatePacket *p,
	     ref_ptr<FastPathAttributeList<IPv4> >& original_pa_list,
	     ref_ptr<FastPathAttributeList<A> >& pa_list,
	     Safi safi);
    /**
     * Given an update packet find all the WITHDRAWs with <AFI,SAFI>
     * specified and inject one by on into the plumbing.
     *
     * @param p - packet to tease apart
     * @param safi - Subsequent address family identifier
     *
     * @return true if an <AFI,SAFI> was found.
     */
    template <typename A> 
    bool withdraw(const UpdatePacket *p, 
		  ref_ptr<FastPathAttributeList<IPv4> >& original_pa_list,
		  Safi safi);

    template <typename A> bool multiprotocol(Safi safi, 
					     BGPPeerData::Direction d) const;


    /**
     * add_route and delete_route are called by the plumbing to
     * propagate a route *to* the peer.
     */
    virtual int start_packet();

    virtual int add_route(const SubnetRoute<IPv4> &rt, 
			  FPAList4Ref& pa_list,
			  bool ibgp, Safi safi);
    virtual int replace_route(const SubnetRoute<IPv4> &old_rt, bool old_ibgp,
			      const SubnetRoute<IPv4> &new_rt, bool new_ibgp,
			      FPAList4Ref& pa_list,
			      Safi safi);
    virtual int delete_route(const SubnetRoute<IPv4> &rt,  
			     FPAList4Ref& pa_list,
			     bool new_ibgp,
			     Safi safi);
    virtual PeerOutputState push_packet();
    virtual void output_no_longer_busy();

    /**
     * The AS number of this router.
     */
    AsNum my_AS_number() const 		{
	return _peer->peerdata()->my_AS_number();
    }

    /**
     * The AS number of the peer router.
     */
    AsNum AS_number() const		{ return _peer->peerdata()->as(); }


    /**
     * Do we use 4-byte AS numbers with this peer?
     */
    bool use_4byte_asnums() const {
	return _peer->peerdata()->use_4byte_asnums();
    }

    virtual PeerType get_peer_type() const { 
	return _peer->peerdata()->get_peer_type(); 
    }

    const string& peername() const	{ return _peername; }
    bool ibgp() const			{
	if (0 == _peer)	{
	    XLOG_ASSERT(originate_route_handler());
	    return false;
	}
	return _peer->peerdata()->ibgp();
    }

    /**
     * @return true if this is the originate route handler.
     */
    virtual bool originate_route_handler() const {return false;}

    /**
     * @return an ID that is unique per peer for use in decision.
     */
    virtual uint32_t get_unique_id() const { return _peer->get_unique_id(); }

    /**
     * @return the neighbours BGP ID as an integer for use by decision.
     */
    virtual const IPv4& id() const { return _peer->peerdata()->id(); }

    /**
     * @return the neighbours IP address an as integer for use by
     * decision.
     */
    uint32_t neighbour_address() const	{
	//return ntohl(_peer->peerdata()->iptuple().get_peer_addr().s_addr);
	return id().addr();
    }

    const IPv4& my_v4_nexthop() const	{
	return _peer->peerdata()->get_v4_local_addr();
    }

    /**
     * Get the local address as a string in numeric form.
     */
    string get_local_addr() const {
	return _peer->peerdata()->iptuple().get_local_addr();
    }

    /**
     * Get the peer address as a string in numeric form.
     */
    string get_peer_addr() const {
	return _peer->peerdata()->iptuple().get_peer_addr();
    }

    /**
     * @param addr fill in the address if this is IPv4.
     * @return true if the peer address is IPv4.
     */
    bool get_peer_addr(IPv4& addr) const {
	return _peer->peerdata()->iptuple().get_peer_addr(addr);
    }

    /**
     * @return the number of prefixes in the RIB-IN.
     */
    uint32_t get_prefix_count() const;

    virtual EventLoop& eventloop() const;


#ifdef HAVE_IPV6

    virtual int add_route(const SubnetRoute<IPv6> &rt, 
			  FPAList6Ref& pa_list,
			  bool ibgp, Safi safi);
    virtual int replace_route(const SubnetRoute<IPv6> &old_rt, bool old_ibgp,
			      const SubnetRoute<IPv6> &new_rt, bool new_ibgp,
			      FPAList6Ref& pa_list,
			      Safi safi);
    virtual int delete_route(const SubnetRoute<IPv6> &rt,  
			     FPAList6Ref& pa_list,
			     bool new_ibgp,
			     Safi safi);
    const IPv6& my_v6_nexthop() const	{
	return _peer->peerdata()->get_v6_local_addr();
    }

    /**
     * @param addr fill in the address if this is IPv6.
     * @return true if the peer address is IPv6.
     */
    bool get_peer_addr(IPv6& addr) const {
	return _peer->peerdata()->iptuple().get_peer_addr(addr);
    }


#endif //ipv6

protected:
    BGPPlumbing *_plumbing_unicast;
    BGPPlumbing *_plumbing_multicast;
private:
    string _peername;
    BGPPeer *_peer;
    bool _peering_is_up; /*whether we still think it's up (it may be
                           down, but the FSM hasn't told us yet) */
    UpdatePacket *_packet; /* this is a packet we construct to send */

    /*stats*/
    uint32_t _nlri_total;
    uint32_t _packets;
};

#endif // __BGP_PEER_HANDLER_HH__
