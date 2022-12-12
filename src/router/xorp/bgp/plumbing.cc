// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
// vim:set sts=4 ts=8:

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



// #define DEBUG_LOGGING
// #define DEBUG_PRINT_FUNCTION_NAME
#define CHECK_TIME

#include "bgp_module.h"

#include "libxorp/xorp.h"
#include "libxorp/debug.h"
#include "libxorp/xlog.h"
#include "libxorp/timer.hh"
#include "libxorp/timespent.hh"

#include "route_table_reader.hh"
#include "plumbing.hh"
#include "bgp.hh"
#include "profile_vars.hh"
#include "dump_iterators.hh"


BGPPlumbing::BGPPlumbing(const Safi safi,
			 RibIpcHandler* ribhandler,
			 AggregationHandler* aggrhandler,
			 NextHopResolver<IPv4>& next_hop_resolver_ipv4,
#ifdef HAVE_IPV6
			 NextHopResolver<IPv6>& next_hop_resolver_ipv6,
#endif
			 PolicyFilters& pfs,
			 BGPMain& bgp)
    : _bgp(bgp),
      _rib_handler(ribhandler),
      _aggr_handler(aggrhandler),
      _next_hop_resolver_ipv4(next_hop_resolver_ipv4),
      _safi(safi),
      _policy_filters(pfs),
      _plumbing_ipv4("[IPv4:" + string(pretty_string_safi(safi)) + "]", *this,
		     _next_hop_resolver_ipv4)
#ifdef HAVE_IPV6
    , _next_hop_resolver_ipv6(next_hop_resolver_ipv6)
    , _plumbing_ipv6("[IPv6:" + string(pretty_string_safi(safi)) + "]", *this, 
		     _next_hop_resolver_ipv6)
#endif
{
}

int
BGPPlumbing::add_peering(PeerHandler* peer_handler) 
{
    int result = 0;
    result |= plumbing_ipv4().add_peering(peer_handler);
#ifdef HAVE_IPV6
    result |= plumbing_ipv6().add_peering(peer_handler);
#endif
    return result;
}

int
BGPPlumbing::stop_peering(PeerHandler* peer_handler) 
{
    debug_msg("BGPPlumbing::stop_peering\n");
    int result = 0;
    result |= plumbing_ipv4().stop_peering(peer_handler);
#ifdef HAVE_IPV6
    result |= plumbing_ipv6().stop_peering(peer_handler);
#endif
    return result;
}

int
BGPPlumbing::peering_went_down(PeerHandler* peer_handler) 
{
    debug_msg("BGPPlumbing::peering_went_down\n");
    int result = 0;
    TIMESPENT();
    result |= plumbing_ipv4().peering_went_down(peer_handler);
    TIMESPENT_CHECK();
#ifdef HAVE_IPV6
    result |= plumbing_ipv6().peering_went_down(peer_handler);
    TIMESPENT_CHECK();
#endif
    return result;
}

int 
BGPPlumbing::peering_came_up(PeerHandler* peer_handler) 
{
    debug_msg("BGPPlumbing::peering_came_up\n");
    int result = 0;
    result |= plumbing_ipv4().peering_came_up(peer_handler);
#ifdef HAVE_IPV6
    result |= plumbing_ipv6().peering_came_up(peer_handler);
#endif
    return result;
}

int
BGPPlumbing::delete_peering(PeerHandler* peer_handler) 
{
    debug_msg("BGPPlumbing::delete_peering\n");
    int result = 0;
    result |= plumbing_ipv4().delete_peering(peer_handler);
#ifdef HAVE_IPV6
    result |= plumbing_ipv6().delete_peering(peer_handler);
#endif
    return result;
}

void
BGPPlumbing::flush(PeerHandler* peer_handler) 
{
    debug_msg("BGPPlumbing::flush\n");
    plumbing_ipv4().flush(peer_handler);
#ifdef HAVE_IPV6
    plumbing_ipv6().flush(peer_handler);
#endif
}

int 
BGPPlumbing::add_route(const IPv4Net& net,
		       FPAList4Ref& pa_list,
		       const PolicyTags& policy_tags,
		       PeerHandler* peer_handler) 
{
    debug_msg("BGPPlumbing::add_route IPv4\n");
    PROFILE(if (main().profile().enabled(profile_route_ribin))
		main().profile().log(profile_route_ribin,
				     c_format("add %s", net.str().c_str())));

    XLOG_ASSERT(!pa_list->is_locked());
    return plumbing_ipv4().add_route(net, pa_list, policy_tags, peer_handler);
}

int 
BGPPlumbing::delete_route(InternalMessage<IPv4> &rtmsg, 
			  PeerHandler* peer_handler) 
{
    PROFILE(if (main().profile().enabled(profile_route_ribin))
		main().profile().log(profile_route_ribin,
				     c_format("delete %s", rtmsg.net().str().c_str())));

    return plumbing_ipv4().delete_route(rtmsg, peer_handler);
}

int 
BGPPlumbing::delete_route(const IPNet<IPv4>& net,
			  PeerHandler* peer_handler) 
{
    PROFILE(if (main().profile().enabled(profile_route_ribin))
		main().profile().log(profile_route_ribin,
				     c_format("delete %s", net.str().c_str())));

    return plumbing_ipv4().delete_route(net, peer_handler);
}

const SubnetRoute<IPv4>* 
BGPPlumbing::lookup_route(const IPNet<IPv4> &net) const 
{
    return const_cast<BGPPlumbing *>(this)->
	plumbing_ipv4().lookup_route(net);
}

template<>
void
BGPPlumbing::push<IPv4>(PeerHandler* peer_handler) 
{
    debug_msg("BGPPlumbing::push\n");
    plumbing_ipv4().push(peer_handler);
}

void
BGPPlumbing::output_no_longer_busy(PeerHandler *peer_handler) 
{
    plumbing_ipv4().output_no_longer_busy(peer_handler);
#ifdef HAVE_IPV6
    plumbing_ipv6().output_no_longer_busy(peer_handler);
#endif
}

