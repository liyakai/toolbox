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

// $XORP: xorp/ospf/auth.hh,v 1.16 2008/11/21 00:07:55 atanu Exp $

#ifndef __OSPF_AUTH_HH__
#define __OSPF_AUTH_HH__

#include <openssl/md5.h>



class EventLoop;

/**
 * @short Base clase for OSPFv2 authentication mechanisms.
 *
 * The AuthHandlerBase class defines the interfaces for OSPFv2
 * authentication handlers.  Handlers are responsible for
 * authenticating inbound datagrams and adding authentication data to
 * outbound datagrams.
 *
 * Error during authentication set an error buffer that clients may
 * query using the error() method.
 */
class AuthHandlerBase {
 public:
    virtual ~AuthHandlerBase();

    /**
     * Get the effective name of the authentication scheme.
     *
     * @return the name of the authentication scheme.
     */
    virtual const char* effective_name() const = 0;

    /**
     * Reset the authentication state.
     */
    virtual void reset() = 0;

    /**
     * Additional bytes that will be added to the payload.
     *
     * @return the number of additional bytes that need to be added to
     * the payload.
     */
    virtual uint32_t additional_payload() const = 0;

    /**
     * Inbound authentication method.
     *
     * @param packet the packet to verify.
     * @param src_addr the source address of the packet.
     * @param new_peer true if this is a new peer.
     * @return true if packet passes authentication checks, false otherwise.
     */
    virtual bool authenticate_inbound(const vector<uint8_t>&	packet,
				      const IPv4&		src_addr,
				      bool			new_peer) = 0;

    /**
     * Outbound authentication method.
     *
     * @param packet the packet to authenticate.
     * @return true if packet was successfully authenticated, false when
     * no valid keys are present.
     */
    virtual bool authenticate_outbound(vector<uint8_t>& packet) = 0;

    /**
     * Get textual description of last error.
     */
    const string& error() const;

protected:
    /**
     * Reset textual description of last error.
     */
    void reset_error();

    /**
     * Set textual description of latest error.
     */
    void set_error(const string& error_msg);

private:
    string _error;
};

/**
 * @short OSPFv2 Authentication handler when no authentication scheme is
 * employed.
 */
class NullAuthHandler : public AuthHandlerBase {
public:
    static const OspfTypes::AuType AUTH_TYPE = OspfTypes::NULL_AUTHENTICATION;

    /**
     * Get the effective name of the authentication scheme.
     *
     * @return the name of the authentication scheme.
     */
    const char* effective_name() const;

    /**
     * Get the method-specific name of the authentication scheme.
     *
     * @return the method-specific name of the authentication scheme.
     */
    static const char* auth_type_name();

    /**
     * Reset the authentication state.
     */
    void reset();

    /**
     * Additional bytes that will be added to the payload.
     *
     * @return the number of additional bytes that need to be added to
     * the payload.
     */
    uint32_t additional_payload() const;

    /**
     * Inbound authentication method.
     *
     * @param packet the packet to verify.
     * @param src_addr the source address of the packet.
     * @param new_peer true if this is a new peer.
     * @return true if packet passes authentication checks, false otherwise.
     */
    bool authenticate_inbound(const vector<uint8_t>&	packet,
			      const IPv4&		src_addr,
			      bool			new_peer);

    /**
     * Outbound authentication method.
     *
     * @param packet the packet to authenticate.
     * @return true if packet was successfully authenticated, false when
     * no valid keys are present.
     */
    bool authenticate_outbound(vector<uint8_t>& packet);
};

/**
 * @short OSPFv2 Authentication handler for plaintext scheme.
 */
class PlaintextAuthHandler : public AuthHandlerBase {
 public:
    static const OspfTypes::AuType AUTH_TYPE = OspfTypes::SIMPLE_PASSWORD;

    /**
     * Get the effective name of the authentication scheme.
     *
     * @return the name of the authentication scheme.
     */
    const char* effective_name() const;

    /**
     * Get the method-specific name of the authentication scheme.
     *
     * @return the method-specific name of the authentication scheme.
     */
    static const char* auth_type_name();

    /**
     * Reset the authentication state.
     */
    void reset();

    /**
     * Additional bytes that will be added to the payload.
     *
     * @return the number of additional bytes that need to be added to
     * the payload.
     */
    uint32_t additional_payload() const;

    /**
     * Inbound authentication method.
     *
     * @param packet the packet to verify.
     * @param src_addr the source address of the packet.
     * @param new_peer true if this is a new peer.
     * @return true if packet passes authentication checks, false otherwise.
     */
    bool authenticate_inbound(const vector<uint8_t>&	packet,
			      const IPv4&		src_addr,
			      bool			new_peer);

