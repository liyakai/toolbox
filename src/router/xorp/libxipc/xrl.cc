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



#include "xrl_module.h"
#include "libxorp/debug.h"
#include "xrl.hh"
#include "xrl_tokens.hh"
#include "libxorp/xlog.h"

const string Xrl::_finder_protocol = "finder";

const char*
Xrl::parse_xrl_path(const char* c_str)
{
    const char *sep, *start;

    // void cached string representation
    clear_cache();

    // Extract protocol
    start = c_str;
    sep = strstr(start, XrlToken::PROTO_TGT_SEP);
    if (0 == sep) {
	// Not found, assume finder protocol
	_protocol = _finder_protocol;
    } else {
	_protocol = string(start, sep - start);
	start = sep + TOKEN_BYTES(XrlToken::PROTO_TGT_SEP) - 1;
    }

    // Extract Target
    sep = strstr(start, XrlToken::TGT_CMD_SEP);
    if (0 == sep)
	xorp_throw0(InvalidString);
    _target = string(start, sep - start);
    start = sep + TOKEN_BYTES(XrlToken::TGT_CMD_SEP) - 1;

    // Extract Command
    sep = strstr(start, XrlToken::CMD_ARGS_SEP);
    if (sep == 0) {
	_command = string(start);
	if (_command.size() == 0) {
	    xorp_throw0(InvalidString);
	}
	return 0;
    }
    _command = string(start, sep - start);
    start = sep + TOKEN_BYTES(XrlToken::CMD_ARGS_SEP) - 1;

    return start;
}

Xrl::Xrl(const string&	protocol,
	 const string&	protocol_target,
	 const string&	command,
	 const XrlArgs&	args)
    : _protocol(protocol), _target(protocol_target), _command(command),
      _args(args), _sna_atom(NULL), _packed_bytes(0), _argp(&_args),
      _to_finder(-1), _resolved(false)
{
}

Xrl::Xrl(const string&	target,
	 const string&	command,
	 const XrlArgs&	args)
    : _protocol(_finder_protocol), _target(target), _command(command),
      _args(args), _sna_atom(NULL), _packed_bytes(0), _argp(&_args),
      _to_finder(-1), _resolved(false)
{
}

Xrl::Xrl(const string& protocol,
	 const string& protocol_target,
	 const string& command)
    : _protocol(protocol), _target(protocol_target), _command(command),
      _sna_atom(NULL), _packed_bytes(0), _argp(&_args), _to_finder(-1),
      _resolved(false)
{
}

Xrl::Xrl(const string& target,
	 const string& command)
    : _protocol(_finder_protocol), _target(target), _command(command),
      _sna_atom(NULL), _packed_bytes(0), _argp(&_args), _to_finder(-1),
      _resolved(false)
{
}

Xrl::Xrl(const char* target,
	 const char* command)
	: _protocol(_finder_protocol), _target(target), _command(command),
	  _sna_atom(NULL), _packed_bytes(0), _argp(&_args), _to_finder(-1),
	  _resolved(false)
{
}

Xrl::Xrl(const char* c_str) throw (InvalidString) 
        : _sna_atom(NULL), _packed_bytes(0), _argp(&_args),
	  _to_finder(-1), _resolved(false)
{
    if (0 == c_str)
	xorp_throw0(InvalidString);

    const char* start = parse_xrl_path(c_str);

    // Extract Arguments and pass to XrlArgs string constructor
    if (0 != start && *start != '\0') {
	try {
	    _args = XrlArgs(start);
	} catch (const InvalidString& is) {
	    debug_msg("Failed to restore xrl args:\n\t\"%s\"", start);
	    throw is;
	}
    }
}

Xrl::Xrl() 
    : _sna_atom(0), _packed_bytes(0), _argp(&_args), _to_finder(-1),
      _resolved(false)
{
}

Xrl::Xrl(const Xrl& x)
{
    copy(x);
}

Xrl&
Xrl::operator=(const Xrl& rhs)
{
    copy(rhs);

    return *this;
}

