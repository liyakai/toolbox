/*
 * obj/x86_64-unknown-linux-gnu/xrl/interfaces/finder_xif.cc
 * vim:set sts=4 ts=8 ft=cpp:
 *
 * Copyright (c) 2001-2011 XORP, Inc and Others
 * See the XORP LICENSE.lgpl file for licensing, conditions, and warranties
 * on use.
 *
 * DO NOT EDIT THIS FILE - IT IS PROGRAMMATICALLY GENERATED
 *
 * Generated by 'clnt-gen'.
 */

#include "finder_xif.hh"

bool
XrlFinderV0p2Client::send_register_finder_client(
	const char*	dst_xrl_target_name,
	const string&	instance_name,
	const string&	class_name,
	const bool&	singleton,
	const string&	in_cookie,
	const RegisterFinderClientCB&	cb
)
{
    Xrl* x = ap_xrl_register_finder_client.get();

    if (!x) {
        x = new Xrl(dst_xrl_target_name, "finder/0.2/register_finder_client");
        x->args().add("instance_name", instance_name);
        x->args().add("class_name", class_name);
        x->args().add("singleton", singleton);
        x->args().add("in_cookie", in_cookie);
        ap_xrl_register_finder_client.reset(x);
    }

    x->set_target(dst_xrl_target_name);

    x->args().set_arg(0, instance_name);
    x->args().set_arg(1, class_name);
    x->args().set_arg(2, singleton);
    x->args().set_arg(3, in_cookie);

    return _sender->send(*x, callback(&XrlFinderV0p2Client::unmarshall_register_finder_client, cb));
}


/* Unmarshall register_finder_client */
void
XrlFinderV0p2Client::unmarshall_register_finder_client(
	const XrlError&	e,
	XrlArgs*	a,
	RegisterFinderClientCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e, 0);
	return;
    } else if (a && a->size() != 1) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(1));
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    string out_cookie;
    try {
	a->get("out_cookie", out_cookie);
    } catch (const XrlArgs::BadArgs& bad_args_err) {
	UNUSED(bad_args_err);
	XLOG_ERROR("Error decoding the arguments: %s", bad_args_err.str().c_str());
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    cb->dispatch(e, &out_cookie);
}

bool
XrlFinderV0p2Client::send_unregister_finder_client(
	const char*	dst_xrl_target_name,
	const string&	instance_name,
	const UnregisterFinderClientCB&	cb
)
{
    Xrl* x = ap_xrl_unregister_finder_client.get();

    if (!x) {
        x = new Xrl(dst_xrl_target_name, "finder/0.2/unregister_finder_client");
        x->args().add("instance_name", instance_name);
        ap_xrl_unregister_finder_client.reset(x);
    }

    x->set_target(dst_xrl_target_name);

    x->args().set_arg(0, instance_name);

    return _sender->send(*x, callback(&XrlFinderV0p2Client::unmarshall_unregister_finder_client, cb));
}


/* Unmarshall unregister_finder_client */
void
XrlFinderV0p2Client::unmarshall_unregister_finder_client(
	const XrlError&	e,
	XrlArgs*	a,
	UnregisterFinderClientCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e);
	return;
    } else if (a && a->size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(0));
	cb->dispatch(XrlError::BAD_ARGS());
	return;
    }
    cb->dispatch(e);
}

bool
XrlFinderV0p2Client::send_set_finder_client_enabled(
	const char*	dst_xrl_target_name,
	const string&	instance_name,
	const bool&	enabled,
	const SetFinderClientEnabledCB&	cb
)
{
    Xrl* x = ap_xrl_set_finder_client_enabled.get();

    if (!x) {
        x = new Xrl(dst_xrl_target_name, "finder/0.2/set_finder_client_enabled");
        x->args().add("instance_name", instance_name);
        x->args().add("enabled", enabled);
        ap_xrl_set_finder_client_enabled.reset(x);
    }

    x->set_target(dst_xrl_target_name);

    x->args().set_arg(0, instance_name);
    x->args().set_arg(1, enabled);

    return _sender->send(*x, callback(&XrlFinderV0p2Client::unmarshall_set_finder_client_enabled, cb));
}


/* Unmarshall set_finder_client_enabled */
void
XrlFinderV0p2Client::unmarshall_set_finder_client_enabled(
	const XrlError&	e,
	XrlArgs*	a,
	SetFinderClientEnabledCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e);
	return;
    } else if (a && a->size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(0));
	cb->dispatch(XrlError::BAD_ARGS());
	return;
    }
    cb->dispatch(e);
}

