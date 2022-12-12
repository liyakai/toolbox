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
#include "libxorp/exceptions.hh"

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "config_param.hh"

//
// XXX: MODIFY FOR YOUR TEST PROGRAM
//
static const char *program_name		= "test_config_param";
static const char *program_description	= "Test ConfigParam class";
static const char *program_version_id	= "0.1";
static const char *program_date		= "December 4, 2002";
static const char *program_copyright	= "See file LICENSE";
static const char *program_return_value	= "0 on success, 1 if test error, 2 if internal error";

static bool s_verbose = false;
bool verbose()			{ return s_verbose; }
void set_verbose(bool v)	{ s_verbose = v; }

static int s_failures = 0;
bool failures()			{ return s_failures; }
void incr_failures()		{ s_failures++; }

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


//
// The callback value argument when config_param_update_callback() is called.
// 
int config_param_update_callback_value = 0;

/**
 * The callback method that will be invoked when the value of ConfigParam
 * changes.
 */
void
config_param_update_callback(int new_value)
{
    config_param_update_callback_value = new_value;
    verbose_log("ConfigParam callback called with new value = %d\n",
		new_value);
}

/**
 * Test ConfigParam.
 */
void
test_config_param()
{
    //
    // Constructor of a parameter with an initial value.
    //
    ConfigParam<int> config_param1(1);
    
    //
    // Constructor of a parameter with an initial value and a callback.
    //
    ConfigParam<int> config_param2(2, callback(config_param_update_callback));
    
    //
    // Get the current value of the parameter.
    //
    verbose_assert(config_param1.get() == 1, "get()");
    verbose_assert(config_param2.get() == 2, "get()");

    //
    // Set the current value of the parameter.
    //
    config_param1.set(11);
    config_param2.set(22);
    verbose_assert(config_param1.get() == 11, "get()");
    verbose_assert(config_param2.get() == 22, "get()");
    verbose_assert(config_param_update_callback_value == 22,
		   "config parameter update callback value");
    
    //
    // Assignment operator
    //
    config_param1 = 111;
    config_param2 = 222;
    verbose_assert(config_param1.get() == 111, "get()");
    verbose_assert(config_param2.get() == 222, "get()");
    verbose_assert(config_param_update_callback_value == 222,
		   "config parameter update callback value");

    //
    // Increment and decrement operators (prefix)
    //
    config_param2 = 111;
    verbose_assert(++config_param2 == 112, "++ (prefix)");
    verbose_assert(config_param_update_callback_value == 112,
		   "config parameter update callback value");
    verbose_assert(--config_param2 == 111, "-- (prefix)");
    verbose_assert(config_param_update_callback_value == 111,
		   "config parameter update callback value");

    //
    // Increment and decrement operators (postfix)
    //
    config_param2 = 111;
    verbose_assert(config_param2++ == 111, "++ (postfix)");
    verbose_assert(config_param2.get() == 112, "++ (postfix)");
    verbose_assert(config_param_update_callback_value == 112,
		   "config parameter update callback value");
    verbose_assert(config_param2-- == 112, "-- (postfix)");
    verbose_assert(config_param2.get() == 111, "-- (postfix)");
    verbose_assert(config_param_update_callback_value == 111,
		   "config parameter update callback value");

    //
    // Methods incr() and decr()
    //
    config_param2 = 111;
    config_param2.incr();
    verbose_assert(config_param2.get() == 112, "incr()");
    verbose_assert(config_param_update_callback_value == 112,
		   "config parameter update callback value");
    config_param2.decr();
    verbose_assert(config_param2.get() == 111, "decr()");
    verbose_assert(config_param_update_callback_value == 111,
		   "config parameter update callback value");
    
    //
    // Get the initial value of the parameter.
    //
    verbose_assert(config_param1.get_initial_value() == 1,
		   "get_initial_value()");
    verbose_assert(config_param2.get_initial_value() == 2,
		   "get_initial_value()");
    
    //
    // Reset the current value of the parameter to its initial value.
    //
    config_param1.reset();
    config_param2.reset();
    verbose_assert(config_param1.get() == 1, "get()");
    verbose_assert(config_param2.get() == 2, "get()");
    verbose_assert(config_param_update_callback_value == 2,
		   "config parameter update callback value");
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
	test_config_param();
	
	ret_value = failures() ? 1 : 0;
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
