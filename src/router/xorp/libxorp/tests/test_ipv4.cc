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

#include "ipv4.hh"

//
// XXX: MODIFY FOR YOUR TEST PROGRAM
//
static const char *program_name		= "test_ipv4";
static const char *program_description	= "Test IPv4 address class";
static const char *program_version_id	= "0.1";
static const char *program_date		= "December 2, 2002";
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

/**
 * Test IPv4 valid constructors.
 */
void
test_ipv4_valid_constructors()
{
    // Test values for IPv4 address: "12.34.56.78"
    const char *addr_string = "12.34.56.78";
    uint32_t ui = htonl((12 << 24) | (34 << 16) | (56 << 8) | 78);
    struct in_addr in_addr;
    in_addr.s_addr = ui;
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
    sin.sin_len = sizeof(sin);
#endif
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ui;
    struct sockaddr *sap = (struct sockaddr *)&sin;
    struct sockaddr_storage *ssp = (struct sockaddr_storage *)&sin;

    //
    // Default constructor.
    //
    IPv4 ip1;
    verbose_match(ip1.str(), "0.0.0.0");

    //
    // Constructor from a string.
    //
    IPv4 ip2(addr_string);
    verbose_match(ip2.str(), addr_string);

    //
    // Constructor from another IPv4 address.
    //
    IPv4 ip3(ip2);
    verbose_match(ip3.str(), addr_string);

    //
    // Constructor from an integer value.
    //
    IPv4 ip4(ui);
    verbose_match(ip4.str(), addr_string);

    //
    // Constructor from a (uint8_t *) memory pointer.
    //
    IPv4 ip5((uint8_t *)&ui);
    verbose_match(ip5.str(), addr_string);

    //
    // Constructor from in_addr structure.
    //
    IPv4 ip6(in_addr);
    verbose_match(ip6.str(), addr_string);

    //
    // Constructor from sockaddr structure.
    //
    IPv4 ip7(*sap);
    verbose_match(ip7.str(), addr_string);

    //
    // Constructor from sockaddr_storage structure.
    //
    IPv4 ip8(*ssp);
    verbose_match(ip8.str(), addr_string);

    //
    // Constructor from sockaddr_in structure.
    //
    IPv4 ip9(sin);
    verbose_match(ip9.str(), addr_string);
}

/**
 * Test IPv4 invalid constructors.
 */
void
test_ipv4_invalid_constructors()
{
    // Test values for IPv4 address: "12.34.56.78"
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
    sin.sin_len = sizeof(sin);
#endif
    sin.sin_family = AF_UNSPEC;		// Note: invalid IP address family
    sin.sin_addr.s_addr = htonl((12 << 24) | (34 << 16) | (56 << 8) | 78);
    struct sockaddr *sap = (struct sockaddr *)&sin;
    struct sockaddr_storage *ssp = (struct sockaddr_storage *)&sin;

    //
    // Constructor from an invalid address string.
    //
    try {
	// Invalid address string: note the typo -- lack of a "dot" after "12"
	IPv4 ip("1234.56.78");
	verbose_log("Cannot catch invalid IP address \"1234.56.78\" : FAIL\n");
	incr_failures();
	UNUSED(ip);
    } catch (const InvalidString& e) {
	// The problem was caught
	verbose_log("%s : OK\n", e.str().c_str());
    }

    //
    // Constructor from an invalid sockaddr structure.
    //
    try {
	IPv4 ip(*sap);
	verbose_log("Cannot catch invalid IP address family AF_UNSPEC : FAIL\n");
	incr_failures();
	UNUSED(ip);
    } catch (const InvalidFamily& e) {
	// The problem was caught
	verbose_log("%s : OK\n", e.str().c_str());
    }

    //
    // Constructor from an invalid sockaddr_storage structure.
    //
    try {
	IPv4 ip(*ssp);
	verbose_log("Cannot catch invalid IP address family AF_UNSPEC : FAIL\n");
	incr_failures();
	UNUSED(ip);
    } catch (const InvalidFamily& e) {
	// The problem was caught
	verbose_log("%s : OK\n", e.str().c_str());
    }

    //
    // Constructor from an invalid sockaddr_in structure.
    //
    try {
	IPv4 ip(sin);
	verbose_log("Cannot catch invalid IP address family AF_UNSPEC : FAIL\n");
	incr_failures();
	UNUSED(ip);
    } catch (const InvalidFamily& e) {
	// The problem was caught
	verbose_log("%s : OK\n", e.str().c_str());
    }
}

/**
 * Test IPv4 valid copy in/out methods.
 */