    /**
     * Outbound authentication method.
     *
     * @param packet the packet to authenticate.
     * @return true if packet was successfully authenticated, false when
     * no valid keys are present.
     */
    bool authenticate_outbound(vector<uint8_t>& packet);

    /**
     * Get the authentication key.
     *
     * @return the authentication key.
     */
    const string& key() const;

    /**
     * Set the authentication key.
     *
     * @param plaintext_key the plain-text key.
     */
    void set_key(const string& plaintext_key);

 private:
    string	_key;
    uint8_t	_key_data[Packet::AUTH_PAYLOAD_SIZE];
};

/**
 * @short OSPFv2 Authentication handler for MD5 scheme.
 *
 * Class to check inbound MD5 authenticated packets and add
 * authentication data to outbound OSPF packets. The OSPFv2 MD5
 * authentication scheme is described in Section D.3 of RFC 2328.
 */
class MD5AuthHandler : public AuthHandlerBase {
public:
    static const OspfTypes::AuType AUTH_TYPE =
	OspfTypes::CRYPTOGRAPHIC_AUTHENTICATION;

    /**
     * Class to hold MD5 key information.
     */
    class MD5Key {
    public:
	/**
	 * Construct an MD5 Key.
	 *
	 * @param key_id unique ID associated with key.
	 * @param key phrase used for MD5 digest computation.
	 * @param start_timeval start time when key becomes valid.
	 * @param end_timeval end time when key becomes invalid.
	 * @param max_time_drift the maximum time drift among all routers.
	 * @param start_timer the timer to mark when the key becomes valid.
	 * @param end_timer the timer to mark when the key becomes invalid.
	 */
	MD5Key(uint8_t		key_id,
	       const string&	key,
	       const TimeVal&	start_timeval,
	       const TimeVal&	end_timeval,
	       const TimeVal&	max_time_drift,
	       XorpTimer	start_timer,
	       XorpTimer	end_timer);

	/**
	 * Get the ID associated with the key.
	 */
	uint8_t	id() const			{ return _id; }

	/**
	 * Get pointer to key data.  The data is of size KEY_BYTES.
	 */
	const char*	key_data() const	{ return _key_data; }

	/**
	 * Get the size of the key data in bytes.
	 */
	uint32_t	key_data_bytes() const	{ return KEY_BYTES; }

	/**
	 * Get key data as a string.
	 */
	string	 	key() const;

	/**
	 * Get the start time of the key.
	 */
	const TimeVal&	start_timeval() const	{ return _start_timeval; }

	/**
	 * Get the end time of the key.
	 */
	const TimeVal&	end_timeval() const	{ return _end_timeval; }

	/**
	 * Get the maximum time drift among all routers.
	 */
	const TimeVal&	max_time_drift() const	{ return _max_time_drift; }

	/**
	 * Get indication of whether key is persistent.
	 */
	bool		is_persistent() const	{ return _is_persistent; }

	/**
	 * Set the flag whether the key is persistent.
	 *
	 * @param v if true the key is persistent.
	 */
	void		set_persistent(bool v)	{ _is_persistent = v; }

	/**
	 * Get whether ID matches a particular value (convenient for STL
	 * algorithms).
	 */
	bool		id_matches(uint8_t o) const	{ return _id == o; }

	/**
	 * Get key validity status of key at a particular time.
	 *
	 * @param when the time to test whether the key is valid.
	 */
	bool		valid_at(const TimeVal& when) const;

	/**
	 * Reset the key for all sources.
	 */
	void		reset();

	/**
	 * Reset the key for a particular source.
	 *
	 * @param src_addr the source address.
	 */
	void		reset(const IPv4& src_addr);

	/**
	 * Indicate whether valid packets have been received from a source
	 * with this key ID.
	 *
	 * @param src_addr the source address.
	 * @return true if a packet has been received from the source,
	 * otherwise false.
	 */
	bool		packets_received(const IPv4& src_addr) const;

	/**
	 * Get last received sequence number from a source.
	 *
	 * @param src_addr the source address.
	 * @return last sequence number seen from the source. Value may be
	 * garbage if no packets have been received (check first with
	 * @ref packets_received()).
	 */
	uint32_t	last_seqno_recv(const IPv4& src_addr) const;

	/**
	 * Set last sequence number received from a source. This method
	 * implicitly set packets received to true.
	 *
	 * @param src_addr the source address.
	 * @param seqno the last sequence number received from the source.
	 */
	void		set_last_seqno_recv(const IPv4& src_addr,
					    uint32_t seqno);

