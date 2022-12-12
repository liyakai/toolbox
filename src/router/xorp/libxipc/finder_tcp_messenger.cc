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



#include "finder_module.h"

#include "libxorp/xorp.h"
#include "libxorp/xlog.h"
#include "libxorp/debug.h"

#include "libcomm/comm_api.h"

#include "finder_tcp_messenger.hh"


FinderTcpMessenger::FinderTcpMessenger(EventLoop&		e,
				       FinderMessengerManager*	mm,
				       XorpFd			sock,
				       XrlCmdMap&		cmds)
    : FinderMessengerBase(e, mm, cmds), FinderTcpBase(e, sock)
{
    if (manager())
	manager()->messenger_birth_event(this);
}

FinderTcpMessenger::~FinderTcpMessenger()
{
    if (manager())
	manager()->messenger_death_event(this);
    drain_queue();
}

bool
FinderTcpMessenger::read_event(int	      errval,
			       const uint8_t* data,
			       uint32_t	      data_bytes)
{
    if (errval != 0) {
	/* An error has occurred, the FinderTcpBase class will close
	 * connection following this notification.
	 */
	debug_msg("Got errval %d, data %p, data_bytes %u\n",
		  errval, data, XORP_UINT_CAST(data_bytes));
	return true;
    }

    // TODO:  This doesn't seem to conform to STL, not sure how it worked before.
    //string s(data, data + data_bytes);
    // Going to assume second arg should just be data_bytes (length)
    // Needs some actual testing! --Ben
    string s((const char*)(data), data_bytes);

    string ex;
    try {
	try {
	    ParsedFinderXrlMessage fm(s.c_str());
	    dispatch_xrl(fm.seqno(), fm.xrl());
	    return true;
	} catch (const WrongFinderMessageType&) {
	    ParsedFinderXrlResponse fm(s.c_str());
	    dispatch_xrl_response(fm.seqno(), fm.xrl_error(), fm.xrl_args());
	    return true;
	}
    } catch (const InvalidString& e) {
	ex = e.str();
    } catch (const BadFinderMessageFormat& e) {
	ex = e.str();
    } catch (const WrongFinderMessageType& e) {
	ex = e.str();
    } catch (const XorpException& e) {
	ex = e.str();
    } catch (...) {
	ex = "Unexpected ?";
    }
    XLOG_ERROR("Got exception %s, closing connection", ex.c_str());
    close();
    return false;
}

bool
FinderTcpMessenger::send(const Xrl& xrl, const SendCallback& scb)
{
#if 0
    if (pending()) {
	XLOG_FATAL("Hit pending");
	return false;
    }
#endif
    FinderXrlMessage* msg = new FinderXrlMessage(xrl);

    if (store_xrl_response(msg->seqno(), scb) == false) {
	XLOG_ERROR("Could not store xrl response\n");
	delete msg;
	return false;
    }

    if (_out_queue.empty()) {
	_out_queue.push_back(msg);
	push_queue();
    } else {
	_out_queue.push_back(msg);
    }

    return true;
}

bool
FinderTcpMessenger::pending() const
{
    return (false == _out_queue.empty());
}

void
FinderTcpMessenger::reply(uint32_t	  seqno,
			  const XrlError& xe,
			  const XrlArgs*  args)
{
    FinderXrlResponse* msg = new FinderXrlResponse(seqno, xe, args);
    if (_out_queue.empty()) {
	_out_queue.push_back(msg);
	push_queue();
    } else {
	_out_queue.push_back(msg);
    }
}

inline static const uint8_t*
get_data(const FinderMessageBase& fm)
{
    const char* p = fm.str().c_str();
    return reinterpret_cast<const uint8_t*>(p);
}

inline static uint32_t
get_data_bytes(const FinderMessageBase& fm)
{
    return fm.str().size();
}