void
test_ipv4_valid_copy_in_out()
{
    // Test values for IPv4 address: "12.34.56.78"
    const char *addr_string4 = "12.34.56.78";
    uint32_t ui = htonl((12 << 24) | (34 << 16) | (56 << 8) | 78);
    struct in_addr in_addr;
    in_addr.s_addr = ui;
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
    sin.sin_len = sizeof(sin);
#endif
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ui;

    struct sockaddr *sap;
    struct sockaddr_storage *ssp;


    //
    // Copy the IPv4 raw address to specified memory location.
    //
    IPv4 ip1(addr_string4);
    uint8_t ip1_uint8[4];
    verbose_assert(ip1.copy_out(&ip1_uint8[0]) == 4,
		   "copy_out(uint8_t *) for IPv4 address");
    verbose_assert(memcmp(&ui, &ip1_uint8[0], 4) == 0,
		   "compare copy_out(uint8_t *) for IPv4 address");

    //
    // Copy the IPv4 raw address to an in_addr structure.
    //
    IPv4 ip3(addr_string4);
    struct in_addr ip3_in_addr;
    verbose_assert(ip3.copy_out(ip3_in_addr) == 4,
		   "copy_out(in_addr&) for IPv4 address");
    verbose_assert(memcmp(&in_addr, &ip3_in_addr, 4) == 0,
		   "compare copy_out(in_addr&) for IPv4 address");

    //
    // Copy the IPv4 raw address to a sockaddr structure.
    //
    IPv4 ip5(addr_string4);
    struct sockaddr_in ip5_sockaddr_in;
    sap = (struct sockaddr *)&ip5_sockaddr_in;
    verbose_assert(ip5.copy_out(*sap) == 4,
		   "copy_out(sockaddr&) for IPv4 address");
    verbose_assert(memcmp(&sin, &ip5_sockaddr_in, sizeof(sin)) == 0,
		   "compare copy_out(sockaddr&) for IPv4 address");

    //
    // Copy the IPv4 raw address to a sockaddr_storage structure.
    //
    IPv4 ip5_2(addr_string4);
    struct sockaddr_in ip5_2_sockaddr_in;
    ssp = (struct sockaddr_storage *)&ip5_2_sockaddr_in;
    verbose_assert(ip5_2.copy_out(*ssp) == 4,
		   "copy_out(sockaddr_storage&) for IPv4 address");
    verbose_assert(memcmp(&sin, &ip5_2_sockaddr_in, sizeof(sin)) == 0,
		   "compare copy_out(sockaddr_storage&) for IPv4 address");

    //
    // Copy the IPv4 raw address to a sockaddr_in structure.
    //
    IPv4 ip7(addr_string4);
    struct sockaddr_in ip7_sockaddr_in;
    verbose_assert(ip7.copy_out(ip7_sockaddr_in) == 4,
		   "copy_out(sockaddr_in&) for IPv4 address");
    verbose_assert(memcmp(&sin, &ip7_sockaddr_in, sizeof(sin)) == 0,
		   "compare copy_out(sockaddr_in&) for IPv4 address");

    //
    // Copy a raw address into IPv4 structure.
    //
    IPv4 ip11;
    verbose_assert(ip11.copy_in((uint8_t *)&ui) == 4,
		   "copy_in(uint8_t *) for IPv4 address");
    verbose_match(ip11.str(), addr_string4);

    //
    // Copy a raw IPv4 address from a in_addr structure into IPv4 structure.
    //
    IPv4 ip13;
    verbose_assert(ip13.copy_in(in_addr) == 4,
		   "copy_in(in_addr&) for IPv4 address");
    verbose_match(ip13.str(), addr_string4);

    //
    // Copy a raw address from a sockaddr structure into IPv4 structure.
    //
    IPv4 ip15;
    sap = (struct sockaddr *)&sin;
    verbose_assert(ip15.copy_in(*sap) == 4,
		   "copy_in(sockaddr&) for IPv4 address");
    verbose_match(ip15.str(), addr_string4);

    //
    // Copy a raw address from a sockaddr_storage structure into IPv4
    // structure.
    //
    IPv4 ip15_2;
    ssp = (struct sockaddr_storage *)&sin;
    verbose_assert(ip15_2.copy_in(*ssp) == 4,
		   "copy_in(sockaddr_storage&) for IPv4 address");
    verbose_match(ip15_2.str(), addr_string4);

    //
    // Copy a raw address from a sockaddr_in structure into IPv4 structure.
    //
    IPv4 ip17;
    verbose_assert(ip17.copy_in(sin) == 4,
		   "copy_in(sockaddr_in&) for IPv4 address");
    verbose_match(ip17.str(), addr_string4);
}

/**
 * Test IPv4 invalid copy in/out methods.
 */
