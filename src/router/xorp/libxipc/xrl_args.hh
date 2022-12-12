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


#ifndef __LIBXIPC_XRL_ARGS_HH__
#define __LIBXIPC_XRL_ARGS_HH__

#include "libxorp/xorp.h"
#include "libxorp/ipv4.hh"
#include "libxorp/ipv6.hh"
#include "libxorp/ipv4net.hh"
#include "libxorp/ipv6net.hh"
#include "libxorp/mac.hh"
#include "libxorp/exceptions.hh"

#include "xrl_atom.hh"


class XrlArgs {
public:
    typedef vector<XrlAtom>	    ATOMS;
    typedef ATOMS::const_iterator   const_iterator;
    typedef ATOMS::iterator	    iterator;

    // Exceptions
    struct BadArgs : public XorpException {
	BadArgs(const char* file, int line, const string& reason) :
	    XorpException("XrlArgs::BadArgs", file, line),
	_reason(reason) {}
	const string why() const { return (_reason); }
    private:
	string _reason;
    };
    class XrlAtomNotFound { };
    class XrlAtomFound { };

public:
    XrlArgs() : _have_name(false) {}
    explicit XrlArgs(const char* str) throw (InvalidString);

    ~XrlArgs() {}

    /* --- XrlAtom accessors --- */
    XrlArgs& add(const XrlAtom& xa) throw (XrlAtomFound);

    const XrlAtom& get(const XrlAtom& dataless) const throw (XrlAtomNotFound);
    const XrlAtom& get(unsigned idx, const char* name) const
					    throw (XrlAtomNotFound);

    void remove(const XrlAtom& dataless) throw (XrlAtomNotFound);

    /* --- bool accessors --- */

    XrlArgs& add_bool(const char* name, bool val) throw (XrlAtomFound);

    const bool_t& get_bool(const char* name) const throw (BadArgs);

    void remove_bool(const char* name) throw (XrlAtomNotFound);

    XrlArgs& add(const char* n, bool v) throw (XrlAtomFound);

    void get(const char* n, bool& t) const throw (BadArgs);

    /* --- int32 accessors --- */

    XrlArgs& add_int32(const char* name, int32_t val) throw (XrlAtomFound);

    const int32_t& get_int32(const char* name) const throw (BadArgs);

    void remove_int32(const char* name) throw (XrlAtomNotFound);

    XrlArgs& add(const char* n, int32_t v) throw (XrlAtomFound);

    void get(const char* n, int32_t& t) const throw (BadArgs);

    /* --- uint32 accessors --- */

    XrlArgs& add_uint32(const char* name, uint32_t v) throw (XrlAtomFound);

    const uint32_t& get_uint32(const char* name) const throw (BadArgs);

    void remove_uint32(const char* name) throw (XrlAtomNotFound);

    XrlArgs& add(const char* n, uint32_t v) throw (XrlAtomFound);

    void get(const char* n, uint32_t& t) const throw (BadArgs);

    /* --- ipv4 accessors --- */

    XrlArgs& add_ipv4(const char* n, const IPv4& a) throw (XrlAtomFound);

    const IPv4& get_ipv4(const char* name) const throw (BadArgs);

    void remove_ipv4(const char* name) throw (XrlAtomNotFound);

    XrlArgs& add(const char* n, const IPv4& a) throw (XrlAtomFound);

    void get(const char* n, IPv4& a) const throw (BadArgs);

    /* --- ipv4net accessors --- */

    XrlArgs& add_ipv4net(const char* n, const IPv4Net& a) throw (XrlAtomFound);

    const IPv4Net& get_ipv4net(const char* name) const throw (BadArgs);

    void remove_ipv4net(const char* name) throw (XrlAtomNotFound);

    XrlArgs& add(const char* n, const IPv4Net& v) throw (XrlAtomFound);

    void get(const char* n, IPv4Net& t) const throw (BadArgs);

    /* --- ipv6 accessors --- */

    XrlArgs& add_ipv6(const char* name, const IPv6& addr) throw (XrlAtomFound);

    const IPv6& get_ipv6(const char* name) const throw (BadArgs);

    void remove_ipv6(const char* name) throw (XrlAtomNotFound);

    XrlArgs& add(const char* n, const IPv6& a) throw (XrlAtomFound);