void
Xrl::copy(const Xrl& x)
{
    _protocol	    = x._protocol;
    _target	    = x._target;
    _command	    = x._command;
    _args	    = x.args();
    _string_no_args = x._string_no_args;
    _sna_atom	    = NULL;
    _packed_bytes   = 0; // XXX could use this, but we need to init sna
    _argp	    = &_args;
    _to_finder	    = x._to_finder;
    _resolved	    = x._resolved;
    _resolved_sender	    = x._resolved_sender;
}

Xrl::~Xrl()
{
    delete _sna_atom;
}

string
Xrl::str() const
{
    string s = string_no_args();
    if (args().size()) {
	return s + string(XrlToken::CMD_ARGS_SEP) + args().str();
    }
    return s;
}

bool
Xrl::operator==(const Xrl& x) const
{
    return ((x._protocol == _protocol) && (x._target == _target) &&
	    (x._command == _command) && (x.args() == args()));
}

bool
Xrl::is_resolved() const
{
    // This value is ripe for caching.
    return strcasecmp(_protocol.c_str(), _finder_protocol.c_str());
}

size_t
Xrl::packed_bytes() const
{
    if (!_packed_bytes) {
	    if (!_sna_atom)
		_sna_atom = new XrlAtom(this->string_no_args());

	    _packed_bytes = args().packed_bytes(_sna_atom);
    }

    return _packed_bytes;
}

size_t
Xrl::pack(uint8_t* buffer, size_t buffer_bytes) const
{
    XLOG_ASSERT(_sna_atom);

    return args().pack(buffer, buffer_bytes, _sna_atom);
}

size_t
Xrl::unpack(const uint8_t* buffer, size_t buffer_bytes)
{
    args().clear();

    XrlAtom xa;

    size_t unpacked_bytes = args().unpack(buffer, buffer_bytes, &xa);
    if (unpacked_bytes == 0)
	return 0;

    if (xa.type() != xrlatom_text || xa.has_data() == false)
	return 0;

    parse_xrl_path(xa.text().c_str());

    return unpacked_bytes;
}

void
Xrl::set_args(const Xrl& xrl) const
{
    _packed_bytes = 0;
    _argp = &xrl._args;
}

size_t
Xrl::unpack_command(string& cmd, const uint8_t* in, size_t len)
{
    size_t rc, used = 0;
    uint32_t cnt;
    const char* t;
    uint32_t tl;
    const char* p = NULL;

    used += XrlArgs::unpack_header(cnt, in, len);
    if (!used)
	return 0;

    if (cnt == 0)
	return 0;

    // We now expect a text atom representing the XRL path.
    in  += used;
    len -= used;

    rc = XrlAtom::peek_text(t, tl, in, len);
    if (rc == 0)
	return 0;

    used += rc;

    // OK now we parse out the command.  We can't use libc functions because the
    // string aint 0 terminated.
    cnt = 0;
    for (uint32_t i = 0; i < tl; i++) {
	if (cnt == 3) {
	    p = t;
	    cnt++;
	}

	if (cnt == 4) {
	    if (*p++ == '?') // end of command
		break;

	} else if (*t++ == '/') // find start of command
	    cnt++;
    }
    if (!p)
	return 0;

    // XXX we don't sanity check protocol & target

    cmd.assign(t, p - t);

    return used;
}

size_t
Xrl::fill(const uint8_t* in, size_t len)
{
    _packed_bytes = 0;

    return args().fill(in, len);
}

bool
Xrl::to_finder() const
{
    if (_to_finder == -1) {
	_to_finder = (protocol() == _finder_protocol 
	              && target().substr(0, 6) == _finder_protocol);
    }

    return _to_finder;
}

void
Xrl::set_target(const char* target)
{
    // XXX slowish - maybe we can use pointer value in some cases.
    if (::strcmp(target, _target.c_str()) == 0)
	return;

    clear_cache();

    _target.assign(target);
}

void
Xrl::clear_cache()
{
    _string_no_args = "";
    _packed_bytes   = 0;
    _to_finder	    = -1;
    _resolved	    = false;

    _resolved_sender.reset();

    delete _sna_atom;
    _sna_atom = NULL;
}