	/**
	 * Get next sequence number for outbound packets.  The counter
	 * is automatically updated with each call of this method.
	 */
	uint32_t next_seqno_out()		{ return _o_seqno++; }

    protected:
	static const uint32_t KEY_BYTES = 16;

	uint8_t		_id;		// Key ID
	char		_key_data[KEY_BYTES]; // Key data
	TimeVal		_start_timeval;	// Start time of the key
	TimeVal		_end_timeval;	// End time of the key
	TimeVal		_max_time_drift; // Max. time drift among all routers
	bool		_is_persistent;	// True if key is persistent
	map<IPv4, bool>	_pkts_recv;	// True if packets received
	map<IPv4, uint32_t> _lr_seqno;	// Last received seqno
	uint32_t	_o_seqno;	// Next outbound sequence number
	XorpTimer	_start_timer;	// Key start timer
	XorpTimer	_stop_timer;	// Key stop timer

	friend class MD5AuthHandler;
    };

    typedef list<MD5Key> KeyChain;

public:
    /**
     * Constructor
     *
     * @param eventloop the EventLoop instance to used for time reference.
     */
    MD5AuthHandler(EventLoop& eventloop);

    /**
     * Get the effective name of the authentication scheme.
     *
     * @return the name of the authentication scheme.
     */
    const char* effective_name() const;

    /**
     * Get the method-specific name of the authentication scheme.
     *
     * @return the method-specific name of the authentication scheme.
     */
    static const char* auth_type_name();

    /**
     * Reset the authentication state.
     */
    void reset();

    /**
     * Additional bytes that will be added to the payload.
     *
     * @return the number of additional bytes that need to be added to
     * the payload.
     */
    uint32_t additional_payload() const;

    /**
     * Inbound authentication method.
     *
     * @param packet the packet to verify.
     * @param src_addr the source address of the packet.
     * @param new_peer true if this is a new peer.
     * @return true if packet passes authentication checks, false otherwise.
     */
    bool authenticate_inbound(const vector<uint8_t>&	packet,
			      const IPv4&		src_addr,
			      bool			new_peer);

    /**
     * Outbound authentication method.
     *
     * @param packet the packet to authenticate.
     * @return true if packet was successfully authenticated, false when
     * no valid keys are present.
     */
    bool authenticate_outbound(vector<uint8_t>& packet);

    /**
     * Add a key to the MD5 key chain.
     *
     * If the key already exists, it is updated with the new settings.
     *
     * @param key_id unique ID associated with key.
     * @param key phrase used for MD5 digest computation.
     * @param start_timeval start time when key becomes valid.
     * @param end_timeval end time when key becomes invalid.
     * @param max_time_drift the maximum time drift among all routers.
     * @param error_msg the error message (if error).
     * @return true on success, false if end time is less than start time
     * or key has already expired.
     */
    bool add_key(uint8_t	key_id,
		 const string&	key,
		 const TimeVal&	start_timeval,
		 const TimeVal&	end_timeval,
		 const TimeVal&	max_time_drift,
		 string&	error_msg);

    /**
     * Remove a key from the MD5 key chain.
     *
     * @param key_id unique ID of key to be removed.
     * @param error_msg the error message (if error).
     * @return true if the key was found and removed, otherwise false.
     */
    bool remove_key(uint8_t key_id, string& error_msg);

    /**
     * A callback that a key from the MD5 key chain has become valid.
     *
     * @param key_id unique ID of the key that has become valid.
     */
    void key_start_cb(uint8_t key_id);

    /**
     * A callback that a key from the MD5 key chain has expired and is invalid.
     *
     * @param key_id unique ID of the key that has expired.
     */
    void key_stop_cb(uint8_t key_id);

    /**
     * Select the best key for outbound messages.
     *
     * The chosen key is the one with most recent start-time in the past.
     * If there is more than one key that matches the criteria, then select
     * the key with greatest ID.
     *
     * @param now current time.
     */
    MD5Key* best_outbound_key(const TimeVal& now);

    /**
     * Reset the keys for all sources.
     */
    void reset_keys();

    /**
     * Get all valid keys managed by the MD5AuthHandler.
     *
     * @return list of all valid keys.
     */
    const KeyChain& valid_key_chain() const	{ return _valid_key_chain; }

    /**
     * Get all invalid keys managed by the MD5AuthHandler.
     *
     * @return list of all invalid keys.
     */
    const KeyChain& invalid_key_chain() const	{ return _invalid_key_chain; }