void
test_ipv4_invalid_copy_in_out()
{
    // Test values for IPv4 address: "12.34.56.78"
    // const char *addr_string4 = "12.34.56.78";
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
    sin.sin_len = sizeof(sin);
#endif
    sin.sin_family = AF_UNSPEC;		// Note: invalid IP address family
    sin.sin_addr.s_addr = htonl((12 << 24) | (34 << 16) | (56 << 8) | 78);

    struct sockaddr *sap;
    struct sockaddr_storage *ssp;

    //
    // Copy-in from a sockaddr structure for invalid address family.
    //
    try {
	IPv4 ip;
	sap = (struct sockaddr *)&sin;
	ip.copy_in(*sap);
	verbose_log("Cannot catch invalid IP address family AF_UNSPEC : FAIL\n");
	incr_failures();
    } catch (const InvalidFamily& e) {
	// The problem was caught
	verbose_log("%s : OK\n", e.str().c_str());
    }

    //
    // Copy-in from a sockaddr_storage structure for invalid address family.
    //
    try {
	IPv4 ip;
	ssp = (struct sockaddr_storage *)&sin;
	ip.copy_in(*ssp);
	verbose_log("Cannot catch invalid IP address family AF_UNSPEC : FAIL\n");
	incr_failures();
    } catch (const InvalidFamily& e) {
	// The problem was caught
	verbose_log("%s : OK\n", e.str().c_str());
    }

    //
    // Copy-in from a sockaddr_in structure for invalid address family.
    //
    try {
	IPv4 ip;
	ip.copy_in(sin);
	verbose_log("Cannot catch invalid IP address family AF_UNSPEC : FAIL\n");
	incr_failures();
    } catch (const InvalidFamily& e) {
	// The problem was caught
	verbose_log("%s : OK\n", e.str().c_str());
    }
}

/**
 * Test IPv4 operators.
 */
void
test_ipv4_operators()
{
    IPv4 ip_a("0.255.0.255");
    IPv4 ip_b("255.0.255.255");
    IPv4 ip_not_a("255.0.255.0");
    IPv4 ip_a_or_b("255.255.255.255");
    IPv4 ip_a_and_b("0.0.0.255");
    IPv4 ip_a_xor_b("255.255.255.0");

    //
    // Equality Operator
    //
    verbose_assert(ip_a == ip_a, "operator==");
    verbose_assert(!(ip_a == ip_b), "operator==");

    //
    // Not-Equal Operator
    //
    verbose_assert(!(ip_a != ip_a), "operator!=");
    verbose_assert(ip_a != ip_b, "operator!=");

    //
    // Less-Than Operator
    //
    verbose_assert(ip_a < ip_b, "operator<");

    //
    // Bitwise-Negation Operator
    //
    verbose_assert(~ip_a == ip_not_a, "operator~");

    //
    // OR Operator
    //
    verbose_assert((ip_a | ip_b) == ip_a_or_b, "operator|");

    //
    // AND Operator
    //
    verbose_assert((ip_a & ip_b) == ip_a_and_b, "operator&");

    //
    // XOR Operator
    //
    verbose_assert((ip_a ^ ip_b) == ip_a_xor_b, "operator^");

    //
    // Operator <<
    //
    verbose_assert(IPv4("0.255.0.255") << 16 == IPv4("0.255.0.0"),
		   "operator<<");
    verbose_assert(IPv4("0.255.0.0") << 1 == IPv4("1.254.0.0"),
		   "operator<<");

    //
    // Operator >>
    //
    verbose_assert(IPv4("0.255.0.255") >> 16 == IPv4("0.0.0.255"),
		   "operator>>");
    verbose_assert(IPv4("0.0.0.255") >> 1 == IPv4("0.0.0.127"),
		   "operator>>");

    //
    // Decrement Operator
    //
    verbose_assert(--IPv4("0.255.0.255") == IPv4("0.255.0.254"),
		   "operator--()");
    verbose_assert(--IPv4("0.0.0.0") == IPv4("255.255.255.255"),
		   "operator--()");

    //
    // Increment Operator
    //
    verbose_assert(++IPv4("0.255.0.254") == IPv4("0.255.0.255"),
		   "operator++()");
    verbose_assert(++IPv4("255.255.255.255") == IPv4("0.0.0.0"),
		   "operator++()");
}

/**
 * Test IPv4 address type.
 */
