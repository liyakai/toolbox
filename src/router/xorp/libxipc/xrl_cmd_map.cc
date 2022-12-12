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
#include "libxorp/debug.h"

#include "xrl_cmd_map.hh"

void
XrlCmdEntry::invoke_sync(const XrlArgs& in, XrlRespCallback out,
			 XrlRecvSyncCallback impl)
{
    XrlArgs out_args;
    XrlCmdError e = impl->dispatch(in, &out_args);
    out->dispatch(e, &out_args);
}

bool
XrlCmdMap::add_handler(const XrlCmdEntry& cmd)
{
    if (get_handler(cmd.name())) {
	debug_msg("XrlCmdMap::add_handler for \"%s\" failed"
		  ": a handler already exists\n", cmd.name().c_str());
	return false;
    }
    _cmd_map.insert(CmdMap::value_type(cmd.name(), cmd));
    return true;
}

bool
XrlCmdMap::add_handler_internal(const string& cmd,
				const XrlRecvAsyncCallback& rcb)
{
    return add_handler(XrlCmdEntry(cmd, rcb));
}

const XrlCmdEntry*
XrlCmdMap::get_handler(const string& name) const
{
    CmdMap::const_iterator c = _cmd_map.find(name);
    if (c == _cmd_map.end())
	return 0;
    return &c->second;
}

bool
XrlCmdMap::remove_handler(const string& name)
{
    CmdMap::iterator c = _cmd_map.find(name);
    if (c == _cmd_map.end())
	return false;
    _cmd_map.erase(c);
    return true;
}

const XrlCmdEntry*
XrlCmdMap::get_handler(uint32_t index) const
{
    CmdMap::const_iterator c;
    for (c = _cmd_map.begin(); c != _cmd_map.end(); c++, index--) {
	if (index == 0) {
	    return &c->second;
	}
    }
    return NULL;
}

uint32_t
XrlCmdMap::count_handlers() const
{
    return _cmd_map.size();
}

void
XrlCmdMap::get_command_names(list<string>& out) const
{
    CmdMap::const_iterator ci;
    for (ci = _cmd_map.begin(); ci != _cmd_map.end(); ++ci) {
	out.push_back(ci->first);
    }
}

void
XrlCmdMap::finalize()
{
}
