// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
// vim:set sts=4 ts=8:

// Copyright (c) 2001-2011 XORP, Inc and Others
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, Version 2, June
// 1991 as published by the Free Software Foundation. Redistribution
// and/or modification of this program under the terms of any other
// version of the GNU General Public License is not permitted.
// 
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For more details,
// see the GNU General Public License, Version 2, a copy of which can be
// found in the XORP LICENSE.gpl file.
// 
// XORP Inc, 2953 Bunker Hill Lane, Suite 204, Santa Clara, CA 95054, USA;
// http://xorp.net


#ifndef __POLICY_COMMON_ELEM_FILTER_HH__
#define __POLICY_COMMON_ELEM_FILTER_HH__

#include "policy/backend/policy_filter.hh"
#include "element_base.hh"
#include "libxorp/ref_ptr.hh"


/**
 * @short a filter element.  Used when versioning.
 */
class ElemFilter : public Element {
public:
    static Hash _hash;
    
    ElemFilter(const RefPf& pf) : Element(_hash), _pf(pf) {}
    string str() const { return "policy filter"; }
    const RefPf& val() const { return _pf; }

    const char* type() const { return "filter"; }

    string dbgstr() const {
	ostringstream oss;
	oss << "ElemFilter: hash: " << (int)(hash()) << flush;
	return oss.str();
    }

private:
    RefPf _pf;
};

#endif // __POLICY_COMMON_ELEM_FILTER_HH__
