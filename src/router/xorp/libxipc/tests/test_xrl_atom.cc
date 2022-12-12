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



// test_xrl_atom: String Serialization Tests

#include "xrl_module.h"

#include "libxorp/xorp.h"
#include "libxorp/xlog.h"
#include "libxorp/random.h"

#include "xrl_atom.hh"
#include "xrl_atom_encoding.hh"


static bool g_trace = false;
#define tracef(args...) if (g_trace) printf(args)

static void
dump(const uint8_t* p, size_t p_bytes)
{
    for (size_t i = 0; i < p_bytes; i++) {
	tracef("%02x ", p[i]);
    }
    tracef("\n");
}

static bool
assignment_test(const XrlAtom& a)
{
    XrlAtom b(a);
    tracef("Assignment test... %s\n", (a == b) ? "yes" : "NO");
    return (a == b);
}

static bool
ascii_test(const XrlAtom& a)
{
    // Test 1 string serialization
    XrlAtom b(a.str().c_str());
    if (a != b) {
	tracef("%s %d %d\n", a.str().c_str(), a.type(), a.has_data());
	tracef("%s %d %d\n", b.str().c_str(), b.type(), b.has_data());
    }
    tracef("\tASCII Serialization... %s\n", (a == b) ? "yes" : "NO");
    return (a == b);
}

static bool
binary_test(const XrlAtom& a)
{
    XrlAtom b;
    vector<uint8_t> buffer(a.packed_bytes());
    if (a.pack(&buffer[0], buffer.size()) == a.packed_bytes()) {
	dump(&buffer[0], buffer.size());
	b.unpack(&buffer[0], buffer.size());
    }
    tracef("\tBinary Serialization I...");
    tracef("%s\n", (a == b) ? "yes" : "NO");
    return (a == b);
}

static bool
binary_test2(const XrlAtom& a)
{
    tracef("\tBinary Serialization II...");

    // buffer is too small shouldn't be able to pack
    XrlAtom b;
    vector<uint8_t> buffer(a.packed_bytes() - 1);
    if (a.pack(&buffer[0], buffer.size()) != 0) {
	tracef("NO\n");
	return false;
    } else {
	tracef("yes\n");
	return true;
    }
}

static bool
binary_test3(const XrlAtom& a)
{
    tracef("\tBinary Serialization III...");

    XrlAtom b;
    // Buffer is too big, shouldn't make any difference.
    vector<uint8_t> buffer(a.packed_bytes() + 1);
    if (a.pack(&buffer[0], buffer.size()) == a.packed_bytes()) {
	b.unpack(&buffer[0], buffer.size());
    }
    tracef("%s\n", (a == b) ? "yes" : "NO");
    return a == b;
}

static void
test_atom(const XrlAtom& a)
{

    // Print test name
    tracef("Testing %s %s\n",
	   (a.name().size()) ? "Named" : "Unnamed",
	   a.type_name());

    const char* name = a.name().c_str();
    const char* tname = a.type_name();

    if (assignment_test(a) == false) {
	fprintf(stderr, "Failed assignment test: %s:%s", name, tname);
	exit(-1);
    } else if (ascii_test(a) == false) {
	fprintf(stderr, "Failed ascii test: %s:%s", name, tname);
	exit(-1);
    } else if (binary_test(a) == false) {
	fprintf(stderr, "Failed binary test: %s:%s", name, tname);
	exit(-1);
    } else if (binary_test2(a) == false) {
	fprintf(stderr, "Failed binary test 2: %s:%s", name, tname);
	exit(-1);
    } else if (binary_test3(a) == false) {
	fprintf(stderr, "Failed binary test 3: %s:%s", name, tname);
	exit(-1);
    }
}