void
test_ipv4_address_type()
{
    IPv4 ip4_zero("0.0.0.0");			// Zero, not unicast
    IPv4 ip4_unicast1("0.0.0.1");		// Unicast
    IPv4 ip4_unicast2("1.2.3.4");		// Unicast
    IPv4 ip4_unicast3("223.255.255.255");	// Unicast
    IPv4 ip4_class_a1("0.0.0.0");		// Class A
    IPv4 ip4_class_a2("12.34.56.78");		// Class A
    IPv4 ip4_class_a3("127.255.255.255");	// Class A
    IPv4 ip4_class_b1("128.0.0.0");		// Class B
    IPv4 ip4_class_b2("128.2.3.4");		// Class B
    IPv4 ip4_class_b3("191.255.255.255");	// Class B
    IPv4 ip4_class_c1("192.0.0.0");		// Class C
    IPv4 ip4_class_c2("192.2.3.4");		// Class C
    IPv4 ip4_class_c3("223.255.255.255");	// Class C
    IPv4 ip4_multicast1("224.0.0.0");		// Multicast
    IPv4 ip4_multicast2("224.2.3.4");		// Multicast
    IPv4 ip4_multicast3("239.255.255.255");	// Multicast
    IPv4 ip4_experimental1("240.0.0.0");	// Experimental
    IPv4 ip4_experimental2("240.2.3.4");	// Experimental
    IPv4 ip4_experimental3("255.255.255.255");	// Experimental
    //
    IPv4 ip4_multicast_linklocal1("224.0.0.1");	// Link-local multicast
    IPv4 ip4_loopback1("127.0.0.1");		// Loopback
    IPv4 ip4_loopback2("127.255.255.255");	// Loopback

    //
    // Test if an address is numerically zero.
    //
    verbose_assert(ip4_zero.is_zero() == true, "is_zero()");
    verbose_assert(ip4_unicast1.is_zero() == false, "is_zero()");
    verbose_assert(ip4_unicast2.is_zero() == false, "is_zero()");
    verbose_assert(ip4_unicast3.is_zero() == false, "is_zero()");
    verbose_assert(ip4_class_a1.is_zero() == true, "is_zero()");
    verbose_assert(ip4_class_a2.is_zero() == false, "is_zero()");
    verbose_assert(ip4_class_a3.is_zero() == false, "is_zero()");
    verbose_assert(ip4_class_b1.is_zero() == false, "is_zero()");
    verbose_assert(ip4_class_b2.is_zero() == false, "is_zero()");
    verbose_assert(ip4_class_b3.is_zero() == false, "is_zero()");
    verbose_assert(ip4_class_c1.is_zero() == false, "is_zero()");
    verbose_assert(ip4_class_c2.is_zero() == false, "is_zero()");
    verbose_assert(ip4_class_c3.is_zero() == false, "is_zero()");
    verbose_assert(ip4_multicast1.is_zero() == false, "is_zero()");
    verbose_assert(ip4_multicast2.is_zero() == false, "is_zero()");
    verbose_assert(ip4_multicast3.is_zero() == false, "is_zero()");
    verbose_assert(ip4_experimental1.is_zero() == false, "is_zero()");
    verbose_assert(ip4_experimental2.is_zero() == false, "is_zero()");
    verbose_assert(ip4_experimental3.is_zero() == false, "is_zero()");

    //
    // Test if an address is a valid unicast address.
    //
    verbose_assert(ip4_zero.is_unicast() == false, "is_unicast()");
    verbose_assert(ip4_unicast1.is_unicast() == true, "is_unicast()");
    verbose_assert(ip4_unicast2.is_unicast() == true, "is_unicast()");
    verbose_assert(ip4_unicast3.is_unicast() == true, "is_unicast()");
    verbose_assert(ip4_class_a1.is_unicast() == false, "is_unicast()");
    verbose_assert(ip4_class_a2.is_unicast() == true, "is_unicast()");
    verbose_assert(ip4_class_a3.is_unicast() == true, "is_unicast()");
    verbose_assert(ip4_class_b1.is_unicast() == true, "is_unicast()");
    verbose_assert(ip4_class_b2.is_unicast() == true, "is_unicast()");
    verbose_assert(ip4_class_b3.is_unicast() == true, "is_unicast()");
    verbose_assert(ip4_class_c1.is_unicast() == true, "is_unicast()");
    verbose_assert(ip4_class_c2.is_unicast() == true, "is_unicast()");
    verbose_assert(ip4_class_c3.is_unicast() == true, "is_unicast()");
    verbose_assert(ip4_multicast1.is_unicast() == false, "is_unicast()");
    verbose_assert(ip4_multicast2.is_unicast() == false, "is_unicast()");
    verbose_assert(ip4_multicast3.is_unicast() == false, "is_unicast()");
    verbose_assert(ip4_experimental1.is_unicast() == false, "is_unicast()");
    verbose_assert(ip4_experimental2.is_unicast() == false, "is_unicast()");
    verbose_assert(ip4_experimental3.is_unicast() == false, "is_unicast()");

    //
    // Test if an address is a valid multicast address.
    //
    verbose_assert(ip4_zero.is_multicast() == false, "is_multicast()");
    verbose_assert(ip4_unicast1.is_multicast() == false, "is_multicast()");
    verbose_assert(ip4_unicast2.is_multicast() == false, "is_multicast()");
    verbose_assert(ip4_unicast3.is_multicast() == false, "is_multicast()");
    verbose_assert(ip4_class_a1.is_multicast() == false, "is_multicast()");
    verbose_assert(ip4_class_a2.is_multicast() == false, "is_multicast()");
    verbose_assert(ip4_class_a3.is_multicast() == false, "is_multicast()");
    verbose_assert(ip4_class_b1.is_multicast() == false, "is_multicast()");
    verbose_assert(ip4_class_b2.is_multicast() == false, "is_multicast()");
    verbose_assert(ip4_class_b3.is_multicast() == false, "is_multicast()");
    verbose_assert(ip4_class_c1.is_multicast() == false, "is_multicast()");
    verbose_assert(ip4_class_c2.is_multicast() == false, "is_multicast()");
    verbose_assert(ip4_class_c3.is_multicast() == false, "is_multicast()");
    verbose_assert(ip4_multicast1.is_multicast() == true, "is_multicast()");
    verbose_assert(ip4_multicast2.is_multicast() == true, "is_multicast()");
    verbose_assert(ip4_multicast3.is_multicast() == true, "is_multicast()");
    verbose_assert(ip4_experimental1.is_multicast() == false,
		   "is_multicast()");
    verbose_assert(ip4_experimental2.is_multicast() == false,
		   "is_multicast()");
    verbose_assert(ip4_experimental3.is_multicast() == false,
		   "is_multicast()");

    //
    // Test if an address is a valid Class A address.
    //
    verbose_assert(ip4_zero.is_class_a() == true, "is_class_a()");
    verbose_assert(ip4_unicast1.is_class_a() == true, "is_class_a()");
    verbose_assert(ip4_unicast2.is_class_a() == true, "is_class_a()");
    verbose_assert(ip4_unicast3.is_class_a() == false, "is_class_a()");
    verbose_assert(ip4_class_a1.is_class_a() == true, "is_class_a()");
    verbose_assert(ip4_class_a2.is_class_a() == true, "is_class_a()");
    verbose_assert(ip4_class_a3.is_class_a() == true, "is_class_a()");
    verbose_assert(ip4_class_b1.is_class_a() == false, "is_class_a()");
    verbose_assert(ip4_class_b2.is_class_a() == false, "is_class_a()");
    verbose_assert(ip4_class_b3.is_class_a() == false, "is_class_a()");
    verbose_assert(ip4_class_c1.is_class_a() == false, "is_class_a()");
    verbose_assert(ip4_class_c2.is_class_a() == false, "is_class_a()");
    verbose_assert(ip4_class_c3.is_class_a() == false, "is_class_a()");
    verbose_assert(ip4_multicast1.is_class_a() == false, "is_class_a()");
    verbose_assert(ip4_multicast2.is_class_a() == false, "is_class_a()");
    verbose_assert(ip4_multicast3.is_class_a() == false, "is_class_a()");
    verbose_assert(ip4_experimental1.is_class_a() == false, "is_class_a()");
    verbose_assert(ip4_experimental2.is_class_a() == false, "is_class_a()");
    verbose_assert(ip4_experimental3.is_class_a() == false, "is_class_a()");

    //
    // Test if an address is a valid Class B address.
    //
    verbose_assert(ip4_zero.is_class_b() == false, "is_class_b()");
    verbose_assert(ip4_unicast1.is_class_b() == false, "is_class_b()");
    verbose_assert(ip4_unicast2.is_class_b() == false, "is_class_b()");
    verbose_assert(ip4_unicast3.is_class_b() == false, "is_class_b()");
    verbose_assert(ip4_class_a1.is_class_b() == false, "is_class_b()");
    verbose_assert(ip4_class_a2.is_class_b() == false, "is_class_b()");
    verbose_assert(ip4_class_a3.is_class_b() == false, "is_class_b()");
    verbose_assert(ip4_class_b1.is_class_b() == true, "is_class_b()");
    verbose_assert(ip4_class_b2.is_class_b() == true, "is_class_b()");
    verbose_assert(ip4_class_b3.is_class_b() == true, "is_class_b()");
    verbose_assert(ip4_class_c1.is_class_b() == false, "is_class_b()");
    verbose_assert(ip4_class_c2.is_class_b() == false, "is_class_b()");
    verbose_assert(ip4_class_c3.is_class_b() == false, "is_class_b()");
    verbose_assert(ip4_multicast1.is_class_b() == false, "is_class_b()");
    verbose_assert(ip4_multicast2.is_class_b() == false, "is_class_b()");
    verbose_assert(ip4_multicast3.is_class_b() == false, "is_class_b()");
    verbose_assert(ip4_experimental1.is_class_b() == false, "is_class_b()");
    verbose_assert(ip4_experimental2.is_class_b() == false, "is_class_b()");
    verbose_assert(ip4_experimental3.is_class_b() == false, "is_class_b()");

    //
    // Test if an address is a valid Class C address.
    //
    verbose_assert(ip4_zero.is_class_c() == false, "is_class_c()");
    verbose_assert(ip4_unicast1.is_class_c() == false, "is_class_c()");
    verbose_assert(ip4_unicast2.is_class_c() == false, "is_class_c()");
    verbose_assert(ip4_unicast3.is_class_c() == true, "is_class_c()");
    verbose_assert(ip4_class_a1.is_class_c() == false, "is_class_c()");
    verbose_assert(ip4_class_a2.is_class_c() == false, "is_class_c()");
    verbose_assert(ip4_class_a3.is_class_c() == false, "is_class_c()");
    verbose_assert(ip4_class_b1.is_class_c() == false, "is_class_c()");
    verbose_assert(ip4_class_b2.is_class_c() == false, "is_class_c()");
    verbose_assert(ip4_class_b3.is_class_c() == false, "is_class_c()");
    verbose_assert(ip4_class_c1.is_class_c() == true, "is_class_c()");
    verbose_assert(ip4_class_c2.is_class_c() == true, "is_class_c()");
    verbose_assert(ip4_class_c3.is_class_c() == true, "is_class_c()");
    verbose_assert(ip4_multicast1.is_class_c() == false, "is_class_c()");
    verbose_assert(ip4_multicast2.is_class_c() == false, "is_class_c()");
    verbose_assert(ip4_multicast3.is_class_c() == false, "is_class_c()");
    verbose_assert(ip4_experimental1.is_class_c() == false, "is_class_c()");
    verbose_assert(ip4_experimental2.is_class_c() == false, "is_class_c()");
    verbose_assert(ip4_experimental3.is_class_c() == false, "is_class_c()");

    //
    // Test if an address is a valid experimental address.
    //
    verbose_assert(ip4_zero.is_experimental() == false,
		   "is_experimental()");
    verbose_assert(ip4_unicast1.is_experimental() == false,
		   "is_experimental()");
    verbose_assert(ip4_unicast2.is_experimental() == false,
		   "is_experimental()");
    verbose_assert(ip4_unicast3.is_experimental() == false,
		   "is_experimental()");
    verbose_assert(ip4_class_a1.is_experimental() == false,
		   "is_experimental()");
    verbose_assert(ip4_class_a2.is_experimental() == false,
		   "is_experimental()");
    verbose_assert(ip4_class_a3.is_experimental() == false,
		   "is_experimental()");
    verbose_assert(ip4_class_b1.is_experimental() == false,
		   "is_experimental()");
    verbose_assert(ip4_class_b2.is_experimental() == false,
		   "is_experimental()");
    verbose_assert(ip4_class_b3.is_experimental() == false,
		   "is_experimental()");
    verbose_assert(ip4_class_c1.is_experimental() == false,
		   "is_experimental()");
    verbose_assert(ip4_class_c2.is_experimental() == false,
		   "is_experimental()");
    verbose_assert(ip4_class_c3.is_experimental() == false,
		   "is_experimental()");
    verbose_assert(ip4_multicast1.is_experimental() == false,
		   "is_experimental()");
    verbose_assert(ip4_multicast2.is_experimental() == false,
		   "is_experimental()");
    verbose_assert(ip4_multicast3.is_experimental() == false,
		   "is_experimental()");
    verbose_assert(ip4_experimental1.is_experimental() == true,
		   "is_experimental()");
    verbose_assert(ip4_experimental2.is_experimental() == true,
		   "is_experimental()");
    verbose_assert(ip4_experimental3.is_experimental() == true,
		   "is_experimental()");

    //
    // Test if an address is a valid link-local unicast address.
    //
    verbose_assert(ip4_zero.is_linklocal_unicast() == false,
		   "is_linklocal_unicast()");
    verbose_assert(ip4_unicast1.is_linklocal_unicast() == false,
		   "is_linklocal_unicast()");
    verbose_assert(ip4_unicast2.is_linklocal_unicast() == false,
		   "is_linklocal_unicast()");
    verbose_assert(ip4_unicast3.is_linklocal_unicast() == false,
		   "is_linklocal_unicast()");

    //
    // Test if an address is a valid interface-local multicast address.
    //
    verbose_assert(ip4_multicast1.is_interfacelocal_multicast() == false,
		   "is_interfacelocal_multicast()");
    verbose_assert(ip4_multicast2.is_interfacelocal_multicast() == false,
		   "is_interfacelocal_multicast()");
    verbose_assert(ip4_multicast3.is_interfacelocal_multicast() == false,
		   "is_interfacelocal_multicast()");

    //
    // Test if an address is a valid link-local multicast address.
    //
    verbose_assert(ip4_multicast_linklocal1.is_linklocal_multicast() == true,
		   "is_linklocal_multicast()");
    verbose_assert(ip4_multicast2.is_linklocal_multicast() == false,
		   "is_linklocal_multicast()");
    verbose_assert(ip4_multicast3.is_linklocal_multicast() == false,
		   "is_linklocal_multicast()");

    //
    // Test if an address is a valid loopback address.
    //
    verbose_assert(ip4_loopback1.is_loopback() == true, "is_loopback()");
    verbose_assert(ip4_loopback2.is_loopback() == true, "is_loopback()");
    verbose_assert(ip4_zero.is_loopback() == false, "is_loopback()");
    verbose_assert(ip4_unicast1.is_loopback() == false, "is_loopback()");
    verbose_assert(ip4_unicast2.is_loopback() == false, "is_loopback()");
    verbose_assert(ip4_unicast3.is_loopback() == false, "is_loopback()");
}