    void get(const char* n, IPv6& a) const throw (BadArgs);

    /* --- ipv6net accessors --- */

    XrlArgs& add_ipv6net(const char* name, const IPv6Net& addr)
	throw (XrlAtomFound);

    const IPv6Net& get_ipv6net(const char* name) const throw (BadArgs);

    void remove_ipv6net(const char* name) throw (XrlAtomNotFound);

    XrlArgs& add(const char* n, const IPv6Net& a) throw (XrlAtomFound);

    void get(const char* n, IPv6Net& a) const throw (BadArgs);

    /* --- ipvx accessors --- */

    XrlArgs& add_ipvx(const char* name, const IPvX& ipvx) throw (XrlAtomFound);

    const IPvX get_ipvx(const char* name) const throw (BadArgs);

    void remove_ipvx(const char* name) throw (XrlAtomNotFound);

    XrlArgs& add(const char* n, const IPvX& a) throw (XrlAtomFound);

    void get(const char* n, IPvX& a) const throw (BadArgs);

    /* --- ipvxnet accessors --- */

    XrlArgs& add_ipvxnet(const char* name, const IPvXNet& ipvxnet)
	throw (XrlAtomFound);

    const IPvXNet get_ipvxnet(const char* name) const throw (BadArgs);

    void remove_ipvxnet(const char* name) throw (XrlAtomNotFound);

    XrlArgs& add(const char* n, const IPvXNet& a) throw (XrlAtomFound);

    void get(const char* n, IPvXNet& a) const throw (BadArgs);

    /* --- mac accessors --- */

    XrlArgs& add_mac(const char* name, const Mac& addr) throw (XrlAtomFound);

    const Mac& get_mac(const char* name) const throw (BadArgs);

    void remove_mac(const char* name) throw (XrlAtomNotFound);

    XrlArgs& add(const char* n, const Mac& a) throw (XrlAtomFound);

    void get(const char* n, Mac& a) const throw (BadArgs);

    /* --- string accessors --- */

    XrlArgs& add_string(const char* name, const string& addr)
	throw (XrlAtomFound);

    const string& get_string(const char* name) const throw (BadArgs);

    void remove_string(const char* name) throw (XrlAtomNotFound);

    XrlArgs& add(const char* n, const string& a) throw (XrlAtomFound);

    void get(const char* n, string& a) const throw (BadArgs);

    /* --- list accessors --- */

    XrlArgs& add_list(const char* name, const XrlAtomList& addr)
	throw (XrlAtomFound);

    const XrlAtomList& get_list(const char* name) const throw (BadArgs);

    void remove_list(const char* name) throw (XrlAtomNotFound);

    XrlArgs& add(const char* n, const XrlAtomList& a) throw (XrlAtomFound);

    void get(const char* n, XrlAtomList& a) const throw (BadArgs);

    /* --- binary data accessors --- */
    XrlArgs& add_binary(const char* name, const vector<uint8_t>& addr)
	throw (XrlAtomFound);

    const vector<uint8_t>& get_binary(const char* name) const throw (BadArgs);

    void remove_binary(const char* name) throw (XrlAtomNotFound);

    XrlArgs& add(const char* n, const vector<uint8_t>& a) throw (XrlAtomFound);

    void get(const char* n, vector<uint8_t>& a) const throw (BadArgs);

    /* --- int64 accessors --- */

    XrlArgs& add_int64(const char* name, int64_t v) throw (XrlAtomFound);

    const int64_t& get_int64(const char* name) const throw (BadArgs);

    void remove_int64(const char* name) throw (XrlAtomNotFound);

    XrlArgs& add(const char* n, int64_t v) throw (XrlAtomFound);

    void get(const char* n, int64_t& t) const throw (BadArgs);

    /* --- uint64 accessors --- */

    XrlArgs& add_uint64(const char* name, uint64_t v) throw (XrlAtomFound);

    const uint64_t& get_uint64(const char* name) const throw (BadArgs);

    void remove_uint64(const char* name) throw (XrlAtomNotFound);

    XrlArgs& add(const char* n, uint64_t v) throw (XrlAtomFound);

    void get(const char* n, uint64_t& t) const throw (BadArgs);

    /* --- fp64 accessors --- */

    XrlArgs& add_fp64(const char* name, fp64_t v) throw (XrlAtomFound);