bool
XrlFinderV0p2Client::send_finder_client_enabled(
	const char*	dst_xrl_target_name,
	const string&	instance_name,
	const FinderClientEnabledCB&	cb
)
{
    Xrl* x = ap_xrl_finder_client_enabled.get();

    if (!x) {
        x = new Xrl(dst_xrl_target_name, "finder/0.2/finder_client_enabled");
        x->args().add("instance_name", instance_name);
        ap_xrl_finder_client_enabled.reset(x);
    }

    x->set_target(dst_xrl_target_name);

    x->args().set_arg(0, instance_name);

    return _sender->send(*x, callback(&XrlFinderV0p2Client::unmarshall_finder_client_enabled, cb));
}


/* Unmarshall finder_client_enabled */
void
XrlFinderV0p2Client::unmarshall_finder_client_enabled(
	const XrlError&	e,
	XrlArgs*	a,
	FinderClientEnabledCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e, 0);
	return;
    } else if (a && a->size() != 1) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(1));
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    bool enabled;
    try {
	a->get("enabled", enabled);
    } catch (const XrlArgs::BadArgs& bad_args_err) {
	UNUSED(bad_args_err);
	XLOG_ERROR("Error decoding the arguments: %s", bad_args_err.str().c_str());
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    cb->dispatch(e, &enabled);
}

bool
XrlFinderV0p2Client::send_add_xrl(
	const char*	dst_xrl_target_name,
	const string&	xrl,
	const string&	protocol_name,
	const string&	protocol_args,
	const AddXrlCB&	cb
)
{
    Xrl* x = ap_xrl_add_xrl.get();

    if (!x) {
        x = new Xrl(dst_xrl_target_name, "finder/0.2/add_xrl");
        x->args().add("xrl", xrl);
        x->args().add("protocol_name", protocol_name);
        x->args().add("protocol_args", protocol_args);
        ap_xrl_add_xrl.reset(x);
    }

    x->set_target(dst_xrl_target_name);

    x->args().set_arg(0, xrl);
    x->args().set_arg(1, protocol_name);
    x->args().set_arg(2, protocol_args);

    return _sender->send(*x, callback(&XrlFinderV0p2Client::unmarshall_add_xrl, cb));
}


/* Unmarshall add_xrl */
void
XrlFinderV0p2Client::unmarshall_add_xrl(
	const XrlError&	e,
	XrlArgs*	a,
	AddXrlCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e, 0);
	return;
    } else if (a && a->size() != 1) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(1));
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    string resolved_xrl_method_name;
    try {
	a->get("resolved_xrl_method_name", resolved_xrl_method_name);
    } catch (const XrlArgs::BadArgs& bad_args_err) {
	UNUSED(bad_args_err);
	XLOG_ERROR("Error decoding the arguments: %s", bad_args_err.str().c_str());
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    cb->dispatch(e, &resolved_xrl_method_name);
}

bool
XrlFinderV0p2Client::send_remove_xrl(
	const char*	dst_xrl_target_name,
	const string&	xrl,
	const RemoveXrlCB&	cb
)
{
    Xrl* x = ap_xrl_remove_xrl.get();

    if (!x) {
        x = new Xrl(dst_xrl_target_name, "finder/0.2/remove_xrl");
        x->args().add("xrl", xrl);
        ap_xrl_remove_xrl.reset(x);
    }

    x->set_target(dst_xrl_target_name);

    x->args().set_arg(0, xrl);

    return _sender->send(*x, callback(&XrlFinderV0p2Client::unmarshall_remove_xrl, cb));
}


/* Unmarshall remove_xrl */
void
XrlFinderV0p2Client::unmarshall_remove_xrl(
	const XrlError&	e,
	XrlArgs*	a,
	RemoveXrlCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e);
	return;
    } else if (a && a->size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(0));
	cb->dispatch(XrlError::BAD_ARGS());
	return;
    }
    cb->dispatch(e);
}

bool
XrlFinderV0p2Client::send_resolve_xrl(
	const char*	dst_xrl_target_name,
	const string&	xrl,
	const ResolveXrlCB&	cb
)
{
    Xrl* x = ap_xrl_resolve_xrl.get();

    if (!x) {
        x = new Xrl(dst_xrl_target_name, "finder/0.2/resolve_xrl");
        x->args().add("xrl", xrl);
        ap_xrl_resolve_xrl.reset(x);
    }

    x->set_target(dst_xrl_target_name);

    x->args().set_arg(0, xrl);

    return _sender->send(*x, callback(&XrlFinderV0p2Client::unmarshall_resolve_xrl, cb));
}