/**
 * Test IPv4 address constant values.
 */
void
test_ipv4_address_const()
{
    //
    // Test the address octet-size.
    //
    verbose_assert(IPv4::addr_bytelen() == 4, "addr_bytelen()");

    //
    // Test the address bit-length.
    //
    verbose_assert(IPv4::addr_bitlen() == 32, "addr_bitlen()");

    //
    // Test the mask length for the multicast base address.
    //
    verbose_assert(IPv4::ip_multicast_base_address_mask_len() == 4,
		   "ip_multicast_base_address_mask_len()");

    //
    // Test the mask length for the Class A base address.
    //
    verbose_assert(IPv4::ip_class_a_base_address_mask_len() == 1,
		   "ip_class_a_base_address_mask_len()");

    //
    // Test the mask length for the Class B base address.
    //
    verbose_assert(IPv4::ip_class_b_base_address_mask_len() == 2,
		   "ip_class_b_base_address_mask_len()");

    //
    // Test the mask length for the Class C base address.
    //
    verbose_assert(IPv4::ip_class_c_base_address_mask_len() == 3,
		   "ip_class_c_base_address_mask_len()");

    //
    // Test the mask length for the experimental base address.
    //
    verbose_assert(IPv4::ip_experimental_base_address_mask_len() == 4,
		   "ip_experimental_base_address_mask_len()");

    //
    // Test the address family.
    //
    verbose_assert(IPv4::af() == AF_INET, "af()");

    //
    // Test the IP protocol version.
    //
    verbose_assert(IPv4::ip_version() == 4, "ip_version()");
    verbose_assert(IPv4::ip_version_str() == "IPv4", "ip_version_str()");

    //
    // Test pre-defined constant addresses
    //
    verbose_assert(IPv4::ZERO() == IPv4("0.0.0.0"), "ZERO()");

    verbose_assert(IPv4::ANY() == IPv4("0.0.0.0"), "ANY()");

    verbose_assert(IPv4::ALL_ONES() == IPv4("255.255.255.255"),
		   "ALL_ONES()");

    verbose_assert(IPv4::LOOPBACK() == IPv4("127.0.0.1"),
		   "LOOPBACK()");

    verbose_assert(IPv4::MULTICAST_BASE() == IPv4("224.0.0.0"),
		   "MULTICAST_BASE()");

    verbose_assert(IPv4::MULTICAST_ALL_SYSTEMS() == IPv4("224.0.0.1"),
		   "MULTICAST_ALL_SYSTEMS()");

    verbose_assert(IPv4::MULTICAST_ALL_ROUTERS() == IPv4("224.0.0.2"),
		   "MULTICAST_ALL_ROUTERS()");

    verbose_assert(IPv4::DVMRP_ROUTERS() == IPv4("224.0.0.4"),
		   "DVMRP_ROUTERS()");

    verbose_assert(IPv4::OSPFIGP_ROUTERS() == IPv4("224.0.0.5"),
		   "OSPFIGP_ROUTERS()");

    verbose_assert(IPv4::OSPFIGP_DESIGNATED_ROUTERS() == IPv4("224.0.0.6"),
		   "OSPIGP_DESIGNATED_ROUTERS()");

    verbose_assert(IPv4::RIP2_ROUTERS() == IPv4("224.0.0.9"),
		   "RIP2_ROUTERS()");

    verbose_assert(IPv4::PIM_ROUTERS() == IPv4("224.0.0.13"),
		   "PIM_ROUTERS()");

    verbose_assert(IPv4::SSM_ROUTERS() == IPv4("224.0.0.22"),
		   "SSM_ROUTERS()");

    verbose_assert(IPv4::CLASS_A_BASE() == IPv4("0.0.0.0"),
		   "CLASS_A_BASE()");

    verbose_assert(IPv4::CLASS_B_BASE() == IPv4("128.0.0.0"),
		   "CLASS_B_BASE()");

    verbose_assert(IPv4::CLASS_C_BASE() == IPv4("192.0.0.0"),
		   "CLASS_C_BASE()");

    verbose_assert(IPv4::EXPERIMENTAL_BASE() == IPv4("240.0.0.0"),
		   "EXPERIMENTAL_BASE()");
}