    const fp64_t& get_fp64(const char* name) const throw (BadArgs);

    void remove_fp64(const char* name) throw (XrlAtomNotFound);

    XrlArgs& add(const char* n, fp64_t v) throw (XrlAtomFound);

    void get(const char* n, fp64_t& t) const throw (BadArgs);


    // ... Add your type's add, get, remove functions here ...

    // Append all atoms from an existing XrlArgs structure
    XrlArgs& add(const XrlArgs& args) throw (XrlAtomFound);

    // Equality testing
    bool matches_template(XrlArgs& t) const;
    bool operator==(const XrlArgs& t) const;

    // Accessor helpers
    size_t size() const;

    const XrlAtom& operator[](uint32_t index) const; // throw out_of_range
    const XrlAtom& item(uint32_t index) const;

    const XrlAtom& operator[](const string& name) const
	throw (XrlAtomNotFound);

    const XrlAtom& item(const string& name) const throw (XrlAtomNotFound);

    void push_back(const XrlAtom& xa);
    const XrlAtom& back() throw (XrlAtomNotFound);
    void pop_back() throw (XrlAtomNotFound);

    const_iterator begin() const	{ return _args.begin(); }
    const_iterator end() const		{ return _args.end(); }

    void clear()			{ _args.clear(); }
    bool empty()			{ return _args.empty(); }
    void swap(XrlArgs& xa)		{ _args.swap(xa._args); }

    /**
     * Get number of bytes needed to pack atoms contained within
     * instance.
     */
    size_t packed_bytes(XrlAtom* head = NULL) const;

    /**
     * Pack contained atoms into a byte array.  The size of the byte
     * array should be larger or equal to the value returned by
     * packed_bytes().
     *
     * @param buffer buffer to receive data.
     * @param buffer_bytes size of buffer.
     * @return size of packed data on success, 0 on failure.
     */
    size_t pack(uint8_t* buffer, size_t buffer_bytes,
                XrlAtom* head = NULL) const;

    /**
     * Unpack atoms from byte array into instance.  The atoms are
     * appended to the list of atoms associated with instance.
     *
     * @param buffer to read data from.
     * @param buffer_bytes size of buffer.  The size should exactly match
     *        number of bytes of packed atoms, ie packed_bytes() value
     *        used when packing.
     * @return number of bytes turned into atoms on success, 0 on failure.
     */
    size_t unpack(const uint8_t* buffer, size_t buffer_bytes,
		  XrlAtom* head = NULL);

    size_t fill(const uint8_t* buffer, size_t buffer_bytes);

    static size_t unpack_header(uint32_t& cnt, const uint8_t* in, size_t len);

    // String serialization methods
    string str() const;

    template <class T>
    void set_arg(int idx, const T& arg);

protected:
    void check_not_found(const XrlAtom &xa) throw (XrlAtomFound);

    ATOMS _args;
    bool  _have_name;
};


// ----------------------------------------------------------------------------
// Inline methods

inline XrlArgs&
XrlArgs::add(const char* n, bool v) throw (XrlAtomFound)
{
    return add_bool(n, v);
}

inline void
XrlArgs::get(const char* n, bool& t) const throw (BadArgs)
{
    t = get_bool(n);
}

inline XrlArgs&
XrlArgs::add(const char* n, int32_t v) throw (XrlAtomFound)
{
    return add_int32(n, v);
}

inline void
XrlArgs::get(const char* n, int32_t& t) const throw (BadArgs)
{
    t = get_int32(n);
}

inline XrlArgs&
XrlArgs::add(const char* n, uint32_t v) throw (XrlAtomFound)
{
    return add_uint32(n, v);
}

inline void
XrlArgs::get(const char* n, uint32_t& t) const throw (BadArgs)
{
    t = get_uint32(n);
}

inline XrlArgs&
XrlArgs::add(const char* n, const IPv4& a) throw (XrlAtomFound)
{
    return add_ipv4(n, a);
}

inline void
XrlArgs::get(const char* n, IPv4& a) const throw (BadArgs)
{
    a = get_ipv4(n);
}

inline XrlArgs&
XrlArgs::add(const char* n, const IPv4Net& v) throw (XrlAtomFound)
{
    return add_ipv4net(n, v);
}