uint32_t
BGPPlumbing::get_prefix_count(const PeerHandler *peer_handler)
{
    return
	plumbing_ipv4().
	get_prefix_count(const_cast<PeerHandler *>(peer_handler))
#ifdef HAVE_IPV6
	+ plumbing_ipv6().
	get_prefix_count(const_cast<PeerHandler *>(peer_handler))
#endif
	;
}

template<>
uint32_t 
BGPPlumbing::create_route_table_reader<IPv4>(const IPNet<IPv4>& prefix)
{
    return plumbing_ipv4().create_route_table_reader(prefix);
}

bool 
BGPPlumbing::read_next_route(uint32_t token, 
			     const SubnetRoute<IPv4>*& route, 
			     IPv4& peer_id)
{
    return plumbing_ipv4().read_next_route(token, route, peer_id);
}

bool
BGPPlumbing::status(string& reason) const
{
    if (const_cast<BGPPlumbing *>(this)->
	plumbing_ipv4().status(reason) == false) {
	return false;
    }
#ifdef HAVE_IPV6
    if (const_cast<BGPPlumbing *>(this)->
	plumbing_ipv6().status(reason) == false) {
	return false;
    }
#endif
    return true;
}

void
BGPPlumbing::push_routes() {
    plumbing_ipv4().push_routes();
#ifdef HAVE_IPV6
    plumbing_ipv6().push_routes();
#endif
}

/***********************************************************************/

template <class A>
BGPPlumbingAF<A>::BGPPlumbingAF(const string& ribname,
				BGPPlumbing& master,
				NextHopResolver<A>& next_hop_resolver)
    : _ribname(ribname), _master(master), _next_hop_resolver(next_hop_resolver)
{
    debug_msg("BGPPlumbingAF constructor called for RIB %s\n", 
	      ribname.c_str());
    _awaits_push = false;

    //We want to seed the route table reader token so that if BGP
    //restarts, an old token is unlikely to be accepted.
    _max_reader_token = getpid() << 16;

    /*
     * Initial plumbing is:
     *
     *     DecisionTable -> FanoutTable -> FilterTable -> ..
     *        ..-> RibOutTable -> RibIpcHandler
     *
     * This is the path taken by routes that we propagate to the
     * RIB process for local use.
     *
     * All the plumbing regarding BGP Peers gets added later.
     *
     * The RibIpcHandler resides in the master plumbing class.  The
     * rest is AF specific, so resides here.
     */

    _decision_table = 
	new DecisionTable<A>(ribname + "DecisionTable",
			     _master.safi(),
			     _next_hop_resolver);
    _next_hop_resolver.add_decision(_decision_table);

    _policy_sourcematch_table =
	new PolicyTableSourceMatch<A>(ribname + "PolicyExportSourceMatchTable",
				      _master.safi(),
				      _decision_table,
				      _master.policy_filters(),
				      _master.main().eventloop());
    _decision_table->set_next_table(_policy_sourcematch_table);

    _aggregation_table =
	new AggregationTable<A>(ribname + "AggregationTable",
				_master,
				_policy_sourcematch_table);
    _policy_sourcematch_table->set_next_table(_aggregation_table);

    _fanout_table =
	new FanoutTable<A>(ribname + "FanoutTable",
			   _master.safi(),
			   _aggregation_table,
			   _master.aggr_handler(),
			   _aggregation_table);
    _aggregation_table->set_next_table(_fanout_table);


    /*
     * Plumb the input branch
     */
    
    _ipc_rib_in_table =
	new RibInTable<A>(_ribname + "IpcRibInTable",
			  _master.safi(),
			  _master.rib_handler());
    _in_map[_master.rib_handler()] = _ipc_rib_in_table;

    FilterTable<A>* filter_in =
	new FilterTable<A>(_ribname + "IpcChannelInputFilter",
			   _master.safi(),
			   _ipc_rib_in_table,
			   _next_hop_resolver);
    filter_in->do_versioning();
    _ipc_rib_in_table->set_next_table(filter_in);
    
    PolicyTableImport<A>* policy_filter_in =
	new PolicyTableImport<A>(_ribname + "IpcChannelInputPolicyFilter",
				 _master.safi(),
				 filter_in,
				 _master.policy_filters(),
				 A(), A());
    filter_in->set_next_table(policy_filter_in);

    // No policy import filters on routes coming from the RIB.
    // XXX We still need the table to do the necessary policy route dump voodoo
    // though.
    policy_filter_in->enable_filtering(false);

    CacheTable<A>* cache_in = 
	new CacheTable<A>(_ribname + "IpcChannelInputCache",
			  _master.safi(),
			  policy_filter_in,
			  _master.rib_handler());
    policy_filter_in->set_next_table(cache_in);

    NhLookupTable<A> *nexthop_in =
	new NhLookupTable<A>(_ribname + "IpcChannelNhLookup",
			     _master.safi(),
			     &_next_hop_resolver,
			     cache_in);
    cache_in->set_next_table(nexthop_in);

    nexthop_in->set_next_table(_decision_table);
    _decision_table->add_parent(nexthop_in, _master.rib_handler(),
				_ipc_rib_in_table->genid());

    _tables.insert(filter_in);
    _tables.insert(policy_filter_in);
    _tables.insert(cache_in);
    _tables.insert(nexthop_in);

    /*
     * Plumb the output branch
     */

    FilterTable<A> *filter_out =
	new FilterTable<A>(ribname + "IpcChannelOutputFilter",
			   _master.safi(),
			   _fanout_table,
			   _next_hop_resolver);
    _tables.insert(filter_out);

    XLOG_ASSERT(_master.rib_handler());

    // No policy export filters on routes destined to the RIB

    // Drop in an aggregation filter - beheave like an IBGP peering
    filter_out->add_aggregation_filter(true);

#if 0
    //
    // outgoing caches are obsolete
    //
    CacheTable<A> *cache_out =
	new CacheTable<A>(ribname + "IpcChannelOutputCache",
			  _master.safi(),
			  filter_out,
			  _master.rib_handler());
    filter_out->set_next_table(cache_out);
    _tables.insert(cache_out);
#endif

    _ipc_rib_out_table =
	new RibOutTable<A>(ribname + "IpcRibOutTable",
			   _master.safi(),
			   filter_out,
			   _master.rib_handler());
    _out_map[_master.rib_handler()] = _ipc_rib_out_table;
    filter_out->set_next_table(_ipc_rib_out_table);

    _fanout_table->add_next_table(filter_out, _master.rib_handler(),
				  _ipc_rib_in_table->genid());
}

