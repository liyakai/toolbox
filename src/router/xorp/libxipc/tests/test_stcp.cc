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



// #define DEBUG_LOGGING

#include "xrl_module.h"

#include "libxorp/xlog.h"
#include "libxorp/debug.h"

#include "xrl_error.hh"
#include "xrl_pf_stcp.hh"
#include "xrl_router.hh"

static bool g_trace = false;
#define tracef(args...) \
do { \
    if (g_trace) { printf(args) ; fflush(stdout); } \
} while (0)

// ----------------------------------------------------------------------------
// Hello message handlers (zero arguments to Xrl)

static bool hello_done = false;

static const XrlCmdError
hello_recv_handler(const XrlArgs& inputs,
		   XrlArgs*	  outputs)
{
    tracef("hello_recv_handler: inputs %s outputs %p\n",
	   inputs.str().c_str(), outputs);
    return XrlCmdError::OKAY();
}

static void
hello_reply_handler(const XrlError&	e,
		    XrlArgs*		response,
		    Xrl			request)
{
    if (e != XrlError::OKAY()) {
	fprintf(stderr, "hello failed: %s\n", e.str().c_str());
	exit(-1);
    }
    tracef("hello_reply_handler: request %s response %p\n",
	   request.str().c_str(), response);
    hello_done = true;
}

static void
test_hello(EventLoop& e, XrlPFSTCPSender &s)
{
    Xrl x("anywhere", "hello");

    debug_msg("test_hello\n");
    s.send(x, false, callback(hello_reply_handler, x));
    while (hello_done == false) {
	e.run();
    }
    hello_done = false;
}

// ----------------------------------------------------------------------------
// Hello message handlers (zero arguments to Xrl)

static bool int32_done = false;

static const XrlCmdError
int32_recv_handler(const XrlArgs& inputs,
		   XrlArgs*       outputs)
{
    tracef("int32_recv_handler: inputs %s outputs %p\n",
	   inputs.str().c_str(), outputs);
    if (outputs) {
	outputs->add_int32("an_int32", 123456);
    }
    return XrlCmdError::OKAY();
}

static void
int32_reply_handler(const XrlError& e,
		    XrlArgs*	    response,
		    Xrl		    request)

{
    if (e != XrlError::OKAY()) {
	fprintf(stderr, "get_int32 failed: %s\n", e.str().c_str());
	exit(-1);
    }
    tracef("int32_reply_handler: request %s response %p\n",
	   request.str().c_str(), response);
    tracef("int32 -> %s\n", response->str().c_str());
    int32_done = true;
}

static void
test_int32(EventLoop& e, XrlPFSTCPSender& s)
{
    Xrl x("anywhere", "get_int32");

    debug_msg("test_int32\n");
    s.send(x, false, callback(int32_reply_handler, x));

    while (int32_done == 0) {
	e.run();
    }
    int32_done = false;
}

static const char* NOISE = "Random arbitrary noise";

static const XrlCmdError
no_execute_recv_handler(const XrlArgs&  /* inputs*/,
			XrlArgs*    	/* outputs */,
			const char* noise)
{
    return XrlCmdError::COMMAND_FAILED(noise);
}

static void
no_execute_reply_handler(const XrlError& e,
			 XrlArgs*	 /* response */,
			 bool*		 done)
{
    if (e != XrlError::COMMAND_FAILED()) {
	fprintf(stderr, "no_execute_handler failed: %s\n", e.str().c_str());
	exit(-1);
    }
    if (e.note() != string(NOISE)) {
	fprintf(stderr, "no_execute_handler failed different reasons:"
		"expected:\t%s\ngot:\t\t%s\n", NOISE, e.note().c_str());
	exit(-1);
    }
    *done = true;
}

static void
test_xrlerror_note(EventLoop&e, XrlPFSTCPListener& l)
{
    Xrl x("anywhere", "no_execute");
    string t("test");

    XrlPFSTCPSender s(t, e, l.address());

    bool done = false;
    s.send(x, false, callback(no_execute_reply_handler, &done));

    while (done == false) {
	e.run();
    }
}

static bool
print_twirl()
{
    static const char t[] = { '\\', '|', '/', '-' };
    static const size_t nt = sizeof(t) / sizeof(t[0]);
    static size_t n = 0;
    static char erase = '\0';

    printf("%c%c", erase, t[n % nt]); fflush(stdout);
    n++;
    erase = '\b';
    return true;
}

