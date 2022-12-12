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

#ifndef __LIBXORP_BUFFERED_ASYNCIO_HH__
#define __LIBXORP_BUFFERED_ASYNCIO_HH__

#include "libxorp/xorp.h"
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include "libxorp/callback.hh"
#include "libxorp/eventloop.hh"

class BufferedAsyncReader :
    public NONCOPYABLE
{
public:
    enum Event {
	DATA 		  = 1,
	OS_ERROR 	  = 2,
	END_OF_FILE 	  = 3
    };

    /*
     * Callback has arguments:
     *		BufferedAsyncReader*	caller,
     * 		ErrorCode		e,
     *		uint8_t*		buffer,
     * 		size_t 			buffer_bytes,
     *          size_t 			offset
     *
     * The callback is invoked when data arrives, when an error
     * occurs, or the end of file is detected.  For data, the callback is
     * only invoked when the threshold is crossed.
     */
    typedef XorpCallback4<void, BufferedAsyncReader*, Event, uint8_t*, size_t>::RefPtr Callback;

    /**
     * Constructor.
     *
     * @param e the eventloop.
     * @param fd the file descriptor.
     * @param reserve_bytes the number of bytes to reserve in the data buffer.
     * @param cb the callback to invoke.
     * @param priority the task priority for the eventloop operations.
     */
    BufferedAsyncReader(EventLoop& 	e,
			XorpFd 		fd,
			size_t 		reserve_bytes,
			const Callback& cb,
			int priority = XorpTask::PRIORITY_DEFAULT);

    virtual ~BufferedAsyncReader();

    /**
     * Set threshold for event notification.  Only when this threshold
     * is reached the consumers callback invoked.  If more data is already
     * available, then the event notification will be triggered
     * through a 0 second timer.  This provides an opportunity for
     * other tasks to run.
     *
     * Calling this method may cause the internal buffer state to
     * change.  If it is called from within a consumer callback, then
     * the buffer pointer may become invalid and dereferencing the
     * pointer should be avoided.
     *
     * @param bytes the number of threshold bytes.
     * @return true on success, false if bytes is larger than reserve.
     */
    bool set_trigger_bytes(size_t bytes);

    /**
     * Get the current threshold for event notification.
     */
    size_t trigger_bytes() const;

    /**
     * Acknowledge data at the start of the buffer is finished with.
     *
     * Typically, a consumer would call this from within their
     * callback to say this number of bytes has been processed and can
     * be discarded.
     *
     * @param bytes the number of bytes to dispose.
     * @return true on success, false if bytes is larger than the number
     * of available bytes.
     */
    bool dispose(size_t bytes);

    /**
     * Set reserve for maximum amount of data to receive.
     *
     * @param bytes the number of bytes to reserve.
     * @return true on success, false if error.
     */
    bool set_reserve_bytes(size_t bytes);

    /**
     * Get reserve for maximum amount of data to receive.
     */
    size_t reserve_bytes() const;

    /**
     * Get the number of currently available bytes.
     */
    size_t available_bytes() const;

    int error() const { return _last_error; }

    /**
     * Start.
     *
     * Start asynchrous reads.
     */
    void start();

    /**
     * Stop.
     *
     * Stop asynchrous reading.
     */
    void stop();

    virtual string toString() const;

private:
    BufferedAsyncReader();		// Not directly constructible

private:
    void io_event(XorpFd fd, IoEventType type);
    void announce_event(Event e);
    void provision_trigger_bytes();

    struct Config {
	uint8_t* head;		// The beginning of data
	size_t   head_bytes;	// The number of available bytes with data
	size_t	 trigger_bytes;	// The number of bytes to trigger cb delivery
	size_t	 reserve_bytes;	// The number of bytes to reserve for data
    };

    Config		_config;

    EventLoop&		_eventloop;
    XorpFd		_fd;
    Callback		_cb;

    vector<uint8_t>	_buffer;
    XorpTimer		_ready_timer;
    int			_last_error;
    int			_priority;
};

#endif // __LIBXORP_BUFFERED_ASYNCIO_HH__