inline void
XrlArgs::get(const char* n, IPv4Net& t) const throw (BadArgs)
{
    t = get_ipv4net(n);
}

inline XrlArgs&
XrlArgs::add(const char* n, const IPv6& a) throw (XrlAtomFound)
{
    return add_ipv6(n, a);
}

inline void
XrlArgs::get(const char* n, IPv6& a) const throw (BadArgs)
{
    a = get_ipv6(n);
}

inline XrlArgs&
XrlArgs::add(const char* n, const IPv6Net& a) throw (XrlAtomFound)
{
    return add_ipv6net(n, a);
}

inline void
XrlArgs::get(const char* n, IPv6Net& a) const throw (BadArgs)
{
    a = get_ipv6net(n);
}

inline XrlArgs&
XrlArgs::add(const char* n, const IPvX& a) throw (XrlAtomFound)
{
    return add_ipvx(n, a);
}

inline void
XrlArgs::get(const char* n, IPvX& a) const throw (BadArgs)
{
    a = get_ipvx(n);
}

inline XrlArgs&
XrlArgs::add(const char* n, const IPvXNet& a) throw (XrlAtomFound)
{
    return add_ipvxnet(n, a);
}

inline void
XrlArgs::get(const char* n, IPvXNet& a) const throw (BadArgs)
{
    a = get_ipvxnet(n);
}

inline XrlArgs&
XrlArgs::add(const char* n, const Mac& a) throw (XrlAtomFound)
{
    return add_mac(n, a);
}

inline void
XrlArgs::get(const char* n, Mac& a) const throw (BadArgs)
{
    a = get_mac(n);
}

inline XrlArgs&
XrlArgs::add(const char* n, const string& a) throw (XrlAtomFound)
{
    return add_string(n, a);
}

inline void
XrlArgs::get(const char* n, string& a) const throw (BadArgs)
{
    a = get_string(n);
}

inline XrlArgs&
XrlArgs::add(const char* n, const XrlAtomList& a) throw (XrlAtomFound)
{
    return add_list(n, a);
}

inline void
XrlArgs::get(const char* n, XrlAtomList& a) const throw (BadArgs)
{
    a = get_list(n);
}

inline XrlArgs&
XrlArgs::add(const char* n, const vector<uint8_t>& a) throw (XrlAtomFound)
{
    return add_binary(n, a);
}

inline void
XrlArgs::get(const char* n, vector<uint8_t>& a) const throw (BadArgs)
{
    a = get_binary(n);
}

inline XrlArgs&
XrlArgs::add(const char* n, int64_t v) throw (XrlAtomFound)
{
    return add_int64(n, v);
}

inline void
XrlArgs::get(const char* n, int64_t& t) const throw (BadArgs)
{
    t = get_int64(n);
}

inline XrlArgs&
XrlArgs::add(const char* n, uint64_t v) throw (XrlAtomFound)
{
    return add_uint64(n, v);
}

inline void
XrlArgs::get(const char* n, uint64_t& t) const throw (BadArgs)
{
    t = get_uint64(n);
}

inline XrlArgs&
XrlArgs::add(const char* n, fp64_t v) throw (XrlAtomFound)
{
    return add_fp64(n, v);
}

inline void
XrlArgs::get(const char* n, fp64_t& t) const throw (BadArgs)
{
    t = get_fp64(n);
}

inline const XrlAtom&
XrlArgs::item(uint32_t index) const
{
    return operator[](index);
}

inline const XrlAtom&
XrlArgs::item(const string& name) const throw (XrlAtomNotFound)
{
    return operator[](name);
}

inline void
XrlArgs::push_back(const XrlAtom& xa)
{
    _args.push_back(xa);
}

inline const XrlAtom&
XrlArgs::back() throw (XrlAtomNotFound)
{
    if (_args.empty())
	throw XrlAtomNotFound();
    return _args.back();
}

inline void
XrlArgs::pop_back() throw (XrlAtomNotFound)
{
    if (_args.empty())
	throw XrlAtomNotFound();
    _args.pop_back();
}

template <class T>
inline void
XrlArgs::set_arg(int idx, const T& arg)
{
    XrlAtom& a = _args[idx];

    a.set(arg);
}

#endif // __LIBXIPC_XRL_ARGS_HH__