/* Unmarshall resolve_xrl */
void
XrlFinderV0p2Client::unmarshall_resolve_xrl(
	const XrlError&	e,
	XrlArgs*	a,
	ResolveXrlCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e, 0);
	return;
    } else if (a && a->size() != 1) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(1));
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    XrlAtomList resolutions;
    try {
	a->get("resolutions", resolutions);
    } catch (const XrlArgs::BadArgs& bad_args_err) {
	UNUSED(bad_args_err);
	XLOG_ERROR("Error decoding the arguments: %s", bad_args_err.str().c_str());
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    cb->dispatch(e, &resolutions);
}

bool
XrlFinderV0p2Client::send_get_xrl_targets(
	const char*	dst_xrl_target_name,
	const GetXrlTargetsCB&	cb
)
{
    Xrl* x = ap_xrl_get_xrl_targets.get();

    if (!x) {
        x = new Xrl(dst_xrl_target_name, "finder/0.2/get_xrl_targets");
        ap_xrl_get_xrl_targets.reset(x);
    }

    x->set_target(dst_xrl_target_name);


    return _sender->send(*x, callback(&XrlFinderV0p2Client::unmarshall_get_xrl_targets, cb));
}


/* Unmarshall get_xrl_targets */
void
XrlFinderV0p2Client::unmarshall_get_xrl_targets(
	const XrlError&	e,
	XrlArgs*	a,
	GetXrlTargetsCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e, 0);
	return;
    } else if (a && a->size() != 1) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(1));
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    XrlAtomList target_names;
    try {
	a->get("target_names", target_names);
    } catch (const XrlArgs::BadArgs& bad_args_err) {
	UNUSED(bad_args_err);
	XLOG_ERROR("Error decoding the arguments: %s", bad_args_err.str().c_str());
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    cb->dispatch(e, &target_names);
}

bool
XrlFinderV0p2Client::send_get_xrls_registered_by(
	const char*	dst_xrl_target_name,
	const string&	target_name,
	const GetXrlsRegisteredByCB&	cb
)
{
    Xrl* x = ap_xrl_get_xrls_registered_by.get();

    if (!x) {
        x = new Xrl(dst_xrl_target_name, "finder/0.2/get_xrls_registered_by");
        x->args().add("target_name", target_name);
        ap_xrl_get_xrls_registered_by.reset(x);
    }

    x->set_target(dst_xrl_target_name);

    x->args().set_arg(0, target_name);

    return _sender->send(*x, callback(&XrlFinderV0p2Client::unmarshall_get_xrls_registered_by, cb));
}


/* Unmarshall get_xrls_registered_by */
void
XrlFinderV0p2Client::unmarshall_get_xrls_registered_by(
	const XrlError&	e,
	XrlArgs*	a,
	GetXrlsRegisteredByCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e, 0);
	return;
    } else if (a && a->size() != 1) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(1));
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    XrlAtomList xrls;
    try {
	a->get("xrls", xrls);
    } catch (const XrlArgs::BadArgs& bad_args_err) {
	UNUSED(bad_args_err);
	XLOG_ERROR("Error decoding the arguments: %s", bad_args_err.str().c_str());
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    cb->dispatch(e, &xrls);
}

bool
XrlFinderV0p2Client::send_get_ipv4_permitted_hosts(
	const char*	dst_xrl_target_name,
	const GetIpv4PermittedHostsCB&	cb
)
{
    Xrl* x = ap_xrl_get_ipv4_permitted_hosts.get();

    if (!x) {
        x = new Xrl(dst_xrl_target_name, "finder/0.2/get_ipv4_permitted_hosts");
        ap_xrl_get_ipv4_permitted_hosts.reset(x);
    }

    x->set_target(dst_xrl_target_name);


    return _sender->send(*x, callback(&XrlFinderV0p2Client::unmarshall_get_ipv4_permitted_hosts, cb));
}


/* Unmarshall get_ipv4_permitted_hosts */
void
XrlFinderV0p2Client::unmarshall_get_ipv4_permitted_hosts(
	const XrlError&	e,
	XrlArgs*	a,
	GetIpv4PermittedHostsCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e, 0);
	return;
    } else if (a && a->size() != 1) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(1));
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    XrlAtomList ipv4s;
    try {
	a->get("ipv4s", ipv4s);
    } catch (const XrlArgs::BadArgs& bad_args_err) {
	UNUSED(bad_args_err);
	XLOG_ERROR("Error decoding the arguments: %s", bad_args_err.str().c_str());
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    cb->dispatch(e, &ipv4s);
}

