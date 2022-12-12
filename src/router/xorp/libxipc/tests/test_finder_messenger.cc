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



#include "finder_module.h"

#include "libxorp/xorp.h"
#include "libxorp/xlog.h"
#include "libxorp/eventloop.hh"

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "libcomm/comm_api.h"

#include "finder_tcp_messenger.hh"
#include "permits.hh"
#include "sockutil.hh"


///////////////////////////////////////////////////////////////////////////////
//
// Constants
//

static const char *program_name         = "test_finder_messenger";
static const char *program_description  = "Test carrying and execution of Xrl"
					   "and Xrl responses.";
static const char *program_version_id   = "0.1";
static const char *program_date         = "January, 2003";
static const char *program_copyright    = "See file LICENSE";
static const char *program_return_value = "0 on success, 1 if test error, 2 if internal error";

///////////////////////////////////////////////////////////////////////////////
//
// Verbosity level control
//

static bool s_verbose = false;
bool verbose()                  { return s_verbose; }
void set_verbose(bool v)        { s_verbose = v; }

#define verbose_log(x...) _verbose_log(__FILE__,__LINE__, x)

#define _verbose_log(file, line, x...)					\
do {									\
    if (verbose()) {							\
	printf("From %s:%d: ", file, line);				\
	printf(x);							\
    }									\
} while(0)

///////////////////////////////////////////////////////////////////////////////
//
// Localized Test 
//

const XrlCmdError
hello_cmd(const XrlArgs& inputs, XrlArgs*)
{
    verbose_log("received hello_cmd( args = \"%s\" )\n",
		inputs.str().c_str());
    return XrlCmdError::OKAY();
}

static void
send_hello_complete(const XrlError& e, XrlArgs*, bool *success, bool* done)
{
    *done = true;
    *success = (e == XrlError::OKAY());
    verbose_log("Hello complete (success %d)\n", *success);
    return;
}

static void
send_hello(FinderMessengerBase* fm, bool* success, bool* done)
{
    bool send_okay;

    Xrl xrl("finder", "hello");
    
    send_okay = fm->send(xrl, callback(&send_hello_complete, success, done));
    assert(true == send_okay);
}

static void
add_commands(XrlCmdMap& cmds)
{
    cmds.add_handler("hello", callback(hello_cmd));
}

class DummyFinder : public FinderMessengerManager {
public:
    DummyFinder() : _messenger(0) {}

    virtual ~DummyFinder() {
	if (_messenger) {
	    // XXX: unhook the manager, because it may be gone
	    _messenger->unhook_manager();
	    delete _messenger;
	}

    }

    XrlCmdMap& commands() { return _commands; }

    FinderMessengerBase* messenger() const { return _messenger; }
    
protected:
    void messenger_active_event(FinderMessengerBase* m)
    {
	verbose_log("Messenger %p active event\n", m);
    }

    void messenger_inactive_event(FinderMessengerBase* m)
    {
	verbose_log("Messenger %p inactive event\n", m);
    }

    void messenger_stopped_event(FinderMessengerBase* m)
    {
	verbose_log("Messenger %p stopped event\n", m);
    }

    void messenger_birth_event(FinderMessengerBase* m)
    {
	verbose_log("Messenger %p birth event\n", m);
	assert(_messenger == 0);
	_messenger = m;
    }

    void messenger_death_event(FinderMessengerBase* m)
    {
	verbose_log("Messenger %p death event\n", m);
	if (m == _messenger) {
	    _messenger = 0;
	}
    }

    bool manages(const FinderMessengerBase* m) const
    {
	return m == _messenger;
    }
    
protected:
    FinderMessengerBase* _messenger;
    XrlCmdMap _commands;
};

// For the purposes of this test the client and finder are
// indestinguishable - they are both just holders of the object under
// test tcp based messenger
typedef DummyFinder DummyFinderClient;