template <class A>
BGPPlumbingAF<A>::~BGPPlumbingAF() 
{
    typename set <BGPRouteTable<A>*>::iterator i;
    for(i = _tables.begin(); i != _tables.end(); i++) {
	delete (*i);
    }
    delete _decision_table;
    delete _policy_sourcematch_table;
    delete _fanout_table;
    delete _ipc_rib_in_table;
    delete _ipc_rib_out_table;
}

template <class A>
void
BGPPlumbingAF<A>::configure_inbound_filter(PeerHandler* peer_handler,
					   FilterTable<A>* filter_in)
{
    PeerType peer_type = peer_handler->get_peer_type();
    AsNum my_AS_number = peer_handler->my_AS_number();

    /* 1. configure the loop filters */
    filter_in->add_simple_AS_filter(my_AS_number);

    /* 2. Configure local preference filter.
       Add LOCAL_PREF on receipt from EBGP peer, or when originating.  */
    if (   peer_type == PEER_TYPE_EBGP 
	|| peer_type == PEER_TYPE_EBGP_CONFED 
	|| peer_type == PEER_TYPE_INTERNAL ) {
	filter_in->add_localpref_insertion_filter(
	  LocalPrefAttribute::default_value() );
    }

    /* 3. If this router is a route reflector configure the in bound
       filters */
    LocalData *local_data = _master.main().get_local_data();
    if (local_data->get_route_reflector()) {
	if (peer_type == PEER_TYPE_IBGP ||
	    peer_type == PEER_TYPE_IBGP_CLIENT) {
	    IPv4 bgp_id = local_data->get_id();
	    IPv4 cluster_id = local_data->get_cluster_id();
	    filter_in->add_route_reflector_input_filter(bgp_id,	cluster_id);
	}
    }
}

template <class A>
void
BGPPlumbingAF<A>::configure_outbound_filter(PeerHandler* peer_handler,
					    FilterTable<A>* filter_out)
{
    const AsNum& his_AS_number = peer_handler->AS_number();
    const AsNum& my_AS_number = peer_handler->my_AS_number();
    PeerType peer_type = peer_handler->get_peer_type();
    A my_nexthop(get_local_nexthop(peer_handler));

    /* 1. configure aggregation filters */
    filter_out->add_aggregation_filter(peer_handler->ibgp());

    /* 1.1 configure the loop filters */
    filter_out->add_simple_AS_filter(his_AS_number);

    /* 2. configure as_prepend filters for EBGP peers*/
    if (peer_type == PEER_TYPE_EBGP) {
	filter_out->add_AS_prepend_filter(my_AS_number, false);
    }
    if (peer_type == PEER_TYPE_EBGP_CONFED) {
	filter_out->add_AS_prepend_filter(my_AS_number, true);
    }
    /* 2.1 For routes that we originate add our AS if its not already
       (EBGP peers) present. */
    filter_out->add_originate_route_filter(my_AS_number, peer_type);

    /* 3. Configure MED filter.
	  Remove old MED and add new one on transmission to EBGP peers. */
    /* Note: this MUST come before the nexthop rewriter */
    if (peer_type != PEER_TYPE_IBGP 
	&& peer_type != PEER_TYPE_IBGP_CLIENT) {
	/* By default, we remove the old MED unless we're sending to
	   an IBGP neighbour */
	/* RFC 3065 says we can legally send MED to EBGP_CONFED
	   neighbours, but it's not clear that this is a good default.
	   We choose not to do so - this is safe . */
	filter_out->add_med_removal_filter();
    }
    if (peer_type == PEER_TYPE_EBGP) {
	/* we only add MED when sending to an EBGP peer (not an EBGP
	   confed peer - that doesn't seem to be legal). */
	filter_out->add_med_insertion_filter();
    }

    /* 4. configure next_hop rewriter for EBGP peers*/
    IPNet<A> subnet;
    A peer;
    bool direct = directly_connected(peer_handler, subnet, peer);
    if (peer_type == PEER_TYPE_EBGP) {
	filter_out->add_nexthop_rewrite_filter(my_nexthop, direct, subnet);
    }

    /* 4.a If the route was originated by this router and the nexthop
       and the peer address are equal rewrite the nexthop. */
    filter_out->add_nexthop_peer_check_filter(my_nexthop, peer);

    /* 5. Configure local preference filter.
	  Remove LOCAL_PREF on transmission to EBGP peers. */
    // Note it is legal to send local pref to a confederation peer.
    if (peer_type == PEER_TYPE_EBGP) {
	filter_out->add_localpref_removal_filter();
    }

    /* 6. configure loop filter for IBGP peers, unless this router is
       a route reflector */
    LocalData *local_data = _master.main().get_local_data();
    if (local_data->get_route_reflector()) {
	if (peer_type == PEER_TYPE_IBGP ||
	    peer_type == PEER_TYPE_IBGP_CLIENT) {
	    bool client = peer_type == PEER_TYPE_IBGP_CLIENT;
	    IPv4 bgp_id = local_data->get_id();
	    IPv4 cluster_id = local_data->get_cluster_id();
	    filter_out->add_route_reflector_ibgp_loop_filter(client,
							     bgp_id,
							     cluster_id);
	}
    } else {
	if (peer_type == PEER_TYPE_IBGP) {
	    filter_out->add_ibgp_loop_filter();
	}
    }

    // 6.1. Remove route reflector ORIGINATOR_ID and CLUSTER_LIST
    // attributes that we may have learned from a peer when sending
    // to an EBGP peer. Route reflector attributes are IBGP specific.
    if (peer_type == PEER_TYPE_EBGP ||
	peer_type == PEER_TYPE_EBGP_CONFED) {
	filter_out->add_route_reflector_purge_filter();
    }

    /* 7. configure filter for well-known communities */
    filter_out->add_known_community_filter(peer_type);

    /* 8. Process unknown attributes */
    filter_out->add_unknown_filter();
}

