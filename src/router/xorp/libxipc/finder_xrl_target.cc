// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
// vim:set sts=4 ts=8:

// Copyright (c) 2001-2011 XORP, Inc and Others
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License, Version
// 2.1, June 1999 as published by the Free Software Foundation.
// Redistribution and/or modification of this program under the terms of
// any other version of the GNU Lesser General Public License is not
// permitted.
// 
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For more details,
// see the GNU Lesser General Public License, Version 2.1, a copy of
// which can be found in the XORP LICENSE.lgpl file.
// 
// XORP, Inc, 2953 Bunker Hill Lane, Suite 204, Santa Clara, CA 95054, USA;
// http://xorp.net



#include "libxorp/xorp.h"
#include "libxorp/debug.h"
#include "libxorp/status_codes.h"
#include "libxorp/random.h"

#include "finder_xrl_target.hh"
#include "finder.hh"
#include "permits.hh"
#include "xuid.hh"

static class TraceFinder
{
public:
    TraceFinder() {
	_do_trace = !(getenv("FINDERTRACE") == 0);
    }
    bool on() const { return _do_trace; }
    operator bool() { return _do_trace; }
    void set_context(const string& s) { _context = s; }
    const string& context() const { return _context; }
protected:
    bool _do_trace;
    string _context;
} finder_tracer;

#define finder_trace_init(x...) 					      \
do {									      \
    if (finder_tracer.on()) {						      \
	finder_tracer.set_context(c_format(x));				      \
    }									      \
} while (0)

#define finder_trace_result(x...)					      \
do {									      \
    if (finder_tracer.on()) {						      \
	string r = c_format(x);						      \
	XLOG_INFO("%s -> %s", finder_tracer.context().c_str(), r.c_str());    \
    }									      \
} while (0)


/**
 * Helper method to pass back consistent message when it is discovered that
 * a client is trying to manipulate state for a non-existent target or
 * a target it is not registered to administer.
 */
static inline string
restricted_target_message(const string& tgt_name)
{
    return c_format("Target \"%s\" does not exist "
		    "or caller is not responsible "
		    "for it.", tgt_name.c_str());
}

static inline string
bad_target_message(const string& tgt_name)
{
    return c_format("Target \"%s\" does not exist or is not enabled.",
		    tgt_name.c_str());
}


static string
make_cookie()
{
    static uint32_t invoked = 0;
    static uint32_t hash_base;
    if (invoked == 0) {
	xorp_srandom(((unsigned long)(getpid())) ^ ((unsigned long)(&hash_base)));
	invoked = xorp_random() ^ xorp_random();
	hash_base = xorp_random();
    }
    uint32_t r = xorp_random();
    invoked++;
    return c_format("%08x%08x", XORP_UINT_CAST(invoked),
		    XORP_UINT_CAST(r ^ hash_base));
}


FinderXrlTarget::FinderXrlTarget(Finder& finder)
    : XrlFinderTargetBase(&(finder.commands())), _finder(finder)
{
}

XrlCmdError
FinderXrlTarget::common_0_1_get_target_name(string& name)
{
    name = XrlFinderTargetBase::get_name();
    return XrlCmdError::OKAY();
}

XrlCmdError
FinderXrlTarget::common_0_1_get_version(string& name)
{
    name = XrlFinderTargetBase::version();
    return XrlCmdError::OKAY();
}

XrlCmdError
FinderXrlTarget::common_0_1_get_status(uint32_t& status, string& reason)
{
    //the finder is always ready if it can receive an XRL request.
    status = PROC_READY;
    reason = "Ready";
    return XrlCmdError::OKAY();
}

XrlCmdError
FinderXrlTarget::common_0_1_shutdown()
{
    //XXX it's not clear to me if we want to support shutdown on the finder...
    exit(0);
}

XrlCmdError
FinderXrlTarget::finder_0_2_register_finder_client(const string& tgt_name,
						   const string& class_name,
						   const bool&	 singleton,
						   const string& in_cookie,
						   string&	 out_cookie)
{
    finder_trace_init("register_finder_client(target = \"%s\", "
		      "class = \"%s\", singleton = \"%d\", "
		      "cookie = \"%s\")",
		      tgt_name.c_str(), class_name.c_str(),
		      singleton, in_cookie.c_str());

    if (in_cookie.empty() == false) {
	out_cookie = in_cookie;
	_finder.remove_target_with_cookie(out_cookie);
    } else {
	out_cookie = make_cookie();
    }

    if (_finder.add_target(class_name, tgt_name, singleton, out_cookie)) {
	finder_trace_result("\"%s\" okay",  out_cookie.c_str());
	return XrlCmdError::OKAY();
    }

    finder_trace_result("failed (already registered)");
    return XrlCmdError::COMMAND_FAILED(c_format("%s already registered.",
						tgt_name.c_str()));
}