/**
 * Test IPv4 address manipulation.
 */
void
test_ipv4_manipulate_address()
{
    //
    // Test making an IPv4 mask prefix.
    //
    verbose_assert(IPv4().make_prefix(24) == IPv4("255.255.255.0"),
		   "make_prefix()");
    verbose_assert(IPv4().make_prefix(0) == IPv4("0.0.0.0"),
		   "make_prefix()");
    verbose_assert(IPv4().make_prefix(32) == IPv4("255.255.255.255"),
		   "make_prefix()");

    //
    // Test making an IPv4 address prefix.
    //
    verbose_assert(
	IPv4("12.34.56.78").mask_by_prefix_len(24) == IPv4("12.34.56.0"),
	"mask_by_prefix_len()"
	);

    //
    // Test getting the prefix length of the contiguous mask.
    //
    verbose_assert(IPv4("255.255.255.0").mask_len() == 24,
		   "mask_len()");

    //
    // Test getting the raw value of the address.
    //
    uint32_t n = htonl((12 << 24) | (34 << 16) | (56 << 8) | 78);
    verbose_assert(IPv4("12.34.56.78").addr() == n, "addr()");

    //
    // Test setting the address value
    //
    IPv4 ip_a("1.2.3.4");
    ip_a.set_addr(htonl((12 << 24) | (34 << 16) | (56 << 8) | 78));
    verbose_assert(ip_a == IPv4("12.34.56.78"), "set_addr()");

    //
    // Test extracting bits from an address.
    //
    verbose_assert(IPv4("12.34.56.78").bits(0, 8) == 78, "bits()");

    //
    // Test counting the number of bits in an address.
    //
    verbose_assert(IPv4::ZERO().bit_count() == 0, "bit_count()");
    verbose_assert(IPv4::ALL_ONES().bit_count() == 32, "bit_count()");
    verbose_assert(IPv4("240.15.240.15").bit_count() == 16, "bit_count()");

    //
    // Test counting the number of leading zeroes in an address.
    //
    verbose_assert(IPv4::ZERO().leading_zero_count() == 32,
		   "leading_zero_count()");
    verbose_assert(IPv4::ALL_ONES().leading_zero_count() == 0,
		   "leading_zero_count()");
    verbose_assert(IPv4("0.1.255.255").leading_zero_count() == 15,
		   "leading_zero_count()");
}