/**
 * @short re-instantiate all the static filters.
 *
 * Re-instantiate all the static filters in case the config has
 * changed, and the filter parameters are now different.  The
 * FilterTable will handle consistency issues this might otherwise
 * raise.
 */

template <class A>
void
BGPPlumbingAF<A>::reconfigure_filters(PeerHandler* peer_handler) 
{

    /* We need to find the outbound filter table */
    BGPRouteTable<A> *rt; 
    typename map <PeerHandler*, RibOutTable<A>*>::iterator iter;
    iter = _out_map.find(peer_handler);
    if (iter == _out_map.end()) 
	XLOG_FATAL("BGPPlumbingAF<IPv%u,%s>::reconfigure_filters: peer %p not found",
		   XORP_UINT_CAST(A::ip_version()),
		   pretty_string_safi(_master.safi()),
		   peer_handler);
    rt = iter->second;
    while (1) {
	XLOG_ASSERT(rt != _fanout_table);
	if (rt->type() == FILTER_TABLE) {
	    FilterTable<A> *filter_table = (FilterTable<A> *)rt;

	    /* tell the filter table to checkpoint state, and be ready
	       for reconfiguration */
	    filter_table->reconfigure_filter();
	    
	    /* add new filters */
	    configure_outbound_filter(peer_handler, filter_table);

	    break;
	}
	rt = rt->parent();
    }    


    typename map <PeerHandler*, RibInTable<A>* >::iterator iter2;
    iter2 = _in_map.find(peer_handler);
    if (iter2 == _in_map.end())
	XLOG_FATAL("BGPPlumbingAF<IPv%u,%s>::reconfigure_filters: peer %p not found",
		   XORP_UINT_CAST(A::ip_version()),
		   pretty_string_safi(_master.safi()),
		   peer_handler);
    rt = iter2->second;
    while (1) {
	XLOG_ASSERT(rt != _decision_table);
	if (rt->type() == FILTER_TABLE) {
	    FilterTable<A> *filter_table = (FilterTable<A> *)rt;

	    /* tell the filter table to checkpoint state, and be ready
	       for reconfiguration */
	    filter_table->reconfigure_filter();
	    
	    /* add new filters */
	    configure_inbound_filter(peer_handler, filter_table);

	    break;
	}
	rt = rt->next_table();
    }    
}

template <class A>
int 
BGPPlumbingAF<A>::add_peering(PeerHandler* peer_handler) 
{
    /*
     * A new peer just came up.  We need to create all the RouteTables
     * to handle taking routes from this peer, and sending routes out
     * to the peer.
     *
     * New plumbing:
     *   RibInTable -> DampingTable -> FilterTable -> CacheTable ->..
     *       ..-> NhLookupTable -> DecisionTable.
     *
     *   FanoutTable -> FilterTable -> CacheTable ->..
     *        ..-> RibOutTable -> PeerHandler.
     *
     */

    string peername(peer_handler->peername());


    /*
     * Plumb the input branch
     */
    
    RibInTable<A>* rib_in =
	new RibInTable<A>(_ribname + "RibIn" + peername,
			  _master.safi(),
			  peer_handler);
    _in_map[peer_handler] = rib_in;

    DampingTable<A>* damping_in =
	new DampingTable<A>(_ribname + "Damping" + peername,
			    _master.safi(),
			    rib_in,
			    peer_handler,
			    _master.main().get_local_data()->get_damping());
    rib_in->set_next_table(damping_in);

    FilterTable<A>* filter_in =
	new FilterTable<A>(_ribname + "PeerInputFilter" + peername,
			   _master.safi(),
			   damping_in,
			   _next_hop_resolver);
    filter_in->do_versioning();
    damping_in->set_next_table(filter_in);

    A peer_addr;
    peer_handler->get_peer_addr(peer_addr);
    
    // XXX add get methods
    // XXX what do we do with IPv6?
    A self_addr;
    try {
	self_addr = A(peer_handler->get_local_addr().c_str()); 
    } catch (...) {
    }
    
    PolicyTableImport<A>* policy_filter_in =
	new PolicyTableImport<A>(_ribname + "PeerInputPolicyFilter" + peername,
				 _master.safi(),
				 filter_in,
				 _master.policy_filters(),
				 peer_addr,
				 self_addr);
    filter_in->set_next_table(policy_filter_in);			   

    CacheTable<A>* cache_in = 
	new CacheTable<A>(_ribname + "PeerInputCache" + peername,
			  _master.safi(),
			  policy_filter_in,
			  peer_handler);
    policy_filter_in->set_next_table(cache_in);

    NhLookupTable<A> *nexthop_in =
	new NhLookupTable<A>(_ribname + "NhLookup" + peername,
			     _master.safi(),
			     &_next_hop_resolver,
			     cache_in);
    cache_in->set_next_table(nexthop_in);

    nexthop_in->set_next_table(_decision_table);
    _decision_table->add_parent(nexthop_in, peer_handler, rib_in->genid());

    _tables.insert(rib_in);
    _tables.insert(filter_in);
    _tables.insert(policy_filter_in);
    _tables.insert(cache_in);
    _tables.insert(nexthop_in);

    /*
     * Start things up on the input branch
     */

    /* 1. configure filters */
    configure_inbound_filter(peer_handler, filter_in);

    /* 2. cause all the other peerings to know about this one */
    rib_in->ribin_peering_came_up();
    
    
    /*
     * Plumb the output branch
     */
    FilterTable<A>* filter_out =
	new FilterTable<A>(_ribname + "PeerOutputFilter" + peername,
			   _master.safi(),
			   _fanout_table,
			   _next_hop_resolver);

    PolicyTable<A>* policy_filter_out =
	new PolicyTableExport<A>(_ribname + "PeerOutputPolicyFilter" + peername,
			         _master.safi(),
				 filter_out,
				 _master.policy_filters(),
				 peer_handler->get_peer_addr(),
				 self_addr);
    filter_out->set_next_table(policy_filter_out);

#if 0   
    //
    // outbound cache tables are obsolete
    //
    CacheTable<A>* cache_out = 
	new CacheTable<A>(_ribname + "PeerOutputCache" + peername,
			  _master.safi(),
			  policy_filter_out,
			  peer_handler);
    policy_filter_out->set_next_table(cache_out);
#endif

    RibOutTable<A>* rib_out =
	new RibOutTable<A>(_ribname + "RibOut" + peername,
			   _master.safi(),
			   policy_filter_out,
			   peer_handler);
    policy_filter_out->set_next_table(rib_out);
    _out_map[peer_handler] = rib_out;
    _reverse_out_map[rib_out] = peer_handler;

    _tables.insert(filter_out);
    _tables.insert(policy_filter_out);
    _tables.insert(rib_out);

    /*
     * Start things up on the output branch
     */

    /* 1. configure filters */
    configure_outbound_filter(peer_handler, filter_out);

    /* 2. load up damping filters */
    /* TBD */

    /* 3. finally plumb in the output branch */
    _fanout_table->add_next_table(filter_out, peer_handler, rib_in->genid());

    /* 4. cause the routing table to be dumped to the new peer */
    dump_entire_table(filter_out, _ribname);
    if (_awaits_push)
	push(peer_handler);

    return 0;
}

