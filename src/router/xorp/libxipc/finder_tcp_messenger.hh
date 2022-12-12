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

// $XORP: xorp/libxipc/finder_tcp_messenger.hh,v 1.23 2008/10/02 21:57:21 bms Exp $

#ifndef __LIBXIPC_FINDER_TCP_MESSENGER_HH__
#define __LIBXIPC_FINDER_TCP_MESSENGER_HH__



#include "libxorp/ref_ptr.hh"

#include "finder_tcp.hh"
#include "finder_msgs.hh"
#include "finder_messenger.hh"

class FinderTcpMessenger
    : public FinderMessengerBase, protected FinderTcpBase
{
public:
    FinderTcpMessenger(EventLoop&		e,
		       FinderMessengerManager*	mm,
		       XorpFd			sock,
		       XrlCmdMap&		cmds);

    virtual ~FinderTcpMessenger();

    bool send(const Xrl& xrl, const SendCallback& scb);

    bool pending() const;

    void close()	{ FinderTcpBase::close(); }
    
protected:
    // FinderTcpBase methods
    bool read_event(int		   errval,
		    const uint8_t* data,
		    uint32_t	   data_bytes);

    void write_event(int	    errval,
		     const uint8_t* data,
		     uint32_t	    data_bytes);

    void close_event();

    void error_event();
    
protected:
    void reply(uint32_t seqno, const XrlError& xe, const XrlArgs* reply_args);

protected:
    void push_queue();
    void drain_queue();
    
    /* High water-mark to disable reads, ie reading faster than writing */
    static const uint32_t OUTQUEUE_BLOCK_READ_HI_MARK = 6;

    /* Low water-mark to enable reads again */
    static const uint32_t OUTQUEUE_BLOCK_READ_LO_MARK = 4;    

    typedef list<const FinderMessageBase*> OutputQueue;
    OutputQueue _out_queue;
};

/**
 * Class that creates FinderMessengers for incoming connections.
 */
class FinderTcpListener : public FinderTcpListenerBase {
public:
    typedef FinderTcpListenerBase::AddrList Addr4List;
    typedef FinderTcpListenerBase::NetList Net4List;

    FinderTcpListener(EventLoop& e,
		      FinderMessengerManager& mm,
		      XrlCmdMap& cmds,
		      IPv4 iface,
		      uint16_t port,
		      bool enabled = true)
	throw (InvalidAddress, InvalidPort);

    virtual ~FinderTcpListener();

    /**
     * Instantiate a Messenger instance for sock.
     * @return true on success, false on failure.
     */
    bool connection_event(XorpFd sock);

protected:
    FinderMessengerManager& _mm;
    XrlCmdMap& _cmds;
};

class FinderTcpConnector {
public:
    FinderTcpConnector(EventLoop&		e,
		       FinderMessengerManager&	mm,
		       XrlCmdMap&		cmds,
		       IPv4			host,
		       uint16_t			port);
    virtual ~FinderTcpConnector();

    /**
     * Connect to host specified in constructor.
     *
     * @param created_messenger pointer to be assigned messenger created upon
     * successful connect.
     * @return 0 on success, errno on failure.
     */
    int connect(FinderTcpMessenger*& created_messenger);

    IPv4 finder_address() const;
    uint16_t finder_port() const;
    
protected:
    EventLoop&		    _e;
    FinderMessengerManager& _mm;
    XrlCmdMap&		    _cmds;
    IPv4		    _host;
    uint16_t		    _port;
};

/**
 * Class to establish and manage a single connection to a FinderTcpListener.
 * Should the connection fail after being established a new connection is
 * started.
 */
class FinderTcpAutoConnector
    : public FinderTcpConnector, public FinderMessengerManager
{
public:
    FinderTcpAutoConnector(
			   EventLoop&		     	e,
			   FinderMessengerManager& 	mm,
			   XrlCmdMap&		     	cmds,
			   IPv4		     		host,
			   uint16_t		     	port,
			   bool		     		enabled		= true,
			   uint32_t			give_up_ms	= 0
			   );
    virtual ~FinderTcpAutoConnector();

    void set_enabled(bool en);
    bool enabled() const;
    bool connected() const;
    bool connect_failed() const;

protected:
    void do_auto_connect();
    void start_timer(uint32_t ms = 0);
    void stop_timer();

protected:
    /*
     * Implement FinderMessengerManager interface to catch death of
     * active messenger to trigger auto-reconnect.  All methods are
     * forwarded to _real_manager.
     */
    void messenger_birth_event(FinderMessengerBase*);
    void messenger_death_event(FinderMessengerBase*);
    void messenger_active_event(FinderMessengerBase*);
    void messenger_inactive_event(FinderMessengerBase*);
    void messenger_stopped_event(FinderMessengerBase*);
    bool manages(const FinderMessengerBase*) const;

protected:
    FinderMessengerManager& _real_manager;
    bool		    _connected;
    bool		    _connect_failed;
    bool		    _enabled;
    bool		    _once_active;
    XorpTimer		    _retry_timer;
    XorpTimer		    _giveup_timer;
    int			    _last_error;
    size_t		    _consec_error;

    static const uint32_t CONNECT_RETRY_PAUSE_MS = 100;
    static const uint32_t CONNECT_FAILS_BEFORE_LOGGING = 10;
};

#endif // __LIBXIPC_FINDER_TCP_MESSENGER_HH__
