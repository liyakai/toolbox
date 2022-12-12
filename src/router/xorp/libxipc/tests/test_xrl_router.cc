// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
// vim:set sts=4 ts=8:

// Copyright (c) 2001-2009 XORP, Inc.
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



#include <stdlib.h>

#include "xrl_module.h"
#include "libxorp/xlog.h"
#include "xrl_router.hh"
#include "xrl_pf_stcp.hh"
#include "xrl_args.hh"
#include "finder_server.hh"

#ifndef UNUSED
#define UNUSED(x) (x) = (x)
#endif

// ----------------------------------------------------------------------------
// Xrl request callbacks

static const XrlCmdError
hello_world(const XrlArgs& /* request */,
	    XrlArgs*	   /* response */)
{
    return XrlCmdError::OKAY();
}

static const XrlCmdError
passback_integer(const XrlArgs&	/* request*/,
		 XrlArgs*	response)
{
    response->add_int32("the_number", 5);
    return XrlCmdError::OKAY();
}

// ----------------------------------------------------------------------------
// Xrl dispatch handlers and related

static void
exit_on_xrlerror(const XrlError& e, const char* file, int line)
{

    fprintf(stderr, "From %s: %d XrlError: %s\n", file, line, e.str().c_str());
    exit(-1);
}

static void
hello_world_complete(const XrlError&	e,
		     XrlArgs*		/* response */,
		     bool*		done)
{
    if (e != XrlError::OKAY())
	exit_on_xrlerror(e, __FILE__, __LINE__);

    *done = true;
}

static void
got_integer(const XrlError&	e,
	    XrlArgs*		response,
	    bool*		done)
{
    if (e != XrlError::OKAY())
	exit_on_xrlerror(e, __FILE__, __LINE__);

    try {
	int32_t the_int = response->get_int32("the_number");
	if (the_int != 5) {
	    fprintf(stderr, "Corrupt integer argument.");
	    exit(-1);
	}
    } catch (const XrlArgs::BadArgs& e) {
	printf("Error decoding the argument: %s\n", e.str().c_str());
    }

    // Eg we can iterate through the argument list
    for (size_t i = 0; i < response->size(); i++) {
	/* const XrlAtom &arg = */ response->item(i);
	/*	printf("%s = %d\n", arg.name().c_str(), arg.int32()); */
    }
    *done = true;
}

#include "finder.hh"
#include "finder_tcp_messenger.hh"
#include "finder_xrl_target.hh"
#include "permits.hh"
#include "sockutil.hh"

void
test_main()
{
    EventLoop		eventloop;

    // Normally the Finder runs as a separate process, but for this
    // test we create a Finder in process as we can't guarantee Finder
    // is already running.  Most XORP processes do not have to do this.
    
    FinderServer* finder = new FinderServer(eventloop,
					    FinderConstants::FINDER_DEFAULT_HOST(),
					    FinderConstants::FINDER_DEFAULT_PORT());
    
    // Create and configure "party_A"
    XrlRouter party_a(eventloop, "party_A", finder->addr(), finder->port());
    XrlPFSTCPListener	listener_a(eventloop);
    party_a.add_listener(&listener_a);

    // Add command that party_A knows about
    party_a.add_handler("hello_world", callback(hello_world));
    party_a.add_handler("passback_integer", callback(passback_integer));

    party_a.finalize();
    
    // Create and configure "party_B"
    XrlRouter party_b(eventloop, "party_B", finder->addr(), finder->port());
    XrlPFSTCPListener	listener_b(eventloop);
    party_b.add_listener(&listener_b);
    party_b.finalize();

    //
    // Pause for a second to allow parties to register Xrls and enable them
    // This is a bit slack but XrlRouter interface doesn't have enabled check
    // for time being and finder does.
    //
    bool finito = false;
    XorpTimer t = eventloop.set_flag_after_ms(1000, &finito);
    while (finito == false) {
	eventloop.run();
    }
    
    // "Party_B" send "hello world" to "Party_A"
    bool step1_done = false;
    Xrl x("party_A", "hello_world");
    party_b.send(x, callback(hello_world_complete, &step1_done));

    bool step2_done = false;
    Xrl y("party_A", "passback_integer");
    party_b.send(y, callback(got_integer, &step2_done));

    // Just run...
    finito = false;
    t = eventloop.set_flag_after_ms(5000, &finito);
    while (step1_done == false || step2_done == false) {
	eventloop.run();
	if (finito) {
	    fprintf(stderr, "Test timed out (%d %d)\n",
		    step1_done, step2_done);
	    exit(-1);
	}
    }

    if (finder)
	delete finder;

}

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

    try {
	test_main();
    } catch (...) {
	xorp_catch_standard_exceptions();
    }


    //
    // Gracefully stop and exit xlog
    //
    xlog_stop();
    xlog_exit();

    printf("Test ran okay.\n");
    return 0;
}