/**
 * Test IPv4 invalid address manipulation.
 */
void
test_ipv4_invalid_manipulate_address()
{
    const char *addr_string4 = "12.34.56.78";

    //
    // Test making an invalid IPv4 mask prefix.
    //
    try {
	// Invalid prefix length
	IPv4 ip(IPv4::make_prefix(IPv4::addr_bitlen() + 1));
	verbose_log("Cannot catch invalid IPv4 mask prefix with length %u : FAIL\n",
		    XORP_UINT_CAST(IPv4::addr_bitlen() + 1));
	incr_failures();
	UNUSED(ip);
    } catch (const InvalidNetmaskLength& e) {
	// The problem was caught
	verbose_log("%s : OK\n", e.str().c_str());
    }

    //
    // Test masking with an invalid IPv4 mask prefix.
    //
    try {
	// Invalid mask prefix
	IPv4 ip(addr_string4);
	ip.mask_by_prefix_len(IPv4::addr_bitlen() + 1);
	verbose_log("Cannot catch masking with an invalid IPv4 mask prefix with length %u : FAIL\n",
		    XORP_UINT_CAST(IPv4::addr_bitlen() + 1));
	incr_failures();
    } catch (const InvalidNetmaskLength& e) {
	// The problem was caught
	verbose_log("%s : OK\n", e.str().c_str());
    }
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
	test_ipv4_valid_constructors();
	test_ipv4_invalid_constructors();
	test_ipv4_valid_copy_in_out();
	test_ipv4_invalid_copy_in_out();
	test_ipv4_operators();
	test_ipv4_address_type();
	test_ipv4_address_const();
	test_ipv4_manipulate_address();
	test_ipv4_invalid_manipulate_address();
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
