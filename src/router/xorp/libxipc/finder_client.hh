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

// $XORP: xorp/libxipc/finder_client.hh,v 1.29 2008/10/02 21:57:19 bms Exp $

#ifndef __LIBXIPC_FINDER_CLIENT_HH__
#define __LIBXIPC_FINDER_CLIENT_HH__





#include "xrl/targets/finder_client_base.hh"
#include "finder_messenger.hh"

#include "xrl_pf.hh"

class FinderClientOp;
class FinderClientObserver;

/**
 * A one-to-many container used by the FinderClient to store
 * unresolved-to-resolved Xrl mappings.
 */
class FinderDBEntry {
public:
    typedef list<Xrl> XRLS;

    FinderDBEntry(const string& key);
    FinderDBEntry(const string& key, const string& value);
#ifdef XORP_USE_USTL
    FinderDBEntry() { }
#endif

    const string&	key() const	{ return _key; }
    const list<string>& values() const	{ return _values; }
    list<string>&	values()	{ return _values; }
    const XRLS&		xrls() const;
    void		clear();
    void		pop_front();

protected:
    string	 _key;
    list<string> _values;
    mutable XRLS _xrls;
};

/**
 * Interface class for FinderClient Xrl requests.
 *
 * The methods in this interface are implemented by the FinderClient
 * to handle its XRL requests.  It exists as a separate class to restrict
 * the operations that the XRL interface can invoke.
 */
class FinderClientXrlCommandInterface
{
public:
    virtual ~FinderClientXrlCommandInterface() {}
    virtual void uncache_xrl(const string& xrl) = 0;
    virtual void uncache_xrls_from_target(const string& target) = 0;
    virtual XrlCmdError dispatch_tunneled_xrl(const string& xrl) = 0;
};

/**
 * @short Class that represents clients of the Finder.
 *
 * The FinderClient class performs communication processing with the
 * Finder on behalf of XORP processes.  It handles XRL registration
 * and resolution requests.
 */