void
FinderTcpMessenger::push_queue()
{
    XLOG_ASSERT(false == _out_queue.empty());
    const FinderMessageBase* fm = _out_queue.front();

    assert(0 != fm);
    write_data(get_data(*fm), get_data_bytes(*fm));

    /*
     * Block read queue if output queue has grown too large.  This
     * stops new requests and responses and is intended to allow output
     * queue to drain.
     */
    const size_t qs = _out_queue.size();
    if (qs >= OUTQUEUE_BLOCK_READ_HI_MARK && true == read_enabled()) {
	set_read_enabled(false);
	XLOG_WARNING("Blocking input queue, output queue hi water mark "
		     "reached.");
    } else if (qs == OUTQUEUE_BLOCK_READ_LO_MARK && false == read_enabled()) {
	set_read_enabled(true);
	XLOG_WARNING("Unblocking input queue, output queue lo water mark "
		     "reached.");
    }
}

void
FinderTcpMessenger::drain_queue()
{
    while (false == _out_queue.empty()) {
	delete _out_queue.front();
	_out_queue.pop_front();
    }
}

void
FinderTcpMessenger::write_event(int 		errval,
				const uint8_t*	data,
				uint32_t	data_bytes)
{
    XLOG_ASSERT(false == _out_queue.empty());
    if (errval != 0) {
	/* Tcp connection will be closed shortly */
	return;
    }
    assert(data == get_data(*_out_queue.front()));
    assert(data_bytes == get_data_bytes(*_out_queue.front()));
    delete _out_queue.front();
    _out_queue.pop_front();
    if (false == _out_queue.empty())
	push_queue();
}

void
FinderTcpMessenger::close_event()
{
    if (manager())
	manager()->messenger_stopped_event(this);
}

void
FinderTcpMessenger::error_event()
{
    delete this;
}

///////////////////////////////////////////////////////////////////////////////
//
// FinderTcpListener methods
//

FinderTcpListener::FinderTcpListener(EventLoop&		     e,
				     FinderMessengerManager& mm,
				     XrlCmdMap&		     cmds,
				     IPv4		     interface,
				     uint16_t		     port,
				     bool		     en)
    throw (InvalidAddress, InvalidPort)
    : FinderTcpListenerBase(e, interface, port, en), _mm(mm), _cmds(cmds)
{
}

FinderTcpListener::~FinderTcpListener()
{
}