XrlCmdError
FinderXrlTarget::finder_0_2_unregister_finder_client(const string& tgt_name)
{
    finder_trace_init("unregister_finder_client(\"%s\")", tgt_name.c_str());

    if (_finder.active_messenger_represents_target(tgt_name)) {
	_finder.remove_target(tgt_name);
	finder_trace_result("okay");
	return XrlCmdError::OKAY();
    }

    finder_trace_result("failed");

    return XrlCmdError::COMMAND_FAILED(restricted_target_message(tgt_name));
}

XrlCmdError
FinderXrlTarget::finder_0_2_set_finder_client_enabled(const string& tgt_name,
						      const bool&   en)
{
    finder_trace_init("set_finder_client_enabled(\"%s\", %s)",
		      tgt_name.c_str(), bool_c_str(en));

    if (_finder.active_messenger_represents_target(tgt_name)) {
	_finder.set_target_enabled(tgt_name, en);
	finder_trace_result("okay");
	return XrlCmdError::OKAY();
    }
    finder_trace_result("failed (not originator)");
    return XrlCmdError::COMMAND_FAILED(restricted_target_message(tgt_name));
}

XrlCmdError
FinderXrlTarget::finder_0_2_finder_client_enabled(const string& tgt_name,
						  bool&         en)
{
    finder_trace_init("finder_client_enabled(\"%s\")",
		      tgt_name.c_str());

    if (_finder.target_enabled(tgt_name, en) == false) {
	finder_trace_result("failed (invalid target name)");
	return XrlCmdError::COMMAND_FAILED(
		c_format("Invalid target name \"%s\"", tgt_name.c_str()));
    }
    return XrlCmdError::OKAY();
}

XrlCmdError
FinderXrlTarget::finder_0_2_add_xrl(const string& xrl,
				    const string& protocol_name,
				    const string& protocol_args,
				    string&	  resolved_xrl_method_name)
{
    Xrl u;

    finder_trace_init("add_xrl(\"%s\", \"%s\", \"%s\")",
		      xrl.c_str(), protocol_name.c_str(),
		      protocol_args.c_str());

    // Construct unresolved Xrl
    try {
	u = Xrl(xrl.c_str());
    } catch (InvalidString&) {
	finder_trace_result("fail (bad xrl).");
	return XrlCmdError::COMMAND_FAILED("Invalid xrl string");
    }

    // Check active messenger is responsible for target described in
    // unresolved Xrl
    if (false == _finder.active_messenger_represents_target(u.target())) {
	finder_trace_result("fail (inappropriate message source).");
	return
	    XrlCmdError::COMMAND_FAILED(restricted_target_message(u.target()));
    }

    // Construct resolved Xrl, appended string should very hard to guess :-)
    resolved_xrl_method_name = u.command() + "-" + make_cookie();
    Xrl r(protocol_name, protocol_args, resolved_xrl_method_name);

    // Register Xrl
    if (false == _finder.add_resolution(u.target(), u.str(), r.str())) {
	finder_trace_result("fail (already registered).");
	return XrlCmdError::COMMAND_FAILED("Xrl already registered");
    }
    finder_trace_result("okay");
    return XrlCmdError::OKAY();
}

XrlCmdError
FinderXrlTarget::finder_0_2_remove_xrl(const string&	xrl)
{
    Xrl u;

    finder_trace_init("remove_xrl(\"%s\")", xrl.c_str());

    // Construct Xrl
    try {
	u = Xrl(xrl.c_str());
    } catch (InvalidString&) {
	finder_trace_result("fail (bad xrl).");
	return XrlCmdError::COMMAND_FAILED("Invalid xrl string");
    }

    // Check active messenger is responsible for target described in Xrl
    if (false == _finder.active_messenger_represents_target(u.target())) {
	finder_trace_result("fail (inappropriate message source).");
	return
	    XrlCmdError::COMMAND_FAILED(restricted_target_message(u.target()));
    }

    // Unregister Xrl
    if (false == _finder.remove_resolutions(u.target(), u.str())) {
	finder_trace_result("fail (xrl does not exist).");
	return
	    XrlCmdError::COMMAND_FAILED(restricted_target_message(u.target()));
    }
    finder_trace_result("okay");
    return XrlCmdError::OKAY();
}

