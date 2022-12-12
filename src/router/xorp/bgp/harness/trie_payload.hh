// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-

// Copyright (c) 2001-2009 XORP, Inc.
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

// $XORP: xorp/bgp/harness/trie_payload.hh,v 1.15 2008/11/08 06:14:45 mjh Exp $

#ifndef __BGP_HARNESS_TRIE_PAYLOAD_HH__
#define __BGP_HARNESS_TRIE_PAYLOAD_HH__

/**
 * A BGP update packet can have many NLRIs. Each NLRI is stored in a
 * trie node. Rather than keep multiple copies of a BGP update
 * packet. A single reference counted copy is kept in TrieData. A
 * TriePayload is stored in the trie and holds a pointer to the TrieData.
 */
class TrieData {
public:
    TrieData(const TimeVal& tv, const uint8_t *buf, size_t len,
	     const BGPPeerData *peerdata,
	     TrieData* &first, TrieData* &last) : _tv(tv),
						  _first(first),
						  _last(last) {
	_packet = new UpdatePacket(buf, len, peerdata, 0, false);
	_refcnt = 1;

	_next = 0;
	_prev = _last;		

	if(0 == _first)
	    _first = this;
	if(0 != _last) 
	    _last->_next = this;
	_last = this;
    }

    void incr_refcnt() {
	_refcnt++;
    }

    bool decr_refcnt() {
	_refcnt--;

	XLOG_ASSERT(_refcnt >= 0);

	return 0 == _refcnt;
    }

    const UpdatePacket *data() const {
	XLOG_ASSERT(_refcnt > 0);

	return _packet;
    }

    const TimeVal& tv() const {
	return _tv;
    }
    
    const TrieData* next() const {
	return _next;
    }

    ~TrieData() {
	XLOG_ASSERT(0 == _refcnt);

	if(this == _first)
	    _first = _next;
	else
	    _prev->_next = _next;
	if(this == _last)
	    _last = _prev;
	if(0 != _next) {
	    _next->_prev = _prev;
	}
// 	debug_msg("Deleting: %s\n", _packet->str().c_str());
	delete _packet;
    }

private:
    TimeVal _tv;
    TrieData* &_first;
    TrieData* &_last;

    int _refcnt;
    UpdatePacket *_packet;
    TrieData *_next;
    TrieData *_prev;
};

/**
 * The payload of a RealTrie.
 */
class TriePayload {
public:
    TriePayload() : _data(0) {}

    TriePayload(const TimeVal& tv, const uint8_t *buf, size_t len,
		const BGPPeerData *peerdata,
		TrieData* &first, TrieData* &last) {
	_data = new TrieData(tv, buf, len, peerdata, first, last);
    }

    ~TriePayload() {
	zap();
    }

    TriePayload(const TriePayload& rhs) {
	_data = 0;
	copy(rhs);
    }

    TriePayload& operator=(const TriePayload& rhs) {
	if(&rhs == this)
	    return *this;
	zap();
	copy(rhs);
	    
	return *this;
    }

    void copy(const TriePayload& rhs) {
	if(rhs._data) {
// 	    debug_msg("refcnt: %d %#x\n", rhs._data->_refcnt + 1, rhs._data);
	    rhs._data->incr_refcnt();
	    _data = rhs._data;
	}
    }

    const UpdatePacket *get() const {
	if(0 == _data)
	    return 0;
	return _data->data();
    }

    const UpdatePacket *get(TimeVal& tv) const {
	if(0 == _data)
	    return 0;
	tv = _data->tv();
	return _data->data();
    }

    void zap() {
// 	if(_data)
// 	    debug_msg("refcnt: %d %#x\n", _data->_refcnt - 1, _data);
	if(_data && _data->decr_refcnt()) {
	    delete _data;
	}
	_data = 0;
    }

private:
    TrieData *_data;
};

#endif // __BGP_HARNESS_TRIE_PAYLOAD_HH__