static bool
toggle_flag(bool* flag)
{
    *flag = !(*flag);
    return true;
}

static void
run_test()
{
    static const TimeVal KEEPALIVE_TIME = TimeVal(2, 500000);
    EventLoop eventloop;

    XrlDispatcher cmd_dispatcher("tester");
    cmd_dispatcher.add_handler("hello", callback(hello_recv_handler));
    cmd_dispatcher.add_handler("get_int32", callback(int32_recv_handler));
    cmd_dispatcher.add_handler("no_execute",
			callback(no_execute_recv_handler, NOISE));

    XrlPFSTCPListener listener(eventloop, &cmd_dispatcher);
    string test("test");
    XrlPFSTCPSender s(test, eventloop, listener.address());
    s.set_keepalive_time(KEEPALIVE_TIME);

    tracef("listener address: %s\n", listener.address());

    XorpTimer dp = eventloop.new_periodic(TimeVal(0, 500000),
					  callback(&print_twirl));

    bool run_tests = true;
    XorpTimer rt = eventloop.new_periodic(5 * KEEPALIVE_TIME / 4,
					  callback(&toggle_flag, &run_tests));

    tracef("Testing XrlPFSTCP\n");
    bool stop = false;
    XorpTimer stop_timer = eventloop.set_flag_after(20 * KEEPALIVE_TIME,
						    &stop);

    while (stop == false) {
	assert(s.alive());

	if (run_tests) {
	    test_hello(eventloop, s);
	    test_int32(eventloop, s);
	} else {
	    eventloop.run();
	}
    }
    test_xrlerror_note(eventloop, listener);
}

static void
run_no_keepalive_test()
{
    static const uint32_t KEEPALIVE_MS = 2500;
    EventLoop eventloop;

    XrlDispatcher cmd_dispatcher("tester");
    cmd_dispatcher.add_handler("hello", callback(hello_recv_handler));
    cmd_dispatcher.add_handler("get_int32", callback(int32_recv_handler));
    cmd_dispatcher.add_handler("no_execute",
			callback(no_execute_recv_handler, NOISE));

    setenv("XORP_LISTENER_KEEPALIVE_TIMEOUT", "0", 1);
    setenv("XORP_SENDER_KEEPALIVE_TIME", "0", 1);

    XrlPFSTCPListener listener(eventloop, &cmd_dispatcher);
    string test("test");
    XrlPFSTCPSender s(test, eventloop, listener.address());

    tracef("listener address: %s\n", listener.address());

    XorpTimer dp = eventloop.new_periodic_ms(500, callback(&print_twirl));

    bool run_tests = true;
    XorpTimer rt = eventloop.new_periodic_ms(5 * KEEPALIVE_MS / 4,
					     callback(&toggle_flag, &run_tests));

    tracef("Testing XrlPFSTCP\n");
    bool stop = false;
    XorpTimer stop_timer = eventloop.set_flag_after_ms(20 * KEEPALIVE_MS,
						       &stop);

    while (stop == false) {
	assert(s.alive());

	if (run_tests) {
	    test_hello(eventloop, s);
	    test_int32(eventloop, s);
	} else {
	    eventloop.run();
	}
    }
    test_xrlerror_note(eventloop, listener);

    unsetenv("XORP_LISTENER_KEEPALIVE_TIMEOUT");
    unsetenv("XORP_SENDER_KEEPALIVE_TIME");
}

// ----------------------------------------------------------------------------
// Main

int main(int /* argc */, char *argv[])
{
    //
    // Initialize and start xlog
    //
    xlog_init(argv[0], NULL);
    xlog_set_verbose(XLOG_VERBOSE_LOW);		// Least verbose messages
    // XXX: verbosity of the error messages temporary increased
    xlog_level_set_verbose(XLOG_LEVEL_ERROR, XLOG_VERBOSE_HIGH);
    xlog_add_default_output();
    xlog_start();

#ifndef HOST_OS_WINDOWS
    // Set alarm
    alarm(60);
#endif
    run_test();

#ifndef HOST_OS_WINDOWS
    // Set alarm
    alarm(60);
#endif
    run_no_keepalive_test();

    //
    // Gracefully stop and exit xlog
    //
    xlog_stop();
    xlog_exit();

    return 0;
}