XrlCmdError
FinderXrlTarget::finder_0_2_resolve_xrl(const string&	xrl,
					XrlAtomList&	resolved_xrls)
{
    finder_trace_init("resolve_xrl(\"%s\")", xrl.c_str());

    Xrl u;

    //
    // Construct Xrl
    //
    try {
	u = Xrl(xrl.c_str());
    } catch (InvalidString&) {
	finder_trace_result("fail (bad xrl).");
	return XrlCmdError::COMMAND_FAILED("Invalid xrl string");
    }

    //
    // Xrl may need resolving by class
    //
    const string& instance = _finder.primary_instance(u.target());
    if (u.target() != instance) {
	try {
	    u = Xrl(u.protocol(), instance, u.command());
	} catch (InvalidString&) {
	    finder_trace_result("fail (bad class to instance mapping).");
	    return XrlCmdError::COMMAND_FAILED("Invalid xrl string");
	}
    }

    //
    // Check instance exists and is enabled
    //
    bool en;
    if (_finder.target_enabled(instance, en) == false) {
	finder_trace_result("fail (target does not exist).");
	return XrlCmdError::COMMAND_FAILED(bad_target_message(instance));
    } else if (en == false) {
	finder_trace_result("fail (xrl exists but is not enabled).");
	return XrlCmdError::COMMAND_FAILED("Xrl target is not enabled.");
    }

    const Finder::Resolveables* resolutions = _finder.resolve(instance,
							      u.str());
    if (0 == resolutions) {
	finder_trace_result("fail (does not resolve).");
	return XrlCmdError::COMMAND_FAILED("Xrl does not resolve: " + xrl);
    }

    Finder::Resolveables::const_iterator ci = resolutions->begin();
    while (resolutions->end() != ci) {
	string s;
	try {
	    s = Xrl(ci->c_str()).str();
	} catch (const InvalidString& ) {
	    finder_trace_result("fail (does not resolve as an xrl).");
	    XLOG_ERROR("Resolved something that did not look an xrl: \"%s\"\n",
		       ci->c_str());
	}
	resolved_xrls.append(XrlAtom(s));
	++ci;
    }
    finder_trace_result("resolves okay.");
    return XrlCmdError::OKAY();
}

XrlCmdError
FinderXrlTarget::finder_0_2_get_xrl_targets(XrlAtomList& xal)
{
    list<string> tgts;

    _finder.fill_target_list(tgts);

    // Special case, add finder itself to list
    tgts.push_back("finder");
    tgts.sort();

    for (list<string>::const_iterator i = tgts.begin(); i != tgts.end(); i++) {
	xal.append(XrlAtom(*i));
    }

    return XrlCmdError::OKAY();
}

XrlCmdError
FinderXrlTarget::finder_0_2_get_xrls_registered_by(const string& tgt,
						   XrlAtomList&  xal)
{
    list<string> xrls;

    // Special case, finder request
    if (tgt == "finder") {
	list<string> cmds;
	_finder.commands().get_command_names(cmds);
	// Turn command names into Xrls
	for (list<string>::iterator i = cmds.begin(); i != cmds.end(); i++) {
	    xrls.push_back(Xrl("finder", i->c_str()).str());
	}
    } else if (_finder.fill_targets_xrl_list(tgt, xrls) == false) {
	return
	    XrlCmdError::COMMAND_FAILED
	    (c_format("No such target \"%s\"", tgt.c_str()));
    }
    for (list<string>::const_iterator i = xrls.begin(); i != xrls.end(); i++) {
	xal.append(XrlAtom(*i));
    }
    return XrlCmdError::OKAY();
}

XrlCmdError
FinderXrlTarget::finder_0_2_get_ipv4_permitted_hosts(XrlAtomList& ipv4hosts)
{
    const IPv4Hosts& hl = permitted_ipv4_hosts();
    for (IPv4Hosts::const_iterator ci = hl.begin(); ci != hl.end(); ++ci)
	ipv4hosts.append(XrlAtom(*ci));

    return XrlCmdError::OKAY();
}

