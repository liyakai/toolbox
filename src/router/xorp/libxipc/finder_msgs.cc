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



#include "libxorp/xorp.h"
#include "libxorp/c_format.hh"

#include "xrl_atom_encoding.hh"
#include "finder_msgs.hh"

///////////////////////////////////////////////////////////////////////////////
//
// Format strings for Finder Messages and their parsers.  Under no
// circumstances change one without changing the other...
//

static const uint8_t FINDER_PROTOCOL_MAJOR_VERSION = 0;
static const uint8_t FINDER_PROTOCOL_MINOR_VERSION = 2;

#define FMC_PREAMBLE	"Finder "
#define FMC_VERSION_SEP "."
#define FMC_MSG_TYPE	"\nMsgType "
#define FMC_SEQ_NO	"\nSeqNo "
#define FMC_MSG_DATA	"\nMsgData "
#define STATIC_BYTES(x) (sizeof(x))

const char* FinderMessageBase::c_msg_template =
	FMC_PREAMBLE "%1d" FMC_VERSION_SEP "%1d"
	FMC_MSG_TYPE "%c"
	FMC_SEQ_NO   "%u"
	FMC_MSG_DATA;

// C-String Xrl
const char* FinderXrlMessage::c_msg_template = "%s";

// XrlError no / XrlError note / C-String XrlArgs
const char* FinderXrlResponse::c_msg_template = "%u / %s\n%s";

///////////////////////////////////////////////////////////////////////////////
//
// FinderMessageBase and ParsedFinderMessageBase
//

FinderMessageBase::FinderMessageBase(uint32_t seqno, char type)
{
    _rendered = c_format(c_msg_template,
			 FINDER_PROTOCOL_MAJOR_VERSION,
			 FINDER_PROTOCOL_MINOR_VERSION,
			 type, seqno);
}

FinderMessageBase::~FinderMessageBase()
{
}

static const char*
line_end(const char* c)
{
    while (*c && *c != '\n')
	c++;
    return c;
}

static bool
skip_text(const char*& buf, const char* to_skip)
{
    while (*to_skip) {
	if (*buf != *to_skip)
	    return false;
	buf++;
	to_skip++;
    }
    return true;
}

ParsedFinderMessageBase::ParsedFinderMessageBase(const char* data, char type)
    throw (BadFinderMessageFormat, WrongFinderMessageType)
{
    const char* pos = data;

    // Version number checking
    if (skip_text(pos, FMC_PREAMBLE) == false) {
	xorp_throw(BadFinderMessageFormat,
		   "Corrupt header field label: bad protocol");
    }

    const char* end = line_end(pos);
    if (end - pos < 3) {
	xorp_throw(BadFinderMessageFormat,
		   "bad version number");
    }
    
    int major, minor;
    major = *pos - '0';
    pos ++;
    if (skip_text(pos, FMC_VERSION_SEP) == false) {
	xorp_throw(BadFinderMessageFormat,
		   "Corrupt header field: major/minor separator");
    }
    
    minor = *pos - '0';
    if (major != FINDER_PROTOCOL_MAJOR_VERSION ||
	minor != FINDER_PROTOCOL_MINOR_VERSION) {
	xorp_throw(BadFinderMessageFormat, "Mismatched protocol version");
    }
    pos++;

    // Msg type checking and extraction
    if (skip_text(pos, FMC_MSG_TYPE) == false) {
	xorp_throw(BadFinderMessageFormat,
		   "Corrupt header field label: message type");
    }
    end = line_end(pos);
    if (end - pos != 1) {
	xorp_throw(BadFinderMessageFormat,
		   "Corrupt header field: fat message type");
    }
    _type = *pos;
    if (_type != type) 
	xorp_throw0(WrongFinderMessageType);
    pos++;

    // Seq no checking and extraction
    if (skip_text(pos, FMC_SEQ_NO) == false) {
	xorp_throw(BadFinderMessageFormat,
		   "Corrupt header field label: sequence number");
    }
    end = line_end(pos);
    
    _seqno = 0;
    while (xorp_isdigit(*pos)) {
	_seqno *= 10;
	_seqno += *pos - '0';
	pos++;
    }

    if (pos != end) {
	xorp_throw(BadFinderMessageFormat,
		   "Corrupt header field: sequence number");
    }

    // Message data checking
    if (skip_text(pos, FMC_MSG_DATA) == false) {
	xorp_throw(BadFinderMessageFormat,
		   "Corrupt header field label: message data");
    }
    end = line_end(pos);
    
    _bytes_parsed = pos - data;
}

ParsedFinderMessageBase::~ParsedFinderMessageBase()
{
}

///////////////////////////////////////////////////////////////////////////////
//
// FinderXrlMessage and ParsedFinderXrlMessage
//

uint32_t FinderXrlMessage::c_seqno = 1001;

FinderXrlMessage::FinderXrlMessage(const Xrl& xrl)
    : FinderMessageBase(c_seqno, c_type), _seqno(c_seqno++)
{
    _rendered += c_format(c_msg_template, xrl.str().c_str());
}

ParsedFinderXrlMessage::ParsedFinderXrlMessage(const char* data)
    throw (BadFinderMessageFormat, WrongFinderMessageType, InvalidString)
    : ParsedFinderMessageBase(data, FinderXrlMessage::c_type)
{
    // The following may throw an invalid string exception
    _xrl = new Xrl(data + bytes_parsed());
}

ParsedFinderXrlMessage::~ParsedFinderXrlMessage()
{
    delete _xrl;
} 

///////////////////////////////////////////////////////////////////////////////
//
// FinderXrlMessage
//

FinderXrlResponse::FinderXrlResponse(uint32_t	     seqno,
				     const XrlError& e,
				     const XrlArgs*  args)
    : FinderMessageBase(seqno, c_type)
{
    // Protect note associated with XrlError by xrl encoding string.
    string e_note(xrlatom_encode_value(e.note()));

    if (args) {
	_rendered += c_format(c_msg_template,
			      e.error_code(),
			      e_note.c_str(),
			      args->str().c_str());
    } else {
	_rendered += c_format(c_msg_template,
			      e.error_code(),
			      e_note.c_str(),
			      "");
    }
}

ParsedFinderXrlResponse::ParsedFinderXrlResponse(const char* data)
    throw (BadFinderMessageFormat, WrongFinderMessageType, InvalidString)
    : ParsedFinderMessageBase(data, FinderXrlResponse::c_type), _xrl_args(0)
{
    data += bytes_parsed();
    char* p0 = strstr(const_cast<char*>(data), "/");
    char* p1 = strstr(const_cast<char*>(data), "\n");
    if (p0 == 0 || p1 == 0) {
	xorp_throw(BadFinderMessageFormat, "XrlError not present");
    }

    uint32_t code = 0;
    while (xorp_isdigit(*data)) {
	code *= 10;
	code += *data - '0';
	data++;
    }

    if (XrlError::known_code(code) == false) {
	xorp_throw(InvalidString, "Unknown Xrl error code");
    }

    string note;
    if (p0 + 2 < p1 &&
	xrlatom_decode_value(p0 + 2, p1 - p0 - 2, note) >= 0) {
	xorp_throw(InvalidString, "Code not decode XrlError note.");    
    }

    _xrl_error = XrlError(XrlErrorCode(code), note);

    p1++;
    if (*p1 != '\0')
	_xrl_args = new XrlArgs(p1);
}

ParsedFinderXrlResponse::~ParsedFinderXrlResponse()
{
    delete _xrl_args;
}