template <class A>
int 
BGPPlumbingAF<A>::stop_peering(PeerHandler* peer_handler) 
{

    /* Work our way back to the fanout table from the RibOut so we can
       find the relevant output from the fanout table.  On the way,
       flush any caches we find. */
    BGPRouteTable<A> *rt, *prevrt; 
    typename map <PeerHandler*, RibOutTable<A>*>::iterator iter;
    iter = _out_map.find(peer_handler);
    if (iter == _out_map.end()) 
	XLOG_FATAL("BGPPlumbingAF<IPv%u,%s>::stop_peering: peer %p not found",
		   XORP_UINT_CAST(A::ip_version()),
		   pretty_string_safi(_master.safi()),
		   peer_handler);
    rt = iter->second;
    prevrt = rt;
    while (rt != _fanout_table) {
	debug_msg("rt=%p (%s), _fanout_table=%p\n", 
		  rt, rt->tablename().c_str(), _fanout_table);
	if (rt->type() == CACHE_TABLE)
	    ((CacheTable<A>*)rt)->flush_cache();
	prevrt = rt;
	rt = rt->parent();
	if (rt == NULL) {
	    //peering was already stopped.  This can happen when we're
	    //doing an ALLSTOP.
// 	    XLOG_WARNING("BGPPlumbingAF<IPv%u>::stop_peering: "
// 			 "NULL parent table in stop_peering",
// 			 A::ip_version());
	    return 0;
	}
    }

    uint32_t genid;
    typename map <PeerHandler*, RibInTable<A>* >::iterator iter2;
    iter2 = _in_map.find(peer_handler);
    if (iter2 == _in_map.end())
	XLOG_FATAL("BGPPlumbingAF<A>::peering_went_down: peer %p not found",
		   peer_handler);

    RibInTable<A> *rib_in;
    rib_in = iter2->second;
    genid = rib_in->genid();
    prevrt->peering_went_down(peer_handler, genid, _fanout_table);
    prevrt->set_parent(NULL);
    _fanout_table->remove_next_table(prevrt);
    return 0;
}

template <class A>
int 
BGPPlumbingAF<A>::peering_went_down(PeerHandler* peer_handler) 
{
    typename map <PeerHandler*, RibInTable<A>* >::iterator iter;
    iter = _in_map.find(peer_handler);
    if (iter == _in_map.end())
	XLOG_FATAL("BGPPlumbingAF<A>::peering_went_down: peer %p not found",
		   peer_handler);

    RibInTable<A> *rib_in;
    rib_in = iter->second;
    //peering went down will be propagated downstream by the RIB-In.
    TIMESPENT();
    rib_in->ribin_peering_went_down();
    TIMESPENT_CHECK();
    

    //stop_peering shuts down and disconnects the output branch for this peer
    stop_peering(peer_handler);
    TIMESPENT_CHECK();

    /* we don't flush the input caches - lookup requests should still
       be answered until the DeletionTable gets round to telling the
       downstream tables that the route has been deleted */

    return 0;
}

template <class A>
int 
BGPPlumbingAF<A>::peering_came_up(PeerHandler* peer_handler) 
{
    reconfigure_filters(peer_handler);

    //bring the RibIn back up
    typename map <PeerHandler*, RibInTable<A>* >::iterator iter2;
    iter2 = _in_map.find(peer_handler);
    if (iter2 == _in_map.end())
	XLOG_FATAL("BGPPlumbingAF<A>::peering_went_down: peer %p not found",
		   peer_handler);
    RibInTable<A> *rib_in;
    rib_in = iter2->second;
    rib_in->ribin_peering_came_up();

    //plumb the output branch back into the fanout table
    BGPRouteTable<A> *rt, *prevrt;
    typename map <PeerHandler*, RibOutTable<A>*>::iterator iter;
    iter = _out_map.find(peer_handler);
    if (iter == _out_map.end()) 
	XLOG_FATAL("BGPPlumbingAF<A>::peering_came_up: peer %p not found",
		   peer_handler);
    rt = iter->second;
    prevrt = rt;
    while (rt != NULL) {
	debug_msg("rt=%p (%s), _fanout_table=%p\n", 
		  rt, rt->tablename().c_str(), _fanout_table);
	prevrt = rt;
	rt = rt->parent();
    }

    debug_msg("type = %d", prevrt->type());
    FilterTable<A> *filter_out = dynamic_cast<FilterTable<A> *>(prevrt);
    XLOG_ASSERT(filter_out != NULL);

    filter_out->set_parent(_fanout_table);
    _fanout_table->add_next_table(filter_out, peer_handler, rib_in->genid());

    //do the route dump
    dump_entire_table(filter_out, _ribname);

    if (_awaits_push)
	push(peer_handler);
    return 0;
}