bool
XrlFinderV0p2Client::send_get_ipv4_permitted_nets(
	const char*	dst_xrl_target_name,
	const GetIpv4PermittedNetsCB&	cb
)
{
    Xrl* x = ap_xrl_get_ipv4_permitted_nets.get();

    if (!x) {
        x = new Xrl(dst_xrl_target_name, "finder/0.2/get_ipv4_permitted_nets");
        ap_xrl_get_ipv4_permitted_nets.reset(x);
    }

    x->set_target(dst_xrl_target_name);


    return _sender->send(*x, callback(&XrlFinderV0p2Client::unmarshall_get_ipv4_permitted_nets, cb));
}


/* Unmarshall get_ipv4_permitted_nets */
void
XrlFinderV0p2Client::unmarshall_get_ipv4_permitted_nets(
	const XrlError&	e,
	XrlArgs*	a,
	GetIpv4PermittedNetsCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e, 0);
	return;
    } else if (a && a->size() != 1) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(1));
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    XrlAtomList ipv4nets;
    try {
	a->get("ipv4nets", ipv4nets);
    } catch (const XrlArgs::BadArgs& bad_args_err) {
	UNUSED(bad_args_err);
	XLOG_ERROR("Error decoding the arguments: %s", bad_args_err.str().c_str());
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    cb->dispatch(e, &ipv4nets);
}

bool
XrlFinderV0p2Client::send_get_ipv6_permitted_hosts(
	const char*	dst_xrl_target_name,
	const GetIpv6PermittedHostsCB&	cb
)
{
    Xrl* x = ap_xrl_get_ipv6_permitted_hosts.get();

    if (!x) {
        x = new Xrl(dst_xrl_target_name, "finder/0.2/get_ipv6_permitted_hosts");
        ap_xrl_get_ipv6_permitted_hosts.reset(x);
    }

    x->set_target(dst_xrl_target_name);


    return _sender->send(*x, callback(&XrlFinderV0p2Client::unmarshall_get_ipv6_permitted_hosts, cb));
}


/* Unmarshall get_ipv6_permitted_hosts */
void
XrlFinderV0p2Client::unmarshall_get_ipv6_permitted_hosts(
	const XrlError&	e,
	XrlArgs*	a,
	GetIpv6PermittedHostsCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e, 0);
	return;
    } else if (a && a->size() != 1) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(1));
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    XrlAtomList ipv6s;
    try {
	a->get("ipv6s", ipv6s);
    } catch (const XrlArgs::BadArgs& bad_args_err) {
	UNUSED(bad_args_err);
	XLOG_ERROR("Error decoding the arguments: %s", bad_args_err.str().c_str());
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    cb->dispatch(e, &ipv6s);
}

bool
XrlFinderV0p2Client::send_get_ipv6_permitted_nets(
	const char*	dst_xrl_target_name,
	const GetIpv6PermittedNetsCB&	cb
)
{
    Xrl* x = ap_xrl_get_ipv6_permitted_nets.get();

    if (!x) {
        x = new Xrl(dst_xrl_target_name, "finder/0.2/get_ipv6_permitted_nets");
        ap_xrl_get_ipv6_permitted_nets.reset(x);
    }

    x->set_target(dst_xrl_target_name);


    return _sender->send(*x, callback(&XrlFinderV0p2Client::unmarshall_get_ipv6_permitted_nets, cb));
}


/* Unmarshall get_ipv6_permitted_nets */
void
XrlFinderV0p2Client::unmarshall_get_ipv6_permitted_nets(
	const XrlError&	e,
	XrlArgs*	a,
	GetIpv6PermittedNetsCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e, 0);
	return;
    } else if (a && a->size() != 1) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(1));
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    XrlAtomList ipv6nets;
    try {
	a->get("ipv6nets", ipv6nets);
    } catch (const XrlArgs::BadArgs& bad_args_err) {
	UNUSED(bad_args_err);
	XLOG_ERROR("Error decoding the arguments: %s", bad_args_err.str().c_str());
	cb->dispatch(XrlError::BAD_ARGS(), 0);
	return;
    }
    cb->dispatch(e, &ipv6nets);
}