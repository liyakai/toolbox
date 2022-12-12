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



#include "libxorp_module.h"

#include "libxorp/xorp.h"
#include "libxorp/xlog.h"
#include "libxorp/eventloop.hh"
#include "libxorp/exceptions.hh"

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "time_slice.hh"


//
// XXX: MODIFY FOR YOUR TEST PROGRAM
//
static const char *program_name		= "test_time_slice";
static const char *program_description	= "Test TimeSlice class";
static const char *program_version_id	= "0.1";
static const char *program_date		= "December 4, 2002";
static const char *program_copyright	= "See file LICENSE";
static const char *program_return_value	= "0 on success, 1 if test error, 2 if internal error";

static bool s_verbose = false;
bool verbose()			{ return s_verbose; }
void set_verbose(bool v)	{ s_verbose = v; }

static int s_failures = 0;
bool failures()			{ return (s_failures)? (true) : (false); }
void incr_failures()		{ s_failures++; }
void reset_failures()		{ s_failures = 0; }

#include "libxorp/xorp_tests.hh"


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

/**
 * Test TimeSlice valid constructors.
 */
void
test_time_slice_valid_constructors()
{
    //
    // Constructor for a given time limit and test frequency.
    //
    TimeSlice time_slice(10, 20);
    UNUSED(time_slice);
}

/**
 * Test TimeSlice invalid constructors.
 */
void
test_time_slice_invalid_constructors()
{
    //
    // Currently, there are no TimeSlice invalid constructors.
    //
}

/**
 * Test TimeSlice operators.
 */
void
test_time_slice_operators()
{
    //
    // Currently, there are no TimeSlice operators.
    //
}

/**
 * Supporting function: sleep for a given number of seconds.
 */
void
slow_function(unsigned int sleep_seconds)
{
    TimerList::system_sleep(TimeVal(sleep_seconds, 0));
}

/**
 * Supporting function: sleep for a given number of microseconds.
 */
void
fast_function(unsigned int sleep_microseconds)
{
    TimerList::system_sleep(TimeVal(0, sleep_microseconds));
}

/**
 * Test TimeSlice operations.
 */
void
test_time_slice_operations()
{
    unsigned int sec;
    unsigned int usec;
    unsigned int i, iter;
    
    //
    // Test single slow function.
    //
    verbose_log("TEST 'SINGLE SLOW_FUNCTION' BEGIN:\n");
    verbose_log("Begin time = %s\n", xlog_localtime2string());
    TimeSlice time_slice1(2000000, 1);	// 2s, test every 1 iter
    sec = 3;
    verbose_log("Running slow function for %d seconds...\n", sec);
    slow_function(sec);
    verbose_log("End time = %s\n", xlog_localtime2string());
    verbose_assert(time_slice1.is_expired(),
		   "is_expired() for a single slow function");
    verbose_log("TEST 'SINGLE SLOW_FUNCTION' END:\n\n");
    
    //
    // Test single fast function.
    //
    verbose_log("TEST 'SINGLE FAST_FUNCTION' BEGIN:\n");
    verbose_log("Begin time = %s\n", xlog_localtime2string());
    TimeSlice time_slice2(2000000, 1);	// 2s, test every 1 iter
    usec = 3;
    verbose_log("Running fast function for %d microseconds...\n", usec);
    fast_function(usec);
    verbose_log("End time = %s\n", xlog_localtime2string());
    verbose_assert(! time_slice2.is_expired(),
		   "is_expired() for a single fast function");
    verbose_log("TEST 'SINGLE FAST_FUNCTION' END:\n\n");
    
    //
    // Test fast function run multiple times.
    //
    verbose_log("TEST 'MULTI FAST_FUNCTION' BEGIN:\n");
    verbose_log("Begin time = %s\n", xlog_localtime2string());
    TimeSlice time_slice3(2000000, 10);		// 2s, test every 10 iter
    usec = 3;
    iter = 1000000;
    verbose_log("Running fast function %d times for %d microseconds each...\n",
		iter, usec);
    bool time_expired = false;
    for (i = 0; i < iter; i++) {
	if (time_slice3.is_expired()) {
	    time_expired = true;
	    break;
	}
	fast_function(usec);
    }
    verbose_log("End time = %s\n", xlog_localtime2string());
    verbose_assert(time_expired,
		   c_format("is_expired() for multiple iterations of a fast function (expired after %d iterations)", i));
    verbose_log("TEST 'MULTI FAST_FUNCTION' END:\n\n");
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
	EventLoop eventloop;

	test_time_slice_valid_constructors();
	test_time_slice_invalid_constructors();
	test_time_slice_operators();
	test_time_slice_operations();
	ret_value = failures() ? 1 : 0;
	UNUSED(eventloop);
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
