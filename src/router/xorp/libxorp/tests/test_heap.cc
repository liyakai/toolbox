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

#include "heap.hh"


//
// XXX: MODIFY FOR YOUR TEST PROGRAM
//
static const char *program_name		= "test_heap";
static const char *program_description	= "Test Heap class";
static const char *program_version_id	= "0.1";
static const char *program_date		= "July 12, 2004";
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
    fprintf(stderr, "Return 0 on success, 1 if test error, 2 if internal error.\n");
}

class TestHeap : public Heap {
public:
    TestHeap() : Heap(0) {}

    void test_heap_push();
    void test_heap_push_same_value();
};

/**
 * Test Heap valid constructors.
 */
void
test_heap_valid_constructors()
{

}

/**
 * Test Heap invalid constructors.
 */
void
test_heap_invalid_constructors()
{

}

void
TestHeap::test_heap_push()
{
    struct heap_entry* he;
    int i1 = 1;
    int i2 = 2;
    int i3 = 3;

    push(TimeVal(0, i1), reinterpret_cast<HeapBase *>(&i1));
    push(TimeVal(0, i2), reinterpret_cast<HeapBase *>(&i2));
    push(TimeVal(0, i3), reinterpret_cast<HeapBase *>(&i3));

    verbose_assert(size() == 3, "heap size");
    he = top();
    verbose_assert(he->object == reinterpret_cast<HeapBase *>(&i1),
		   "heap top value 1");

    pop();
    verbose_assert(size() == 2, "heap size");
    he = top();
    verbose_assert(he->object == reinterpret_cast<HeapBase *>(&i2),
		   "heap top value 2");

    pop();
    verbose_assert(size() == 1, "heap size");
    he = top();
    verbose_assert(he->object == reinterpret_cast<HeapBase *>(&i3),
		   "heap top value 3");

    pop();
    verbose_assert(size() == 0, "heap size");
}

void
TestHeap::test_heap_push_same_value()
{
    struct heap_entry* he;
    int i1_1 = 1;
    int i1_2 = 1;

    push(TimeVal(0, i1_1), reinterpret_cast<HeapBase *>(&i1_1));
    push(TimeVal(0, i1_2), reinterpret_cast<HeapBase *>(&i1_2));

    verbose_assert(size() == 2, "heap size");
    he = top();
    verbose_assert(he->object == reinterpret_cast<HeapBase *>(&i1_1),
		   "heap top value 1_1");

    pop();
    verbose_assert(size() == 1, "heap size");
    he = top();
    verbose_assert(he->object == reinterpret_cast<HeapBase *>(&i1_2),
		   "Test heap top value 1_2");
    pop();
    verbose_assert(size() == 0, "heap size");
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
	TestHeap heap;
	test_heap_valid_constructors();
	test_heap_invalid_constructors();
	heap.test_heap_push();
	heap.test_heap_push_same_value();
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