static void
test()
{
    // This is a simple switch to prevent compilation of this test
    // when new XrlAtom types are added. Please add your type below
    // *and* write a test for it.
    for (XrlAtomType t = xrlatom_start /* xrlatom_list */; t != xrlatom_no_type; ++t) {
	switch (t) {
	case xrlatom_no_type:
	    // No test
	    break;
	case xrlatom_boolean:
	    test_atom(XrlAtom("test_boolean_value", true));
	    break;
	case xrlatom_int32:
	    //	    test_atom(XrlAtom(int32_t(0x12345678)));
	    test_atom(XrlAtom("test_int32_value", int32_t(0x12345678)));
	    break;
	case xrlatom_uint32:
	    //	    test_atom(XrlAtom(uint32_t(0xfedcba98)));
	    test_atom(XrlAtom("test_uint32_value", uint32_t(0xfedcba98)));
	    break;
	case xrlatom_ipv4:
	    //	    test_atom(XrlAtom(IPv4("128.16.64.84")));
	    test_atom(XrlAtom("test_ipv4_value", IPv4("128.16.64.72")));
	    break;
	case xrlatom_ipv4net:
	    //	    test_atom(XrlAtom(IPv4Net("128.16.64.84/24")));
	    test_atom(XrlAtom("test_net4_value", IPv4Net("128.16.64.72/24")));
	    break;
	case xrlatom_ipv6:
	    //	    test_atom(XrlAtom(IPv6("fe80::2c0:4fff:fea1:1a71")));
	    test_atom(XrlAtom("test_ip6_value",
			      IPv6("fe80::2c0:4fff:fea1:1a71")));
	    break;
	case xrlatom_ipv6net:
	    //	    test_atom(XrlAtom(IPv6Net("fe80::2c0:4fff:fea1:1a71/96")));
	    test_atom(XrlAtom("A_net6_value",
			      IPv6Net("fe80::2c0:4fff:fea1:1a71/96")));
	    break;
	case xrlatom_mac:
	    //	    test_atom(XrlAtom(Mac("11:22:33:44:55:66")));
	    test_atom(XrlAtom("Some_Ethernet_Mac_address_you_have_there_sir",
			      Mac("11:22:33:44:55:66")));
	    break;
	case xrlatom_text:
	    {
		string t = "ABCabcDEFdef1234 !@#$%^&*(){}[]:;'<>";
#ifdef VERBOSE_STRING_TEST
		{
		    string encoded = xrlatom_encode_value(t);
		    cout << "Original: \"" << t << "\"" << endl;
		    cout << "Encoded:  \"" << encoded << "\"" << endl;
		    string decoded;
		    xrlatom_decode_value(encoded.c_str(), encoded.size(),
					 decoded);
		    cout << "Decoded:  \"" << decoded << "\"" << endl;
		}
#endif /* VERBOSE_STRING_TEST */
		test_atom(XrlAtom("A_string_object", t));
		test_atom(XrlAtom("Empty_string_of_mine", string("")));
	    }
	    break;
	case xrlatom_list:
	    {
		XrlAtomList xl;
		xl.append(XrlAtom ("string_1", string("abc")));
		xl.append(XrlAtom ("string_2", string("def")));
		xl.append(XrlAtom ("string_3", string("ghi")));
		tracef("XrlAtomList looks like: %s\n",
		       XrlAtom("foo", xl).str().c_str());
		test_atom(XrlAtom("An-XrlAtomList", xl));
		tracef("---");
		XrlAtomList yl;
		test_atom(XrlAtom("An-XrlAtomList", yl));
	    }
	    break;
	case xrlatom_binary:
	    {
		for (size_t sz = 1; sz < 10000; sz += 7) {
		    tracef("Binary data size = %u\n", XORP_UINT_CAST(sz));
		    vector<uint8_t> t(sz);
		    for (size_t i = 0; i < sz; i++) {
			t[i] = xorp_random();
		    }
		    assert(t.size() == sz);
		    XrlAtom a("binary_data", t);
		    assert(a.binary().size() == t.size());
		    test_atom(a);
		}
	    }
	    break;
	case xrlatom_int64:
	    test_atom(XrlAtom("test_int64_value",
		int64_t(-1234567890123456789LL)));
	    break;
	case xrlatom_uint64:
	    test_atom(XrlAtom("test_uint64_value",
		uint64_t(0xabadc0ffee123456ULL)));
	    break;
	case xrlatom_fp64:
	    test_atom(XrlAtom("test_fp64_value",
		fp64_t(0.087613017887164087613407)));
	    break;
	}
    }
}

int
main(int argc, const char *argv[])
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

    if (argc == 2 && 	argv[1][0] == '-' && argv[1][1] == 'v')
	g_trace = true;

    XorpUnexpectedHandler x(xorp_unexpected_handler);
    try {
	test();
    } catch (...) {
	xorp_catch_standard_exceptions();
    }

    //
    // Gracefully stop and exit xlog
    //
    xlog_stop();
    xlog_exit();

    return 0;
}
