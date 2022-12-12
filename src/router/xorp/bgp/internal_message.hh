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

// $XORP: xorp/bgp/internal_message.hh,v 1.14 2008/11/08 06:14:36 mjh Exp $

#ifndef __BGP_INTERNAL_MESSAGES_HH__
#define __BGP_INTERNAL_MESSAGES_HH__

#include "libxorp/xorp.h"
#include "libxorp/ref_ptr.hh"
#include "subnet_route.hh"
#include "path_attribute.hh"
class PeerHandler;

#define GENID_UNKNOWN 0

/**
 * @short InternalMessage is used to pass route changes between BGP
 * route table classes.
 *
 * XORP BGP is implemented as a pipelined series of route_tables,
 * starting with a RibInTable for each peering, converging on a
 * DecisionTable to decide which competing route is prefered, and then
 * fanning out again to a RibOutTable for each peer.  Routing changes
 * such as add_route, delete_route, and replace_route propagate
 * through this pipeline.  The "payload" of these changes is an
 * InternalMessage, which contains the route itself, the peering from
 * which this route originated, and the generation ID of the RibIn at
 * that peering.
 */
template<class A>
class InternalMessage
{
public:
    InternalMessage(const SubnetRoute<A> *route,
		    const PeerHandler *origin_peer,
		    uint32_t genid);
    InternalMessage(const SubnetRoute<A> *route,
		    FPAListRef pa_list,
		    const PeerHandler *origin_peer,
		    uint32_t genid);
    ~InternalMessage();
    const IPNet<A>& net() const;
    const SubnetRoute<A> *route() const { return _subnet_route; }
    const PeerHandler* origin_peer() const { return _origin_peer; }
    const A& nexthop() const { return _attributes->nexthop(); }
    FPAListRef& attributes() {return _attributes;}
    const FPAListRef& const_attributes() const {return _attributes;}

    bool changed() const { return _changed; }
    void set_changed() { _changed = true; }
    void clear_changed() const { _changed = false; }

    bool copied() const { return _copied; }
    void set_copied() { _copied = true; }
    void clear_copied() const { _copied = false; }

    bool push() const { return _push; }
    void set_push() { _push = true; }
    void clear_push() { _push = false; }

    bool from_previous_peering() const { return _from_previous_peering; }
    void set_from_previous_peering() { _from_previous_peering = true; }

    uint32_t genid() const { return _genid; }

#if 0
    // This is a hack to override const in DecisionTable.
    void force_clear_push() const { _push = false; }
#endif

    // this should only be called if the msg had been copied
    void inactivate() const {
	if (_copied) {
	    _subnet_route->unref();
	    _subnet_route = NULL;
	}
    }

    string str() const;
protected:
private:
    /**
     * the actual route data.
     */
    mutable const SubnetRoute<A> *_subnet_route;

    /**
     * the path attribute list we use for fast accesses.  This is
     * reference counted so we don't need to worry about freeing it
     * when we copy it from message to message.
     */
    FPAListRef _attributes;

    /**
     * we need origin_peer to make sure we don't send a route back to
     * the peer it came from, or send an IBGP route to an IBGP peer.
     */
    const PeerHandler *_origin_peer;

    /**
     * changed indicates that the route data has been modified since
     * the route was last stored (and so needs storing by a
     * CacheTable).
     */
    mutable bool _changed;

    /**
     * copied indicates that the subnet route data has been copied since
     * the route was last stored (and so needs freeing by the final recipient)
     */
    mutable bool _copied;

    /**
     * genid is the generation ID from the RibIn, if known, or zero if
     * it's not known.
     */
    uint32_t _genid;

    /**
     * push indicates that this is the last route in a batch, so the
     * push to peers is implicit.
     */
    mutable bool _push;

    /**
     * from_previous_peering is set on messages where the deleted route
     * originates from a previous peering that has now gone down.
     */
    bool _from_previous_peering;
};

#endif // __BGP_INTERNAL_MESSAGES_HH__