template <class A>
int 
BGPPlumbingAF<A>::delete_peering(PeerHandler* peer_handler) 
{
    debug_msg("BGPPlumbingAF<A>::drop_peering\n");

    BGPRouteTable<A> *rt, *parent, *child;

    /*
     * Steps:
     *  1. remove the relevant next_table link from the fanout table.
     *  2. delete add the routes in the RIB_In.
     *  3. remove the relevant parent table link from the decision table.
     *  4. tear down the state
     */

    /*
     * Step 1 - remove the relevant next_table link from the fanout table.
     * This stops us being able to send any updates to this peer.
     * We find the relevant link by tracking back from the RibOut.
     */

    stop_peering(peer_handler);

    /*
     * Step 2 - delete all the affected routes
     */
    
    peering_went_down(peer_handler);

    /*
     * Step 3 - remove the relevant parent link from the decision table
     */

    typename map <PeerHandler*, RibInTable<A>* >::iterator iter2;
    iter2 = _in_map.find(peer_handler);
    child = iter2->second;
    rt = child;
    while(child != _decision_table) {
	rt = child;
	child = rt->next_table();
    }
    _decision_table->remove_parent(rt);

    /* Step 4 - tear down all the tables for this peer */

    rt = iter2->second;
    while(rt != _decision_table) {
	child = rt->next_table();
	_tables.erase(rt);
	delete rt;
	rt = child;
    }

    typename map <PeerHandler*, RibOutTable<A>*>::iterator iter;
    iter = _out_map.find(peer_handler);
    if (iter == _out_map.end())
	XLOG_FATAL("BGPPlumbingAF<A>::drop_peering: peer %p not found",
		   peer_handler);

    iter = _out_map.find(peer_handler);
    rt = iter->second;
    while(rt != NULL) {
	parent = rt->parent();
	if (rt->type() == CACHE_TABLE)
	    ((CacheTable<A>*)rt)->flush_cache();
	_tables.erase(rt);
	delete rt;
	rt = parent;
    }
    return 0;
}

template <class A>
void
BGPPlumbingAF<A>::dump_entire_table(FilterTable<A> *filter_out, string ribname)
{
    debug_msg("BGPPlumbingAF<IPv%u:%s>::dump_entire_table\n",
	      XORP_UINT_CAST(A::ip_version()),
	      pretty_string_safi(_master.safi()));

    _fanout_table->dump_entire_table(filter_out, _master.safi(), ribname);

    DumpTable<A> *dump_table =
	dynamic_cast<DumpTable<A> *>(filter_out->parent());
    XLOG_ASSERT(dump_table);

    /*
    ** It is possible that another peer was in the middle of going
    ** down when this peering came up. So sweep through the peers and
    ** look for the deletion tables. Deletion tables can be
    ** nested. Treat them as if they have just sent a
    ** peering_went_down.
    */

    typename map <PeerHandler*, RibInTable<A>* >::iterator iter2;
    for (iter2 = _in_map.begin(); iter2 != _in_map.end(); iter2++) {
	RibInTable<A> *rib_in = iter2->second;
	debug_msg("<%s>\n", rib_in->next_table()->str().c_str());
	DeletionTable<A> *deletion_table =
	    dynamic_cast<DeletionTable<A> *>(rib_in->next_table());
	while (0 != deletion_table) {
	    debug_msg("Found a deletion table\n");
	    dump_table->peering_is_down(iter2->first, deletion_table->genid());

	    deletion_table = 
		dynamic_cast<DeletionTable<A> *>(deletion_table->next_table());
	}
    }
}

template <class A>
void
BGPPlumbingAF<A>::flush(PeerHandler* peer_handler) 
{
    debug_msg("BGPPlumbingAF<IPv%u:%s>::flush\n",
	      XORP_UINT_CAST(A::ip_version()),
	      pretty_string_safi(_master.safi()));

    RibInTable<A> *rib_in;
    typename map <PeerHandler*, RibInTable<A>* >::iterator iter;
    iter = _in_map.find(peer_handler);
    if (iter == _in_map.end())
	XLOG_FATAL("BGPPlumbingAF<IPv%u:%s>: "
		   "flush called for a PeerHandler "
		   "that has no associated RibIn",
		   XORP_UINT_CAST(A::ip_version()),
		   pretty_string_safi(_master.safi()));

    rib_in = iter->second;
    // Only allow flushing of static routes
    XLOG_ASSERT(rib_in == _ipc_rib_in_table);
    _ipc_rib_in_table->flush();
}

template <class A>
int
BGPPlumbingAF<A>::add_route(const IPNet<A>& net,
			    FPAListRef& pa_list,
			    const PolicyTags& policy_tags,
			    PeerHandler* peer_handler) 
{
    debug_msg("BGPPlumbingAF<IPv%u:%s>::add_route\n",
	      XORP_UINT_CAST(A::ip_version()),
	      pretty_string_safi(_master.safi()));

    int result = 0;
    RibInTable<A> *rib_in;
    typename map <PeerHandler*, RibInTable<A>* >::iterator iter;
    iter = _in_map.find(peer_handler);
    if (iter == _in_map.end())
	XLOG_FATAL("BGPPlumbingAF<IPv%u:%s>: "
		   "add_route called for a PeerHandler "
		   "that has no associated RibIn",
		   XORP_UINT_CAST(A::ip_version()),
		   pretty_string_safi(_master.safi()));

    rib_in = iter->second;
    try {
	result = rib_in->add_route(net, pa_list, policy_tags);
    }
    catch(XorpException &e) {
	XLOG_WARNING("Exception in add_route: %s, assuming failure\n",
		     e.str().c_str());
	result = ADD_FAILURE;
    }

    if (result == ADD_USED || result == ADD_UNUSED) {
	_awaits_push = true;
    } else {
	//XXX the add_route returned an error.
	//Do we want to proactively send a push now?
    }
    return result;
}

