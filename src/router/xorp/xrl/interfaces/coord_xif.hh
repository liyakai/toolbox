/*
 * obj/x86_64-unknown-linux-gnu/xrl/interfaces/coord_xif.hh
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

#ifndef __XRL_INTERFACES_COORD_XIF_HH__
#define __XRL_INTERFACES_COORD_XIF_HH__

#undef XORP_LIBRARY_NAME
#define XORP_LIBRARY_NAME "XifCoord"

#include "libxorp/xlog.h"
#include "libxorp/callback.hh"

#include "libxipc/xrl.hh"
#include "libxipc/xrl_error.hh"
#include "libxipc/xrl_sender.hh"

//#include <memory>

class XrlCoordV0p1Client {
public:
    XrlCoordV0p1Client(XrlSender* s) : _sender(s) {}
    virtual ~XrlCoordV0p1Client() {}

    typedef XorpCallback1<void, const XrlError&>::RefPtr CommandCB;
    /**
     *  Send Xrl intended to:
     *
     *  Command to be executed by the coordinating process.
     *
     *  @param dst_xrl_target_name the Xrl target name of the destination.
     *
     *  @param command to be sent to the coordinator.
     */
    bool send_command(
	const char*	dst_xrl_target_name,
	const string&	command,
	const CommandCB&	cb
    );

    typedef XorpCallback2<void, const XrlError&, const string*>::RefPtr StatusCB;
    /**
     *  Send Xrl intended to:
     *
     *  Status, show the state of this peer.
     *
     *  @param dst_xrl_target_name the Xrl target name of the destination.
     */
    bool send_status(
	const char*	dst_xrl_target_name,
	const string&	peer,
	const StatusCB&	cb
    );

    typedef XorpCallback2<void, const XrlError&, const bool*>::RefPtr PendingCB;
    /**
     *  Send Xrl intended to:
     *
     *  If there are still any outstanding commands pending will return true.
     *  Can be used to poll the coordinating process to verify that the
     *  previous command has completed.
     *
     *  @param dst_xrl_target_name the Xrl target name of the destination.
     */
    bool send_pending(
	const char*	dst_xrl_target_name,
	const PendingCB&	cb
    );

protected:
    XrlSender* _sender;

private:
    static void unmarshall_command(
	const XrlError&	e,
	XrlArgs*	a,
	CommandCB		cb
    );

    static void unmarshall_status(
	const XrlError&	e,
	XrlArgs*	a,
	StatusCB		cb
    );

    static void unmarshall_pending(
	const XrlError&	e,
	XrlArgs*	a,
	PendingCB		cb
    );

private:
    /* Declare cached Xrl pointers */
    unique_ptr<Xrl> ap_xrl_command;
    unique_ptr<Xrl> ap_xrl_status;
    unique_ptr<Xrl> ap_xrl_pending;
};

#endif /* __XRL_INTERFACES_COORD_XIF_HH__ */