/*
 * obj/x86_64-unknown-linux-gnu/xrl/interfaces/common_xif.hh
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

#ifndef __XRL_INTERFACES_COMMON_XIF_HH__
#define __XRL_INTERFACES_COMMON_XIF_HH__

#undef XORP_LIBRARY_NAME
#define XORP_LIBRARY_NAME "XifCommon"

#include "libxorp/xlog.h"
#include "libxorp/callback.hh"

#include "libxipc/xrl.hh"
#include "libxipc/xrl_error.hh"
#include "libxipc/xrl_sender.hh"

//#include <memory>

class XrlCommonV0p1Client {
public:
    XrlCommonV0p1Client(XrlSender* s) : _sender(s) {}
    virtual ~XrlCommonV0p1Client() {}

    typedef XorpCallback2<void, const XrlError&, const string*>::RefPtr GetTargetNameCB;
    /**
     *  Send Xrl intended to:
     *
     *  Get name of Xrl Target
     *
     *  @param dst_xrl_target_name the Xrl target name of the destination.
     */
    bool send_get_target_name(
	const char*	dst_xrl_target_name,
	const GetTargetNameCB&	cb
    );

    typedef XorpCallback2<void, const XrlError&, const string*>::RefPtr GetVersionCB;
    /**
     *  Send Xrl intended to:
     *
     *  Get version string from Xrl Target
     *
     *  @param dst_xrl_target_name the Xrl target name of the destination.
     */
    bool send_get_version(
	const char*	dst_xrl_target_name,
	const GetVersionCB&	cb
    );

    typedef XorpCallback3<void, const XrlError&, const uint32_t*, const string*>::RefPtr GetStatusCB;
    /**
     *  Send Xrl intended to:
     *
     *  Get status of Xrl Target
     *
     *  @param dst_xrl_target_name the Xrl target name of the destination.
     */
    bool send_get_status(
	const char*	dst_xrl_target_name,
	const GetStatusCB&	cb
    );

    typedef XorpCallback1<void, const XrlError&>::RefPtr ShutdownCB;
    /**
     *  Send Xrl intended to:
     *
     *  Request clean shutdown of Xrl Target
     *
     *  @param dst_xrl_target_name the Xrl target name of the destination.
     */
    bool send_shutdown(
	const char*	dst_xrl_target_name,
	const ShutdownCB&	cb
    );

    typedef XorpCallback1<void, const XrlError&>::RefPtr StartupCB;
    /**
     *  Send Xrl intended to:
     *
     *  Request a startup of Xrl Target
     *
     *  @param dst_xrl_target_name the Xrl target name of the destination.
     */
    bool send_startup(
	const char*	dst_xrl_target_name,
	const StartupCB&	cb
    );

protected:
    XrlSender* _sender;

private:
    static void unmarshall_get_target_name(
	const XrlError&	e,
	XrlArgs*	a,
	GetTargetNameCB		cb
    );

    static void unmarshall_get_version(
	const XrlError&	e,
	XrlArgs*	a,
	GetVersionCB		cb
    );

    static void unmarshall_get_status(
	const XrlError&	e,
	XrlArgs*	a,
	GetStatusCB		cb
    );

    static void unmarshall_shutdown(
	const XrlError&	e,
	XrlArgs*	a,
	ShutdownCB		cb
    );

    static void unmarshall_startup(
	const XrlError&	e,
	XrlArgs*	a,
	StartupCB		cb
    );

private:
    /* Declare cached Xrl pointers */
    unique_ptr<Xrl> ap_xrl_get_target_name;
    unique_ptr<Xrl> ap_xrl_get_version;
    unique_ptr<Xrl> ap_xrl_get_status;
    unique_ptr<Xrl> ap_xrl_shutdown;
    unique_ptr<Xrl> ap_xrl_startup;
};

#endif /* __XRL_INTERFACES_COMMON_XIF_HH__ */