template <class A>
int 
BGPPlumbingAF<A>::delete_route(InternalMessage<A> &rtmsg, 
			    PeerHandler* peer_handler) 
{
    int result = 0;
    RibInTable<A> *rib_in;
    typename map <PeerHandler*, RibInTable<A>* >::iterator iter;
    iter = _in_map.find(peer_handler);
    if (iter == _in_map.end())
	XLOG_FATAL("BGPPlumbingAF: delete_route called for a \
PeerHandler that has no associated RibIn");

    rib_in = iter->second;

    result = rib_in->delete_route(rtmsg, NULL);

    if (rtmsg.push() == false) {
	if (result == 0) {
	    _awaits_push = true;
	} else {
	    //XXX the delete_route returned an error.
	    //Do we want to proactively send a push now?
	}
    }
    return result;
}

template <class A>
int 
BGPPlumbingAF<A>::delete_route(const IPNet<A>& net,
			       PeerHandler* peer_handler) 
{
    int result = 0;
    RibInTable<A> *rib_in;
    typename map <PeerHandler*, RibInTable<A>* >::iterator iter;
    iter = _in_map.find(peer_handler);
    if (iter == _in_map.end())
	XLOG_FATAL("BGPPlumbingAF: delete_route called for a \
PeerHandler that has no associated RibIn");

    rib_in = iter->second;
#if 0
    // no need to go to all the effort to create a full message here,
    // as we'll do the lookup in ribin anyway
    uint32_t genid;
    FPAListRef pa_list;
    const SubnetRoute<A> *found_route = rib_in->lookup_route(net, genid, pa_list);
    if (found_route == NULL) {
	XLOG_WARNING("Attempt to delete non existent route %s",
		     net.str().c_str());
	return result;
    }
    InternalMessage<A> rtmsg(found_route, pa_list, peer_handler, GENID_UNKNOWN);
    result = rib_in->delete_route(rtmsg, NULL);
#endif
    result = rib_in->delete_route(net);
    return result;
}

template <class A>
void
BGPPlumbingAF<A>::push(PeerHandler* peer_handler) 
{
    debug_msg("BGPPlumbingAF<IPv%u:%s>::push\n",
	      XORP_UINT_CAST(A::ip_version()),
	      pretty_string_safi(_master.safi()));
    if (_awaits_push == false) {
	XLOG_WARNING("push <IPv%u:%s> when none needed",
		     XORP_UINT_CAST(A::ip_version()),
		     pretty_string_safi(_master.safi()));
	return;
    }
    RibInTable<A> *rib_in;
    typename map <PeerHandler*, RibInTable<A>* >::iterator iter;
    iter = _in_map.find(peer_handler);
    if (iter == _in_map.end())
	XLOG_FATAL("BGPPlumbingAF: Push called for a PeerHandler \
that has no associated RibIn");

    rib_in = iter->second;

    rib_in->push(NULL);
}

template <class A>
void
BGPPlumbingAF<A>::output_no_longer_busy(PeerHandler *peer_handler) 
{
    RibOutTable<A> *rib_out;
    typename map <PeerHandler*, RibOutTable<A>* >::iterator iter;
    iter = _out_map.find(peer_handler);
    if (iter == _out_map.end())
	XLOG_FATAL("BGPPlumbingAF: output_no_longer_busy called for a \
PeerHandler that has no associated RibOut");
    rib_out = iter->second;
    rib_out->output_no_longer_busy();
}

template <class A>
const SubnetRoute<A>* 
BGPPlumbingAF<A>::lookup_route(const IPNet<A> &net) const 
{
    //lookup_route returns the route currently being told to the RIB.
    //It's possible this differs from the route we tell a peer,
    //because of output filters that may modify attributes.
    uint32_t genid;
    FPAListRef pa_list;
    return _ipc_rib_out_table->lookup_route(net, genid, pa_list);
}

template <class A>
uint32_t
BGPPlumbingAF<A>::get_prefix_count(PeerHandler* peer_handler) const
{
    typename map <PeerHandler*, RibInTable<A>* >::const_iterator iter;
    iter = _in_map.find(peer_handler);
    if (iter == _in_map.end())
	XLOG_FATAL("BGPPlumbingAF: Get prefix count for a PeerHandler \
that has no associated RibIn");

    return iter->second->route_count();
}

template <>
const IPv4& 
BGPPlumbingAF<IPv4>::get_local_nexthop(const PeerHandler *peerhandler) const 
{
    return peerhandler->my_v4_nexthop();
}

template <>
bool
BGPPlumbingAF<IPv4>::directly_connected(const PeerHandler *peer_handler,
					IPNet<IPv4>& subnet, IPv4& p) const
{
    // Get the routers interface address.
    // There is no reason for the transport address for the session to
    // match the address family, hence the necessity to catch a
    // possible exception (IPv6 session with IPv4 AFI).
    try {
	IPv4 local(peer_handler->get_local_addr().c_str());
	IPv4 peer(peer_handler->get_peer_addr().c_str());

	uint32_t prefix_len;
	if (!_master.main().interface_address_prefix_len4(local, prefix_len))
	    return false;

	IPNet<IPv4> net(local, prefix_len);
	if (net.contains(peer)) {
	    subnet = net;
	    p = peer;
	    return true;
	}
	
    } catch(...) {
	return false;
    }

    return false;
}

template <class A>
list <RibInTable<A>*>
BGPPlumbingAF<A>::ribin_list() const 
{
    list <RibInTable<A>*> _ribin_list;
    typename map <PeerHandler*, RibInTable<A>* >::const_iterator i;
    for (i = _in_map.begin(); i != _in_map.end(); i++) {
	_ribin_list.push_back(i->second);
    }
    return _ribin_list;
}