bool
FinderTcpListener::connection_event(XorpFd sock)
{
    FinderTcpMessenger* m =
	new FinderTcpMessenger(eventloop(), &_mm, sock, _cmds);
    // Check if manager has taken responsibility for messenger and clean up if
    // not.
    if (_mm.manages(m) == false)
	delete m;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// FinderTcpConnector methods
//

FinderTcpConnector::FinderTcpConnector(EventLoop&		e,
				       FinderMessengerManager&	mm,
				       XrlCmdMap&		cmds,
				       IPv4			host,
				       uint16_t 		port)
    : _e(e), _mm(mm), _cmds(cmds), _host(host), _port(port)
{}

FinderTcpConnector::~FinderTcpConnector()
{
}

int
FinderTcpConnector::connect(FinderTcpMessenger*& created_messenger)
{
    struct in_addr host_ia;
    host_ia.s_addr = _host.addr();

    int in_progress = 0;
    XorpFd sock = comm_connect_tcp4(&host_ia, htons(_port),
				    COMM_SOCK_NONBLOCKING, &in_progress, NULL);
    if (!sock.is_valid()) {
	created_messenger = 0;
	int last_error = comm_get_last_error();
	XLOG_ASSERT(0 != last_error);
	return last_error;
    }

    created_messenger = new FinderTcpMessenger(_e, &_mm, sock, _cmds);
    debug_msg("Created messenger %p\n", created_messenger);
    return 0;
}

IPv4
FinderTcpConnector::finder_address() const
{
    return _host;
}

uint16_t
FinderTcpConnector::finder_port() const
{
    return _port;
}

///////////////////////////////////////////////////////////////////////////////
//
// FinderTcpAutoConnector methods
//

FinderTcpAutoConnector::FinderTcpAutoConnector(
				EventLoop&		e,
				FinderMessengerManager& real_manager,
				XrlCmdMap&		cmds,
				IPv4			host,
				uint16_t		port,
				bool			en,
				uint32_t 		give_up_ms
				)
    : FinderTcpConnector(e, *this, cmds, host, port),
      _real_manager(real_manager), _connected(false), _connect_failed(false),
      _enabled(en), _once_active(false), _last_error(0), _consec_error(0)
{
    if (en) {
	start_timer();
	if (give_up_ms) {
	    _giveup_timer =
		e.new_oneoff_after_ms(give_up_ms,
			callback(this,
				 &FinderTcpAutoConnector::set_enabled, false));
	}
    }
}

FinderTcpAutoConnector::~FinderTcpAutoConnector()
{
    // Any existing messenger will die and we need to not restart the timer
    set_enabled(false);
}

void
FinderTcpAutoConnector::set_enabled(bool en)
{
    if (_enabled == en) {
	return;
    }

    _enabled = en;
    if (_connected) {
	// timer better not be running since we're connected
	XLOG_ASSERT(false == _retry_timer.scheduled());
	return;
    }

    if (false == _enabled) {
	stop_timer();
    } else {
	start_timer();
    }
}

bool
FinderTcpAutoConnector::enabled() const
{
    return _enabled;
}

bool
FinderTcpAutoConnector::connected() const
{
    return _connected;
}

bool
FinderTcpAutoConnector::connect_failed() const
{
    return _connect_failed;
}

void
FinderTcpAutoConnector::do_auto_connect()
{
    XLOG_ASSERT(false == _connected);

    _connect_failed = false;

    FinderTcpMessenger* fm;
    int r = connect(fm);
    if (r == 0) {
	XLOG_ASSERT(fm != 0);
	_consec_error = 0;
	_connected = true;
    } else {
	XLOG_ASSERT(fm == 0);
	_connect_failed = true;
	if (r != _last_error) {
	    XLOG_ERROR("Failed to connect to %s/%u: %s",
		       _host.str().c_str(), _port, strerror(r));
	    _consec_error = 0;
	    _last_error = r;
	} else if ((++_consec_error % CONNECT_FAILS_BEFORE_LOGGING) == 0) {
	    XLOG_ERROR("Failed %u times to connect to %s/%u: %s",
		       XORP_UINT_CAST(_consec_error), _host.str().c_str(), 
		       XORP_UINT_CAST(_port),
		       strerror(r));
	    _consec_error = 0;
	}
	_connected = false;
	start_timer(CONNECT_RETRY_PAUSE_MS);
    }
    _last_error = r;
}

void
FinderTcpAutoConnector::start_timer(uint32_t ms)
{
    XLOG_ASSERT(false == _retry_timer.scheduled());
    XLOG_ASSERT(false == _connected);
    _retry_timer =
	_e.new_oneoff_after_ms(
	    ms, callback(this, &FinderTcpAutoConnector::do_auto_connect));
}

void
FinderTcpAutoConnector::stop_timer()
{
    _retry_timer.unschedule();
}

void
FinderTcpAutoConnector::messenger_birth_event(FinderMessengerBase* m)
{
    _real_manager.messenger_birth_event(m);
    //    set_enabled(false);
}

void
FinderTcpAutoConnector::messenger_death_event(FinderMessengerBase* m)
{
    _real_manager.messenger_death_event(m);
    _connected = false;
    if (_enabled && _once_active == false)
	start_timer(CONNECT_RETRY_PAUSE_MS);
}

void
FinderTcpAutoConnector::messenger_active_event(FinderMessengerBase* m)
{
    _real_manager.messenger_active_event(m);
    _once_active = true;
}

void
FinderTcpAutoConnector::messenger_inactive_event(FinderMessengerBase* m)
{
    _real_manager.messenger_inactive_event(m);
}

void
FinderTcpAutoConnector::messenger_stopped_event(FinderMessengerBase* m)
{
    _real_manager.messenger_stopped_event(m);
}

bool
FinderTcpAutoConnector::manages(const FinderMessengerBase* m) const
{
    return _real_manager.manages(m);
}

