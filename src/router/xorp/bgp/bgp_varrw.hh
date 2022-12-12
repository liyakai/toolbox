// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
// vim:set sts=4 ts=8:

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

// $XORP: xorp/bgp/bgp_varrw.hh,v 1.26 2008/11/08 06:14:36 mjh Exp $

#ifndef __BGP_BGP_VARRW_HH__
#define __BGP_BGP_VARRW_HH__

#include "policy/backend/single_varrw.hh"
#include "policy/common/element_factory.hh"
#include "internal_message.hh"

template <class A>
class BGPVarRWCallbacks;

/**
 * @short Allows reading an modifying a BGP route.
 *
 * If the route is modified, the user is responsible for retrieving the
 * filtered message and deleting it.
 *
 * Non-copyable due to inheritance from VarRW<A>.
 */
template <class A>
class BGPVarRW :
    public SingleVarRW
{
public:
    enum {
	VAR_NETWORK4 = VAR_PROTOCOL,
        VAR_NEXTHOP4,
        VAR_NETWORK6,
        VAR_NEXTHOP6,
        VAR_ASPATH,
        VAR_ORIGIN,			// 15
        VAR_NEIGHBOR,
        VAR_LOCALPREF,
        VAR_COMMUNITY,
        VAR_MED,
        VAR_MED_REMOVE,			// 20
        VAR_AGGREGATE_PREFIX_LEN,
        VAR_AGGREGATE_BRIEF_MODE,
        VAR_WAS_AGGREGATED,

	VAR_BGPMAX // must be last
    };

    typedef Element* (BGPVarRW::*ReadCallback)();
    typedef void (BGPVarRW::*WriteCallback)(const Element& e);

    /**
     * This varrw allows for routes to remain untouched even though they are
     * filtered. This is useful in order to check if a route will be accepted
     * or rejected, without caring about its modifications.
     *
     * @param name the name of the filter to print in case of tracing.
     */
    BGPVarRW(const string& name);
    virtual ~BGPVarRW();

    void set_peer(const A& peer);
    void set_self(const A& self);

    /**
     * Attach a route to the varrw.
     *
     * @param rtmsg the message to filter and possibly modify.
     * @param no_modify if true, the route will not be modified.
     */
    void attach_route(InternalMessage<A>& rtmsg, bool no_modify);
    void detach_route(InternalMessage<A>& rtmsg);

    /**
     * Caller owns the message [responsible for delete].
     * Calling multiple times will always return the same message, not a copy.
     *
     * @return the modified message. Null if no changes were made.
     */
    InternalMessage<A>* filtered_message();
    
    // SingleVarRW interface
    Element* single_read(const Id& id);

    void single_write(const Id& id, const Element& e);
    void end_write();

    /**
     * If a route is modified, the caller may obtain it via the filtered_message
     * call.
     *
     * @return true if route was modified. False otherwise.
     */
    bool modified();

    /**
     * Output basic BGP specific information.
     *
     * @return BGP trace based on verbosity level returned from trace().
     */
    virtual string more_tracelog();

    /**
     * Reads the neighbor variable.  This is different on input/output branch.
     *
     * @return the neighbor variable.
     */
    virtual Element* read_neighbor();

    /**
     * Callback wrapper used to call the virtual @ref read_neighbor() method.
     *
     * @return the neighbor variable.
     */
    Element* read_neighbor_base_cb()	{ return read_neighbor(); }

    Element* read_policytags();
    Element* read_filter_im();
    Element* read_filter_sm();
    Element* read_filter_ex();

    Element* read_network4();
    Element* read_network6();

    Element* read_nexthop4();
    Element* read_nexthop6();
    Element* read_aspath();
    Element* read_origin();

    Element* read_localpref();
    Element* read_community();
    Element* read_med();
    Element* read_med_remove();

    Element* read_aggregate_prefix_len();
    Element* read_aggregate_brief_mode();
    Element* read_was_aggregated();

    Element* read_tag();

    void write_filter_im(const Element& e);
    void write_filter_sm(const Element& e);
    void write_filter_ex(const Element& e);
    void write_policytags(const Element& e);

    void write_nexthop4(const Element& e);
    void write_nexthop6(const Element& e);
    void write_aspath(const Element& e);
    void write_origin(const Element& e);

    void write_aggregate_prefix_len(const Element& e);
    void write_aggregate_brief_mode(const Element& e);
    void write_was_aggregated(const Element& e);

    void write_localpref(const Element& e);
    void write_community(const Element& e);
    void write_med(const Element& e);
    void write_med_remove(const Element& e);

    void write_tag(const Element& e);

protected:
    ElementFactory		_ef;
    string			_name;

private:
    void cleanup();
    void write_nexthop(const Element& e);

    InternalMessage<A>*	        _rtmsg;
    bool			_got_fmsg;
    PolicyTags*			_ptags;
    bool			_wrote_ptags;
    FPAListRef	                _palist;
    bool			_no_modify;
    bool			_modified;
    RefPf			_pfilter[3];
    bool			_wrote_pfilter[3];
    bool			_route_modify;
    A				_self;
    A				_peer;

    // Aggregation -> we cannot write those directly into the subnet
    // route so must provide local volatile copies to be operated on
    uint32_t			_aggr_prefix_len;
    bool			_aggr_brief_mode;

    // not impl
    BGPVarRW(const BGPVarRW&);
    BGPVarRW& operator=(const BGPVarRW&);

    static BGPVarRWCallbacks<A> _callbacks;
};

template <class A>
class BGPVarRWCallbacks {
public:
    // XXX don't know how to refer to BGPVarRW<A>::ReadCallback in gcc 2.95
    typedef Element* (BGPVarRW<A>::*RCB)();
    typedef void (BGPVarRW<A>::*WCB)(const Element&);

    void init_rw(const VarRW::Id&, RCB, WCB);

    BGPVarRWCallbacks();

    RCB _read_map[BGPVarRW<A>::VAR_BGPMAX];
    WCB _write_map[BGPVarRW<A>::VAR_BGPMAX];
};

#endif // __BGP_BGP_VARRW_HH__