class FinderClient :
    public FinderMessengerManager, public FinderClientXrlCommandInterface
{
public:
    typedef
    XorpCallback2<void, const XrlError&, const FinderDBEntry*>::RefPtr
    QueryCallback;
    typedef ref_ptr<FinderClientOp> Operation;
    typedef list<Operation> OperationQueue;

    class InstanceInfo;

    typedef map<string, FinderDBEntry>	ResolvedTable;
    typedef map<string, string>		LocalResolvedTable;
    typedef vector<InstanceInfo>	InstanceList;

public:
    /**
     * Constructor.
     */
    FinderClient();

    /**
     * Destructor.
     */
    virtual ~FinderClient();

    /**
     * Register an Xrl Target with the FinderClient and place request with
     * Finder to perform registration. The request to the Finder is
     * asynchronous and there is a delay between when the request is made
     * when it is satisfied.
     *
     * @param instance_name a unique name to be associated with the Target.
     * @param class_name the class name that the Target is an instance of.
     * @param dispatcher pointer to Xrl dispatcher that can execute the
     * command.
     *
     * @return true on success, false if @ref instance_name or @ref
     * class_name are empty.
     */
    bool register_xrl_target(const string&	  instance_name,
			     const string&	  class_name,
			     const XrlDispatcher* dispatcher);

    /**
     * Unregister Xrl Target with FinderClient and place a request with
     * the Finder to remove registration.   The request to the Finder is
     * asynchronous and there is a delay between when the request is made
     * when it is satisfied.
     *
     * @param instance_name unique name associated with Xrl Target.
     */
    bool unregister_xrl_target(const string& instance_name);

    /**
     * Register an Xrl with the Finder.
     *
     * @param instance_name unique name associated with Xrl Target and making
     * the registration.
     *
     * @param xrl string representation of the Xrl.
     * @param pf_name protocol family name that implements Xrl.
     * @param pf_args protocol family arguments to locate dispatcher for
     * registered Xrl.
     *
     * @return true if registration request is successfully enqueued, false
     * otherwise.
     */
    bool register_xrl(const string& instance_name,
		      const string& xrl,
		      const string& pf_name,
		      const string& pf_args);

    /**
     * Request Finder advertise Xrls associated with an Xrl Target instance
     * to other XORP Xrl targets.  Until the Finder has satisfied this
     * request the Xrl Target has no visibility in the Xrl universe.
     *
     * @param instance_name unique name associated with Xrl Target to
     * be advertised.
     *
     * @return true on success, false if instance_name has not previously
     * been registered with FinderClient.
     */
    bool enable_xrls(const string& instance_name);

    /**
     * Request resolution of an Xrl.
     *
     * If the Xrl to be resolved in cache exists in the FinderClients
     * cache, the callback provided as a function argument is invoked
     * immediately.  Otherwise the request is forwarded to the finder,
     * the cache updated, and callback dispatched when the Finder
     * answers the request.
     *
     * @param eventloop the event loop.
     * @param xrl Xrl to be resolved.
     * @param qcb callback to be dispatched when result is availble.
     */
    void query(EventLoop&	    eventloop,
	       const string&	    xrl,
	       const QueryCallback& qcb);

    /**
     * Get number of asynchonous queries pending resolution.
     */
    uint32_t queries_pending() const;

    /**
     * Attempt to resolve Xrl from cache.
     *
     * @param xrl Xrl to be resolved.
     *
     * @return pointer to cached entry on success, 0 otherwise.
     */
    const FinderDBEntry* query_cache(const string& xrl) const;

    /**
     * Remove an resolved entry from cache.
     *
     * This method is typically called when the information provided
     * has been useful to consumer.  ie because the answer is found to
     * be no longer valid.
     *
     */
    void uncache_result(const FinderDBEntry* dbe);

    /**
     * Resolve Xrl that an Xrl Target associated with the FinderClient
     * registered.
     *
     * @param incoming_xrl_command the command component of the Xrl being
     * resolved.
     * @param local_xrl_command the local name of the Xrl command being
     * resolved.
     * @return true and assign value to local_xrl_command on success, false
     * on failure.
     */
    bool query_self(const string& incoming_xrl_command,
		    string& local_xrl_command) const;

    /**
     * Send an Xrl for the Finder to dispatch.  This is the mechanism
     * that allows clients of the Finder to interrogate the Finder through
     * an Xrl interface.
     *
     * @param x Xrl to be dispatched.
     * @param cb callback to be called with dispatch result.
     * @return true on success.
     */
    bool forward_finder_xrl(const Xrl& x, const XrlPFSender::SendCallback& cb);

    /**
     * Accessor for Finder Messenger used by FinderClient instance.
     */
    FinderMessengerBase* messenger();

    /**
     * Get list of operations pending.
     */
    OperationQueue& todo_list()		{ return _todo_list; }

    /**
     * Get List of operations done and are repeatable.
     */
    OperationQueue& done_list()		{ return _done_list; }

    /**
     * Notify successful completion of an operation on the todo list.
     */
    void   notify_done(const FinderClientOp* completed);

    /**
     * Notify failed completion of an operation on the todo list.
     */
    void   notify_failed(const FinderClientOp* completed);

    /**
     * Get the Xrl Commands implemented by the FinderClient.
     */
    XrlCmdMap& commands()		{ return _commands; }

    /**
     * @return true if FinderClient has registered Xrls and is considered
     * operational.
     */
    bool ready() const			{ return _xrls_registered; }

    /**
     * @return true if a connection is established with the Finder.
     */
    bool connected() const		{ return _messenger != 0; }

    /**
     * Attach a FinderClientObserver instance to receive event notifications.
     *
     * @param o pointer to observer to receive notifications.
     *
     * @return true on success, false if an observer is already present.
     */
    bool attach_observer(FinderClientObserver* o);

    /**
     * Detach the FinderClientObserver instance.
     *
     * @param o pointer to the FinderClientObserver be removed.
     *
     * @return true on success, false if the FinderClientObserver
     * is not the current observer.
     */
    bool detach_observer(FinderClientObserver* o);

protected:
    // FinderMessengerManager interface
    void messenger_birth_event(FinderMessengerBase*);
    void messenger_death_event(FinderMessengerBase*);
    void messenger_active_event(FinderMessengerBase*);
    void messenger_inactive_event(FinderMessengerBase*);
    void messenger_stopped_event(FinderMessengerBase*);
    bool manages(const FinderMessengerBase*) const;

protected:
    // FinderClientXrlCommandInterface
    void uncache_xrl(const string& xrl);
    void uncache_xrls_from_target(const string& target);
    XrlCmdError dispatch_tunneled_xrl(const string& xrl);

private:
    void
    dispatch_tunneled_xrl_cb(const XrlError &e, const XrlArgs *a) const;

protected:
    void crank();
    void prepare_for_restart();

protected:
    InstanceList::iterator find_instance(const string& instance);
    InstanceList::const_iterator find_instance(const string& instance) const;

protected:
    OperationQueue	 _todo_list;
    OperationQueue	 _done_list;
    ResolvedTable	 _rt;
    LocalResolvedTable	 _lrt;
    InstanceList	 _ids;

    XrlCmdMap		 _commands;

    FinderMessengerBase* _messenger;
    bool		 _pending_result;
    bool		 _xrls_registered;

    FinderClientObserver* _observer;
};

#endif // __LIBXIPC_FINDER_CLIENT_HH__
