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



#include "xrl_module.h"

#include "libxorp/xorp.h"

#include "xrl_atom_encoding.hh"


// Encoding here is URL encoding like:
// 	Alpha-numerics are not touched
// 	' ' -> '+'
//	Characters with special Xrl meaning and non-printing characters
//	are URL escape quoted (%[0-f][0-f]).

// Characters that require escaping
static const char ESCAPE_CHARS[] = "[]&=+%$,;{}# ";

inline static bool
needs_escape(char c)
{
    size_t n = sizeof(ESCAPE_CHARS) / sizeof(ESCAPE_CHARS[0]);
    for (size_t i = 0; i < n; i++) {
	if (ESCAPE_CHARS[i] == c)
	    return true;
    }
    return ( xorp_iscntrl(c) || !isascii(c) );
}

// ----------------------------------------------------------------------------
// Fast escape required lookup

static uint8_t	escape_table[256 >> 3];
static bool	escape_table_inited;

static void
init_escape_table()
{
    for (int i = 0; i < 8; i++)
	escape_table[i] = 0;
    for (int i = 0; i < 256; i++) {
	if (needs_escape(i)) {
	    escape_table[i>>3] |= 1 << (i & 0x07);
	}
    }
    escape_table_inited = true;
}

// A table lookup would be better than this...only requires 8
// chars and some bit shifting arithmetic - char_index = i >> 3
// bit_index = i & 0x07

inline static bool
fast_needs_escape(char c)
{
    bool n = (escape_table[((uint8_t)c)>>3] & (1 << ((uint8_t)c & 0x07)));
    //    printf(" %c (%02x) %d\n", c, c, n);
    return n;
}

// ----------------------------------------------------------------------------

inline static bool
is_a_quote(char c)
{
    return (c == '%' || c == '+');
}

inline void
escape_encode(char c, char*& buf)
{
    if (c == ' ') {
	*buf++ = '+';
    } else {
	*buf++ = '%';
	int v = (c & 0xf0) >> 4;
	if (v < 10) 
	    *buf++ = v + '0';
	else
	    *buf++ = (v - 10) + 'A';
	v = c & 0x0f;
	if (v < 10)
	    *buf++ = v + '0';
	else
	    *buf++ = (v - 10) + 'A';
    }
}

inline static char
hex_digit(char c)
{
    if (c >= '0' && c <= '9')
	return c - '0';
    if (c >= 'a' && c <= 'f')
	return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
	return c - 'A' + 10;
    return 0x1f;
}

inline static ssize_t
escape_decode(const char* c, char& out)
{
    if (*c == '+') {
	out = ' ';
	return 1;
    }
    assert(*c == '%');
    char h = hex_digit(*(c + 1));
    char l = hex_digit(*(c + 2));
    if (h <= 15 && l <= 15) {
	char c = char(h * 16 + l);
	out = c;
	return 3;
    }
    return -1;
}

string
xrlatom_encode_value(const char* val, size_t val_bytes)
{
    if (!escape_table_inited) init_escape_table();

    const char* val_end = val + val_bytes;

    // Copy regions of text to avoid unnecessary string append operations
    const char* reg_start = val; // region start
    const char* reg_end;	 // region end

    string out;			 // output string

    char encoded_data[val_bytes * 4 + 1];
    while (reg_start != val_end) {
	reg_end = reg_start;
	while (reg_end != val_end && fast_needs_escape(*reg_end) == false) {
	    reg_end++;
	}
	out.append(reg_start, reg_end);

	// Do escapes one at a time.
	reg_start = reg_end;
	char *next_code = &encoded_data[0];
	bool escaped = false;
	while (reg_start != val_end && fast_needs_escape(*reg_start) == true) {
	    //out.append(escape_encode(*reg_start));
	    escape_encode(*reg_start, next_code);
	    reg_start++;
	    escaped = true;
	}
	if (escaped) {
	    *next_code='\0';
	    out.append(encoded_data);
	}
    }
    return out;
}

ssize_t
xrlatom_decode_value(const char* input, size_t input_bytes, string& out)
{
    out.resize(0);

    const char* input_end = input + input_bytes;
    const char* reg_start = input;
    const char* reg_end;

    reg_start = input;
    while (reg_start < input_end) {
	// Copy non-escaped sequences as a block
	reg_end = reg_start;
	while (reg_end < input_end && is_a_quote(*reg_end) == false) {
	    reg_end++;
	}
	out.insert(out.end(), reg_start, reg_end);

	// Deal with escaped sequences one at a time
	reg_start = reg_end;
	while (reg_start < input_end && is_a_quote(*reg_start) == true) {
	    if (*reg_start == '%' && reg_start + 3 > input_end) {
		// Malformed escape at end of string eg, not %[0-f][0-f]
		return (reg_start - input);
	    }
	    char c = 0;
	    ssize_t skip = escape_decode(reg_start, c);
	    out.insert(out.end(), c);
	    if (skip < 1) {
		// Decoding failed return position of failure
		return (reg_start - input);
	    }
	    reg_start += skip;
	}
    }
    return -1;
}

// The code for this function is essentially cut-and-paste of above with
// minor edits.  Could probably be templatized
ssize_t
xrlatom_decode_value(const char* input, size_t input_bytes,
		     vector<uint8_t>& out)
{
    out.resize(0);

    const char* input_end = input + input_bytes;
    const char* reg_start = input;
    const char* reg_end;

    reg_start = input;
    while (reg_start < input_end) {
	// Copy non-escaped sequences as a block
	reg_end = reg_start;
	while (reg_end < input_end && is_a_quote(*reg_end) == false) {
	    reg_end++;
	}
	out.insert(out.end(),
		   reinterpret_cast<const uint8_t*>(reg_start),
		   reinterpret_cast<const uint8_t*>(reg_end));

	// Deal with escaped sequences one at a time
	reg_start = reg_end;
	while (reg_start < input_end && is_a_quote(*reg_start) == true) {
	    if (*reg_start == '%' && reg_start + 3 > input_end) {
		// Malformed escape at end of string eg, not %[0-f][0-f]
		return (reg_start - input);
	    }
	    char c = 0;
	    ssize_t skip = escape_decode(reg_start, c);
	    out.insert(out.end(), c);
	    if (skip < 1) {
		// Decoding failed return position of failure
		return (reg_start - input);
	    }
	    reg_start += skip;
	}
    }
    return -1;
}

#ifdef TEST_XRLATOM_ENCODING

void
test_decode(const string& encoded, int expected_failure_position)
{
    string decoded;
    ssize_t failure = xrlatom_decode_value(encoded, decoded);
    assert(failure == expected_failure_position);
}

void
test_encode_and_decode(const string& s)
{
    string decoded, encoded;
    encoded = xrlatom_encode_value(s);

    assert(xrlatom_decode_value(encoded, decoded) == -1);
    assert(decoded == s);
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

    test_decode("%", 0);	// malformed
    test_decode("%%", 0);	// malformed
    test_decode("%ab", -1);	// okay

    test_encode_and_decode("%");
    test_encode_and_decode("%%");
    test_encode_and_decode("%%abc%&^*(!::\"");
    test_encode_and_decode("foo:bar");
    test_encode_and_decode("The  cat sat on the mat..,\n");

    //
    // Gracefully stop and exit xlog
    //
    xlog_stop();
    xlog_exit();

    return 0;
}

#endif