    /**
     * Test where the MD5AuthHandler contains any keys.
     *
     * @return if the MD5AuthHandler contains any keys, otherwise false.
     */
    bool empty() const;

protected:
    EventLoop&	_eventloop;		// The event loop
    KeyChain	_valid_key_chain;	// The set of all valid keys
    KeyChain	_invalid_key_chain;	// The set of all invalid keys
    NullAuthHandler _null_handler;	// Null handler if no valid keys
};

/**
 * This is the class that should be instantiated to access
 * authentication.
 */
class Auth {
 public:
    Auth(EventLoop& eventloop) : _eventloop(eventloop), _auth_handler(NULL)
    {
	set_method("none");
    }

    ~Auth() {
	if (_auth_handler != NULL) {
	    delete _auth_handler;
	    _auth_handler = NULL;
	}
    }

    bool set_method(const string& method) {
	if (_auth_handler != NULL) {
	    delete _auth_handler;
	    _auth_handler = NULL;
	}

	if ("none" == method) {
	    _auth_handler = new NullAuthHandler;
	    return true;
	}

	if ("simple" == method) {
	    _auth_handler = new PlaintextAuthHandler;
	    return true;
	}

	if ("md5" == method) {
	    _auth_handler = new MD5AuthHandler(_eventloop);
	    return true;
	}

	// Never allow _auth to be zero.
	set_method("none");

	return false;
    }

    /**
     * Apply the authentication scheme to the packet.
     */
    void generate(vector<uint8_t>& pkt) {
	XLOG_ASSERT(_auth_handler != NULL);
	_auth_handler->authenticate_outbound(pkt);
    }

    /**
     * Verify that this packet has passed the authentication scheme.
     */
    bool verify(vector<uint8_t>& pkt, const IPv4& src_addr, bool new_peer) {
	XLOG_ASSERT(_auth_handler != NULL);
	return _auth_handler->authenticate_inbound(pkt, src_addr, new_peer);
    }

    bool verify(vector<uint8_t>& pkt, const IPv6& src_addr, bool new_peer) {
	UNUSED(pkt);
	UNUSED(src_addr);
	UNUSED(new_peer);
	return true;
    }

    /**
     * Additional bytes that will be added to the payload.
     */
    uint32_t additional_payload() const {
	XLOG_ASSERT(_auth_handler != NULL);
	return _auth_handler->additional_payload();
    }

    const string& error() const {
	XLOG_ASSERT(_auth_handler != NULL);
	return _auth_handler->error();
    }

    /**
     * Called to notify authentication system to reset.
     */
    void reset() {
	XLOG_ASSERT(_auth_handler != NULL);
	_auth_handler->reset();
    }

    /**
     * Set a simple password authentication key.
     *
     * Note that the current authentication handler is replaced with
     * a simple password authentication handler.
     *
     * @param password the password to set.
     * @param the error message (if error).
     * @return true on success, otherwise false.
     */
    bool set_simple_authentication_key(const string& password,
				       string& error_msg);

    /**
     * Delete a simple password authentication key.
     *
     * Note that after the deletion the simple password authentication
     * handler is replaced with a Null authentication handler.
     *
     * @param the error message (if error).
     * @return true on success, otherwise false.
     */
    bool delete_simple_authentication_key(string& error_msg);

    /**
     * Set an MD5 authentication key.
     *
     * Note that the current authentication handler is replaced with
     * an MD5 authentication handler.
     *
     * @param key_id unique ID associated with key.
     * @param password phrase used for MD5 digest computation.
     * @param start_timeval start time when key becomes valid.
     * @param end_timeval end time when key becomes invalid.
     * @param max_time_drift the maximum time drift among all routers.
     * @param the error message (if error).
     * @return true on success, otherwise false.
     */
    bool set_md5_authentication_key(uint8_t		key_id,
				    const string&	password,
				    const TimeVal&	start_timeval,
				    const TimeVal&	end_timeval,
				    const TimeVal&	max_time_drift,
				    string&		error_msg);

    /**
     * Delete an MD5 authentication key.
     *
     * Note that after the deletion if there are no more valid MD5 keys,
     * the MD5 authentication handler is replaced with a Null authentication
     * handler.
     *
     * @param key_id the ID of the key to delete.
     * @param the error message (if error).
     * @return true on success, otherwise false.
     */
    bool delete_md5_authentication_key(uint8_t key_id, string& error_msg);

 private:
    EventLoop&		_eventloop;		// The event loop
    AuthHandlerBase*	_auth_handler;		// The authentication handler
};

#endif // __OSPF_AUTH_HH__
