// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-

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

#include "ref_ptr.hh"


///////////////////////////////////////////////////////////////////////////////
//
// Debugging macro's
//

#define POOL_PARANOIA(x) /* x */
#define VERBOSE_POOL_PARANOIA(x) /* x */

///////////////////////////////////////////////////////////////////////////////
//
// ref_counter_pool implementation
//

ref_counter_pool ref_counter_pool::_the_instance;

ref_counter_pool&
ref_counter_pool::instance()
{
    return ref_counter_pool::_the_instance;
}

void
ref_counter_pool::grow()
{
    size_t old_size = _counters.size();
    _counters.resize(old_size + old_size / 8 + 1);

    for (size_t i = old_size; i < _counters.size(); i++) {
	_counters[i] = _free_index;
	_free_index = i;
    }
    POOL_PARANOIA(check());
}

void
ref_counter_pool::check()
{
    int32_t i = _free_index;
    size_t n = 0;
    VERBOSE_POOL_PARANOIA(cout << "L: ");
    while (_counters[i] != LAST_FREE) {
	VERBOSE_POOL_PARANOIA(cout << i << " ");
	i = _counters[i];
	n++;
	if (n == _counters.size()) {
	    dump();
	    abort();
	}
    }
    VERBOSE_POOL_PARANOIA(cout << endl);
}

bool
ref_counter_pool::on_free_list(int32_t index)
{
    int32_t i = _free_index;
    size_t n = 0;
    while (_counters[i] != LAST_FREE) {
	if (i == index) {
	    return true;
	}
	i = _counters[i];
	n++;

	if (n == _counters.size()) {
	    dump();
	    abort();
	}
    }
    return false;
}

void
ref_counter_pool::dump()
{
    for (size_t i = 0; i < _counters.size(); i++) {
	cout << i << " " << _counters[i] << endl;
    }
    cout << "Free index: " << _free_index << endl;
    cout << "Balance: " << _balance << endl;
}

ref_counter_pool::ref_counter_pool()
{
    const size_t n = 1;
    _counters.resize(n);
    _counters[n - 1] = LAST_FREE;
    _free_index = 0;

    grow();
    grow();
    VERBOSE_POOL_PARANOIA(dump());
}

int32_t
ref_counter_pool::new_counter()
{
    VERBOSE_POOL_PARANOIA(dump());
    if (_counters[_free_index] == LAST_FREE) {
	grow();
    }
    POOL_PARANOIA(assert(_counters[_free_index] != LAST_FREE));
    POOL_PARANOIA(check());
    int32_t new_counter = _free_index;
    _free_index = _counters[_free_index];
    _counters[new_counter] = 1;
    ++_balance;
    POOL_PARANOIA(check());
    return new_counter;
}

int32_t
ref_counter_pool::incr_counter(int32_t index)
{
    POOL_PARANOIA(assert(on_free_list(index) == false));
    POOL_PARANOIA(check());
    assert((size_t)index < _counters.size());
    ++_counters[index];
    ++_balance;
    POOL_PARANOIA(check());
    return _counters[index];
}

int32_t
ref_counter_pool::decr_counter(int32_t index)
{
    POOL_PARANOIA(assert((size_t)index < _counters.size()));
    int32_t c = --_counters[index];
    --_balance;
    if (c == 0) {
	POOL_PARANOIA(check());
	/* recycle */
	_counters[index] = _free_index;
	_free_index = index;
	VERBOSE_POOL_PARANOIA(dump());
	POOL_PARANOIA(check());
    }
    assert(c >= 0);
    return c;
}

int32_t
ref_counter_pool::count(int32_t index)
{
    POOL_PARANOIA(assert((size_t)index < _counters.size()));
    return _counters[index];
}

///////////////////////////////////////////////////////////////////////////////
//
// cref_counter_pool implementation
//

cref_counter_pool cref_counter_pool::_the_instance;

cref_counter_pool&
cref_counter_pool::instance()
{
    return cref_counter_pool::_the_instance;
}

void
cref_counter_pool::grow()
{
    size_t old_size = _counters.size();
    _counters.resize(2 * old_size);

    for (size_t i = old_size; i < _counters.size(); i++) {
	_counters[i].count = _free_index;
	_free_index = i;
    }
}

void
cref_counter_pool::check()
{
    int32_t i = _free_index;
    size_t n = 0;
    VERBOSE_POOL_PARANOIA(cout << "L: ");
    while (_counters[i].count != LAST_FREE) {
	VERBOSE_POOL_PARANOIA(cout << i << " ");
	i = _counters[i].count;
	n++;
	if (n == _counters.size()) {
	    dump();
	    abort();
	}
    }
    VERBOSE_POOL_PARANOIA(cout << endl);
}

void
cref_counter_pool::dump()
{
    for (size_t i = 0; i < _counters.size(); i++) {
	cout << i << " " << _counters[i].count << endl;
    }
    cout << "Free index: " << _free_index << endl;
}

cref_counter_pool::cref_counter_pool()
{
    const size_t n = 1;
    _counters.resize(n);
    _free_index = 0; // first free item
    _counters[n - 1].count = LAST_FREE;
    grow();
    grow();
    VERBOSE_POOL_PARANOIA(dump());
}

int32_t
cref_counter_pool::new_counter(void* data)
{
    VERBOSE_POOL_PARANOIA(dump());
    if (_counters[_free_index].count == LAST_FREE) {
	grow();
    }
    POOL_PARANOIA(assert(_counters[_free_index].count != LAST_FREE));
    POOL_PARANOIA(check());
    int32_t new_counter = _free_index;
    _free_index = _counters[_free_index].count;
    _counters[new_counter].count = 1;
    _counters[new_counter].data = data;
    POOL_PARANOIA(check());
    return new_counter;
}

int32_t
cref_counter_pool::incr_counter(int32_t index)
{
    POOL_PARANOIA(check());
    assert((size_t)index < _counters.size());
    _counters[index].count++;
    POOL_PARANOIA(check());
    return _counters[index].count;
}

int32_t
cref_counter_pool::decr_counter(int32_t index)
{
    POOL_PARANOIA(assert((size_t)index < _counters.size()));
    int32_t c = --_counters[index].count;
    if (c == 0) {
	POOL_PARANOIA(check());
	/* recycle */
	_counters[index].count = _free_index;
	_free_index = index;
	VERBOSE_POOL_PARANOIA(dump());
	POOL_PARANOIA(check());
    }
    assert(c >= 0);
    return c;
}

int32_t
cref_counter_pool::count(int32_t index)
{
    POOL_PARANOIA(assert((size_t)index < _counters.size()));
    return _counters[index].count;
}

void*
cref_counter_pool::data(int32_t index)
{
    POOL_PARANOIA(assert((size_t)index < _counters.size()));
    return _counters[index].data;
}