XrlCmdError
FinderXrlTarget::finder_0_2_get_ipv4_permitted_nets(XrlAtomList& ipv4nets)
{
    const IPv4Nets& nl = permitted_ipv4_nets();
    for (IPv4Nets::const_iterator ci = nl.begin(); ci != nl.end(); ++ci)
	ipv4nets.append(XrlAtom(*ci));

    return XrlCmdError::OKAY();
}

XrlCmdError
FinderXrlTarget::finder_0_2_get_ipv6_permitted_hosts(XrlAtomList& ipv6hosts)
{
    const IPv6Hosts& hl = permitted_ipv6_hosts();
    for (IPv6Hosts::const_iterator ci = hl.begin(); ci != hl.end(); ++ci)
	ipv6hosts.append(XrlAtom(*ci));

    return XrlCmdError::OKAY();
}

XrlCmdError
FinderXrlTarget::finder_0_2_get_ipv6_permitted_nets(XrlAtomList& ipv6nets)
{
    const IPv6Nets& nl = permitted_ipv6_nets();
    for (IPv6Nets::const_iterator ci = nl.begin(); ci != nl.end(); ++ci)
	ipv6nets.append(XrlAtom(*ci));

    return XrlCmdError::OKAY();
}

XrlCmdError
FinderXrlTarget::finder_event_notifier_0_1_register_class_event_interest(
						 const string& who,
						 const string& class_name
						 )
{
    finder_trace_init("register_class_event_interest (who = %s, class = %s)",
		      who.c_str(), class_name.c_str());
    string err_msg;
    if (_finder.active_messenger_represents_target(who) == false) {
	finder_trace_result("messenger does not represent target.");
	return XrlCmdError::COMMAND_FAILED("failed (not originator).");
    }
    if (_finder.add_class_watch(who, class_name, err_msg) == false) {
	string em = c_format("failed to add class event watch, who: %s  class_name: %s  err: %s\n",
			     who.c_str(), class_name.c_str(), err_msg.c_str());
	finder_trace_result("%s", em.c_str());
	return XrlCmdError::COMMAND_FAILED(em);
    }
    finder_trace_result("okay");
    return XrlCmdError::OKAY();
}

XrlCmdError
FinderXrlTarget::finder_event_notifier_0_1_deregister_class_event_interest(
						 const string& who,
						 const string& class_name
						 )
{
    finder_trace_init("deregister_class_event_interest (who = %s, class = %s)",
		      who.c_str(), class_name.c_str());
    if (_finder.active_messenger_represents_target(who) == false) {
	finder_trace_result("messenger does not represent target.");
	return XrlCmdError::COMMAND_FAILED("failed (not originator).");
    }
    if (_finder.remove_class_watch(who, class_name)) {
	finder_trace_result("okay, but watch was non-existent.");
	return XrlCmdError::OKAY();
    }
    finder_trace_result("okay");
    return XrlCmdError::OKAY();
}

XrlCmdError
FinderXrlTarget::finder_event_notifier_0_1_register_instance_event_interest(
						 const string& who,
						 const string& instance_name
						 )
{
    string err_msg;
    finder_trace_init("register_instance_event_interest (who = %s, instance = %s)",
		      who.c_str(), instance_name.c_str());
    if (_finder.active_messenger_represents_target(who) == false) {
	finder_trace_result("messenger does not represent target.");
	return XrlCmdError::COMMAND_FAILED("failed (not originator).");
    }
    if (_finder.add_instance_watch(who, instance_name, err_msg) == false) {
	string em = c_format("failed to add instance event watch, who: %s  instance_name: %s  err: %s\n",
			     who.c_str(), instance_name.c_str(), err_msg.c_str());
	finder_trace_result("%s", em.c_str());
	return XrlCmdError::COMMAND_FAILED(em.c_str());
    }
    finder_trace_result("okay");
    return XrlCmdError::OKAY();
}

XrlCmdError
FinderXrlTarget::finder_event_notifier_0_1_deregister_instance_event_interest(
						 const string& who,
						 const string& instance_name
						 )
{
    finder_trace_init("deregister_instance_event_interest (who = %s, instance = %s)",
		      who.c_str(), instance_name.c_str());
    if (_finder.active_messenger_represents_target(who) == false) {
	finder_trace_result("messenger does not represent target.");
	return XrlCmdError::COMMAND_FAILED("failed (not originator).");
    }
    if (_finder.remove_instance_watch(who, instance_name)) {
	finder_trace_result("okay, but watch was non-existent.");
	return XrlCmdError::OKAY();
    }
    finder_trace_result("okay");
    return XrlCmdError::OKAY();
}