template <class A>
uint32_t 
BGPPlumbingAF<A>::create_route_table_reader(const IPNet<A>& prefix)
{
    //Generate a new token that can't clash with any in use, even if
    //the space wraps.
    _max_reader_token++;
    while (_route_table_readers.find(_max_reader_token) 
	   != _route_table_readers.end()) {
	_max_reader_token++;
    }

    RouteTableReader<A> *new_reader = new RouteTableReader<A>(ribin_list(),
							      prefix);
    _route_table_readers[_max_reader_token] = new_reader;
    return _max_reader_token;
}

template <class A>
bool 
BGPPlumbingAF<A>::read_next_route(uint32_t token, 
				  const SubnetRoute<A>*& route, 
				  IPv4& peer_id) 
{
    typename map <uint32_t, RouteTableReader<A>*>::iterator i;
    i = _route_table_readers.find(token);
    if (i == _route_table_readers.end())
	return false;
    RouteTableReader<A> *_reader = i->second;
    bool result = _reader->get_next(route, peer_id);
    if (result == false) {
	//we've finished reading the routing table.
	_route_table_readers.erase(i);
	delete _reader;
    }
    return result;
}

template <class A>
bool
BGPPlumbingAF<A>::status(string&) const
{
    return true;
}


template <class A>
void
BGPPlumbingAF<A>::push_routes() {
    list<const PeerTableInfo<A>*> peer_list;

    _fanout_table->peer_table_info(peer_list);

    _policy_sourcematch_table->push_routes(peer_list);

    /*
    ** It is possible that another peer was in the middle of going
    ** down when this peering came up. So sweep through the peers and
    ** look for the deletion tables. Deletion tables can be
    ** nested. Treat them as if they have just sent a
    ** peering_went_down.
    */

    typename map <PeerHandler*, RibInTable<A>* >::iterator iter2;
    for (iter2 = _in_map.begin(); iter2 != _in_map.end(); iter2++) {
	RibInTable<A> *rib_in = iter2->second;
	debug_msg("<%s>\n", rib_in->next_table()->str().c_str());
	DeletionTable<A> *deletion_table =
	    dynamic_cast<DeletionTable<A> *>(rib_in->next_table());
	while (0 != deletion_table) {
	    debug_msg("Found a deletion table\n");
	    _policy_sourcematch_table
		->peering_is_down(iter2->first, deletion_table->genid());
	    deletion_table = 
		dynamic_cast<DeletionTable<A> *>(deletion_table->next_table());
	}
    }

}

template class BGPPlumbingAF<IPv4>;

/** IPv6 stuff */
#ifdef HAVE_IPV6


int 
BGPPlumbing::add_route(const IPv6Net& net,
		       FPAList6Ref& pa_list,
		       const PolicyTags& policy_tags,
		       PeerHandler* peer_handler)  
{
    debug_msg("BGPPlumbing::add_route IPv6\n");
    PROFILE(if (main().profile().enabled(profile_route_ribin))
		main().profile().log(profile_route_ribin,
				     c_format("add %s", net.str().c_str())));

    XLOG_ASSERT(!pa_list->is_locked());
    return plumbing_ipv6().add_route(net, pa_list, policy_tags, peer_handler);
}


int 
BGPPlumbing::delete_route(InternalMessage<IPv6> &rtmsg, 
			  PeerHandler* peer_handler) 
{
    PROFILE(if (main().profile().enabled(profile_route_ribin))
		main().profile().log(profile_route_ribin,
				     c_format("delete %s", rtmsg.net().str().c_str())));

    return plumbing_ipv6().delete_route(rtmsg, peer_handler);
}

int 
BGPPlumbing::delete_route(const IPNet<IPv6>& net,
			  PeerHandler* peer_handler) 
{
    PROFILE(if (main().profile().enabled(profile_route_ribin))
		main().profile().log(profile_route_ribin,
				     c_format("delete %s", net.str().c_str())));
    return plumbing_ipv6().delete_route(net, peer_handler);
}

const SubnetRoute<IPv6>* 
BGPPlumbing::lookup_route(const IPNet<IPv6> &net) const 
{
    return const_cast<BGPPlumbing *>(this)->
	plumbing_ipv6().lookup_route(net);
}

template<>
void
BGPPlumbing::push<IPv6>(PeerHandler* peer_handler) 
{
    debug_msg("BGPPlumbing::push\n");
    plumbing_ipv6().push(peer_handler);
}


template<>
uint32_t 
BGPPlumbing::create_route_table_reader<IPv6>(const IPNet<IPv6>& prefix)
{
    return plumbing_ipv6().create_route_table_reader(prefix);
}

bool 
BGPPlumbing::read_next_route(uint32_t token, 
			     const SubnetRoute<IPv6>*& route, 
			     IPv4& peer_id)
{
    return plumbing_ipv6().read_next_route(token, route, peer_id);
}

template <>
const IPv6& 
BGPPlumbingAF<IPv6>::get_local_nexthop(const PeerHandler *peerhandler) const 
{
    return peerhandler->my_v6_nexthop();
}

template <>
bool
BGPPlumbingAF<IPv6>::directly_connected(const PeerHandler *peer_handler,
					IPNet<IPv6>& subnet, IPv6& p) const
{
    // Get the routers interface address.
    // There is no reason for the transport address for the session to
    // match the address family, hence the necessity to catch a
    // possible exception (IPv4 session with IPv6 AFI).
    try {
	IPv6 local(peer_handler->get_local_addr().c_str());
	IPv6 peer(peer_handler->get_peer_addr().c_str());

	uint32_t prefix_len;
	if (!_master.main().interface_address_prefix_len6(local, prefix_len))
	    return false;

	IPNet<IPv6> net(local, prefix_len);
	if (net.contains(peer)) {
	    subnet = net;
	    p = peer;
	    return true;
	}
	
    } catch(...) {
	return false;
    }

    return false;
}

template class BGPPlumbingAF<IPv6>;


#endif // ipv6