static int
test_hellos(EventLoop& e,
	    FinderMessengerBase* src,
	    FinderMessengerBase* dst)
{
    bool timeout_flag = false;
    XorpTimer timeout = e.set_flag_after_ms(0, &timeout_flag);

    for (int i = 0; i < 1000; i++) {
	verbose_log("Testing send from %p to %p, iteration %d\n",
		    src, dst, i);

	bool hello_done = false;
	bool hello_success = false;
	XorpTimer hello_timer = e.new_oneoff_after_ms(5,
						      callback(send_hello,
							       src,
							       &hello_success,
							       &hello_done));

	timeout.reschedule_after_ms(500);
	while (false == hello_done) {
	    e.run();
	}

	if (timeout_flag) {
	    verbose_log("Timed out waiting for xrl response.\n");
	    return 1;
	}

	if (false == hello_success) {
	    verbose_log("Xrl dispatched, but error reported.\n");
	    return 1;
	}
    }
    return 0;
}

static int
test_main(void)
{
    EventLoop e;

    IPv4 ipc_addr = get_preferred_ipv4_addr();

    DummyFinder finder;
    FinderTcpListener listener(e,
				 finder, finder.commands(),
				 ipc_addr, 12222);

    add_permitted_host(ipc_addr);

    DummyFinderClient finder_client;
    FinderTcpAutoConnector connector(e,
				       finder_client, finder_client.commands(),
				       ipc_addr, 12222);
    
    add_commands(finder.commands());
    add_commands(finder_client.commands());

    bool timeout_flag = false;
    XorpTimer timeout;

    timeout = e.set_flag_after_ms(5000, &timeout_flag);
    
    while ((finder_client.messenger() == 0 || finder.messenger() == 0) && 
	   timeout_flag == false) {
	e.run();
    }

    if (timeout_flag) {
	verbose_log(
	    "Timed out: finder messenger (%p), client messenger (%p)\n",
	    finder.messenger(), finder_client.messenger());
	return 1;
    }

    timeout.unschedule();

    /* Get client to say hello to finder */

    if (test_hellos(e, finder_client.messenger(), finder.messenger())) {
	return 1;
    }

    /* Get finder to say hello to client */
    if (test_hellos(e, finder.messenger(), finder_client.messenger())) {
	return 1;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// Standard test program rubble.
//

/**
 * Print program info to output stream.
 * 
 * @param stream the output stream the print the program info to.
 */
static void
print_program_info(FILE *stream)
{
    fprintf(stream, "Name:          %s\n", program_name);
    fprintf(stream, "Description:   %s\n", program_description);
    fprintf(stream, "Version:       %s\n", program_version_id);
    fprintf(stream, "Date:          %s\n", program_date);
    fprintf(stream, "Copyright:     %s\n", program_copyright);
    fprintf(stream, "Return:        %s\n", program_return_value);
}

/**
 * Print program usage information to the stderr.
 * 
 * @param progname the name of the program.
 */
static void
usage(const char* progname)
{
    print_program_info(stderr);
    fprintf(stderr, "usage: %s [-v] [-h]\n", progname);
    fprintf(stderr, "       -h          : usage (this message)\n");
    fprintf(stderr, "       -v          : verbose output\n");
}

int
main(int argc, char * const argv[])
{
   int ret_value = 0;
    
    //
    // Initialize and start xlog
    //
    xlog_init(argv[0], NULL);
    xlog_set_verbose(XLOG_VERBOSE_LOW);         // Least verbose messages
    // XXX: verbosity of the error messages temporary increased
    xlog_level_set_verbose(XLOG_LEVEL_ERROR, XLOG_VERBOSE_HIGH);
    xlog_add_default_output();
    xlog_start();
    
    int ch;
    while ((ch = getopt(argc, argv, "hv")) != -1) {
        switch (ch) {
        case 'v':
            set_verbose(true);
            break;
        case 'h':
        case '?':
        default:
            usage(argv[0]);
            xlog_stop();
            xlog_exit();
            if (ch == 'h')
                return (0);
            else
                return (1);
        }
    }
    argc -= optind;
    argv += optind;
    
    XorpUnexpectedHandler x(xorp_unexpected_handler);
    try {
	ret_value = test_main();
    } catch (...) {
        // Internal error
        xorp_print_standard_exceptions();
        ret_value = 2;
    }
   
    //
    // Gracefully stop and exit xlog
    //
    xlog_stop();
    xlog_exit();
    
    return (ret_value);
}
