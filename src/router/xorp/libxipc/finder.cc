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

#include "libxorp/debug.h"
#include "libxorp/xlog.h"

#include "finder_tcp.hh"
#include "finder.hh"

#include "finder_xrl_queue.hh"

///////////////////////////////////////////////////////////////////////////////
//
// FinderTarget
//
// This class is a container for values associated with a particular
// target.  It contains a one-to-many mapping for values associated with
// the target, eg unresolved xrl to resolved xrls, name-to-values, etc...
//

class FinderTarget {
public:
    typedef Finder::Resolveables Resolveables;
    typedef map<string, Resolveables> ResolveMap;
public:
    FinderTarget(const string& name,
		 const string& class_name,
		 const string& cookie,
		 FinderMessengerBase* fm)
	: _name(name), _class_name(class_name), _cookie(cookie),
	  _enabled(false), _messenger(fm)
    {}

    FinderTarget() { _messenger = NULL; }

    ~FinderTarget() {
	debug_msg("Destructing %s\n", name().c_str());
    }

    const string& name() const		{ return _name; }
    const string& class_name() const	{ return _class_name; }
    const string& cookie() const	{ return _cookie; }
    bool enabled() const		{ return _enabled; }
    void set_enabled(bool en)		{ _enabled = en; }

    const FinderMessengerBase* messenger() const { return _messenger; }

    FinderMessengerBase* messenger() { return _messenger; }

    const ResolveMap& resolve_map() const { return _resolutions; }

    bool add_resolution(const string& key, const string& value) {
	Resolveables& r = _resolutions[key];
	if (find(r.begin(), r.end(), value) == r.end())
	    r.push_back(value);
	return true;
    }

    bool remove_resolutions(const string& key) {
	ResolveMap::iterator i = _resolutions.find(key);
	if (_resolutions.end() != i) {
	    _resolutions.erase(i);
	    return true;
	}
	return false;
    }

    const Resolveables* resolveables(const string& key) const {
	ResolveMap::const_iterator i = _resolutions.find(key);
	if (_resolutions.end() == i) {
	    debug_msg("Looking for \"%s\"\n", key.c_str());
	    for (i = _resolutions.begin(); i != _resolutions.end(); ++i) {
		debug_msg("Have \"%s\"\n", i->first.c_str());
	    }
	    return NULL;
	}
	return &i->second;
    }

    bool add_class_watch(const string& class_name) {
	pair<set<string>::iterator,bool> r = _classwatches.insert(class_name);
	return r.second;
    }

    void remove_class_watch(const string& class_name) {
	set<string>::iterator i = _classwatches.find(class_name);
 	if (i != _classwatches.end()) {
	    _classwatches.erase(i);
	}
    }

    bool has_class_watch(const string& class_name) const {
#ifdef DEBUG_LOGGING
	debug_msg("tgt %s has watches on:\n", _name.c_str());
	set<string>::const_iterator wi = _classwatches.begin();
	while (wi != _classwatches.end()) {
	    debug_msg("-> %s\n", wi->c_str());
	    ++wi;
	}
#endif
	return _classwatches.find(class_name) != _classwatches.end();
    }

    bool add_instance_watch(const string& instance_name) {
	pair<set<string>::iterator,bool> r = _instancewatches.insert(instance_name);
	return r.second;
    }

    void remove_instance_watch(const string& instance_name) {
	set<string>::iterator i = _instancewatches.find(instance_name);
 	if (i != _instancewatches.end()) {
	    _instancewatches.erase(i);
	}
    }

    bool has_instance_watch(const string& instance_name) const {
#ifdef DEBUG_LOGGING
	debug_msg("tgt %s has watches on:\n", _name.c_str());
	set<string>::const_iterator wi = _instancewatches.begin();
	while (wi != _instancewatches.end()) {
	    debug_msg("-> %s\n", wi->c_str());
	    ++wi;
	}
#endif
	return _instancewatches.find(instance_name) != _instancewatches.end();
    }

protected:
    string			_name;		// name of target
    string			_class_name;	// name of target class
    string			_cookie;
    bool			_enabled;
    set<string>			_classwatches;	// set of classes being
    						// watched
    set<string>			_instancewatches; // set of targets being
    						// watched
    ResolveMap			_resolutions;	// items registered by target
    FinderMessengerBase*	_messenger;	// source of registrations
};


///////////////////////////////////////////////////////////////////////////////
//
// FinderClass
//
// Stores information about which classes exist and what instances
// exist.
//

class FinderClass {
public:
    FinderClass(const string& name, bool singleton)
	: _name(name), _singleton(singleton)
    {}
#ifdef XORP_USE_USTL
    FinderClass() { }
#endif

    const string& name() const			{ return _name; }
    bool singleton() const			{ return _singleton; }
    const list<string>& instances() const	{ return _instances; }

    bool add_instance(const string& instance) {
	list<string>::const_iterator i = find(_instances.begin(),
					      _instances.end(),
					      instance);
	if (i != _instances.end()) {
	    debug_msg("instance %s already exists.\n",
		    instance.c_str());
	    return false;
	}
	debug_msg("added instance \"%s\".\n",
		instance.c_str());

	debug_msg("Adding instance %s to class %s\n",
		  instance.c_str(), _name.c_str());
	_instances.push_back(instance);
	return true;
    }

    bool remove_instance(const string& instance) {
	list<string>::iterator i = find(_instances.begin(), _instances.end(),
					instance);
	if (i == _instances.end()) {
	    debug_msg("Failed to remove unknown instance %s from class %s\n",
		      instance.c_str(), _name.c_str());

	    return false;
	}
	_instances.erase(i);
	debug_msg("Removed instance %s from class %s\n",
		  instance.c_str(), _name.c_str());

	return true;
    }

protected:
    string	 _name;
    list<string> _instances;
    bool	 _singleton;
};


///////////////////////////////////////////////////////////////////////////////
//
// Finder Event
//
// Used as part of event logging.
//
class FinderEvent {
public:
    enum Tag {
	TARGET_BIRTH = 0x1,
	TARGET_DEATH = 0x2
    };
public:
    FinderEvent(Tag t, const string& class_name, const string& instance)
	: _tag(t),  _class_name(class_name), _instance_name(instance)
    {
	debug_msg("FinderEvent(%s, \"%s\", \"%s\")\n",
		  (_tag == TARGET_BIRTH) ? "birth" : "death",
		  _class_name.c_str(), _instance_name.c_str());
    }
#ifdef XORP_USE_USTL
    FinderEvent() { }
#endif
    ~FinderEvent()
    {
	debug_msg("~FinderEvent(%s, \"%s\", \"%s\")\n",
		  (_tag == TARGET_BIRTH) ? "birth" : "death",
		  _class_name.c_str(), _instance_name.c_str());
    }
    const Tag& tag() const		{ return _tag; }
    const string& instance_name() const	{ return _instance_name; }
    const string& class_name() const	{ return _class_name; }

protected:
    Tag	   _tag;
    string _class_name;
    string _instance_name;
};


///////////////////////////////////////////////////////////////////////////////
//
// Fake Xrl Sender
//
// Provides a means to get an Xrl out of an autogenerated interface.  It
// renders the Xrl passed to it into a pre-supplied string buffer.
//

class XrlFakeSender : public XrlSender {
public:
    XrlFakeSender(string& outbuf) : _buf(outbuf)
    {}

    ~XrlFakeSender()
    {}

    bool send(const Xrl& x, const XrlSender::Callback&) {
	_buf = x.str();
	return true;
    }

    bool pending() const { return false; }

private:
    string& _buf;
};


///////////////////////////////////////////////////////////////////////////////
//
// Internal consistency check
//

static void
validate_finder_classes_and_instances(const Finder::ClassTable&  classes,
				      const Finder::TargetTable& targets)

{
#ifdef FINDER_CONSISTENCY_CHECKS
    typedef Finder::ClassTable  ClassTable;
    typedef Finder::TargetTable TargetTable;

    //
    // Validate each instance associated with classes table exists in
    // TargetTable
    //
    for (ClassTable::const_iterator ci = classes.begin();
	 ci != classes.end(); ++ci) {
	XLOG_ASSERT(ci->second.instances().empty() == false);
	for (list<string>::const_iterator ii = ci->second.instances().begin();
	     ii != ci->second.instances().end(); ++ii) {
	    if (targets.find(*ii) == targets.end()) {
		for (TargetTable::const_iterator ti = targets.begin();
		     ti != targets.end(); ti++) {
		    debug_msg("Known target \"%s\"\n", ti->first.c_str());
		}
		XLOG_FATAL("Missing instance (%s) / %u\n", ii->c_str(),
			   XORP_UINT_CAST(targets.size()));
	    }
	}
    }

    //
    // Validate each instance in the target table exists is associated
    // with a class entry.
    //
    for (TargetTable::const_iterator ti = targets.begin();
	 ti != targets.end(); ++ti) {
	XLOG_ASSERT(ti->first == ti->second.name());
	ClassTable::const_iterator ci = classes.find(ti->second.class_name());
	if (ci == classes.end()) {
	    XLOG_FATAL("Class (%s) associated with instance (%s) does not "
		       "appear in class table.\n",
		       ti->second.class_name().c_str(),
		       ti->second.name().c_str());
	    if (find(ci->second.instances().begin(),
		     ci->second.instances().end(),
		     ti->second.name()) == ci->second.instances().end()) {
		XLOG_FATAL("Instance (%s) is not associated with class in "
			   "expected class (%s).\n",
			   ti->second.name().c_str(),
			   ti->second.class_name().c_str());
	    }
	}
    }

#else // ! FINDER_CONSISTENCY_CHECKS
    return;
    UNUSED(classes);
    UNUSED(targets);
#endif // FINDER_CONSISTENCY_CHECKS
}


///////////////////////////////////////////////////////////////////////////////
//
// Finder
//

Finder::Finder(EventLoop& e) : _e(e), _cmds("finder"), _active_messenger(0)
{
}

Finder::~Finder()
{
    validate_finder_classes_and_instances(_classes, _targets);
    _targets.clear();
    _classes.clear();
    while (false == _messengers.empty()) {
	FinderMessengerBase* old_front = _messengers.front();
	delete _messengers.front();
	// Expect death event for messenger to remove item from list
	assert(_messengers.empty() || (_messengers.front() != old_front));
    }
}

void
Finder::messenger_active_event(FinderMessengerBase* m)
{
    XLOG_ASSERT(0 == _active_messenger);
    debug_msg("messenger %p active\n", m);
    _active_messenger = m;
}

void
Finder::messenger_inactive_event(FinderMessengerBase* m)
{
    XLOG_ASSERT(m == _active_messenger);
    debug_msg("messenger %p inactive\n", m);
    _active_messenger = 0;
}

void
Finder::messenger_stopped_event(FinderMessengerBase* m)
{
    debug_msg("Messenger %p stopped.", m);
    if (m == _active_messenger)
	_active_messenger = NULL;
    delete m;
}

void
Finder::messenger_birth_event(FinderMessengerBase* m)
{
    XLOG_ASSERT(
	_messengers.end() == find(_messengers.begin(), _messengers.end(), m)
	);
    _messengers.push_back(m);

    debug_msg("messenger %p birth\n", m);
    XLOG_ASSERT(_out_queues.end() == _out_queues.find(m));
    _out_queues.insert(OutQueueTable::value_type(m, FinderXrlCommandQueue(m)));
    if (hello_timer_running() == false)
	start_hello_timer();
}

void
Finder::messenger_death_event(FinderMessengerBase* m)
{
    validate_finder_classes_and_instances(_classes, _targets);

    debug_msg("Finder::messenger %p death\n", m);

    //
    // 1. Remove from Messenger list
    //
    FinderMessengerList::iterator mi;
    mi = find(_messengers.begin(), _messengers.end(), m);
    XLOG_ASSERT(_messengers.end() != mi);
    _messengers.erase(mi);

    //
    // 2. Clear up queue associated with messenger
    //
    OutQueueTable::iterator oi;
    oi = _out_queues.find(m);
    XLOG_ASSERT(_out_queues.end() != oi);
    _out_queues.erase(oi);
    XLOG_ASSERT(_out_queues.end() == _out_queues.find(m));

    //
    // 3. Walk targets associated with messenger, remove them and announce fact
    //
    validate_finder_classes_and_instances(_classes, _targets);

    for (TargetTable::iterator ti = _targets.begin(); ti != _targets.end(); ) {
	FinderTarget& tgt = ti->second;
	if (tgt.messenger() != m) {
	    ti++;
	    continue;
	}
	remove_target(ti);
	break;
    }
    announce_events_externally();
    validate_finder_classes_and_instances(_classes, _targets);
}

bool
Finder::manages(const FinderMessengerBase* m) const
{
    return _messengers.end() !=
	find(_messengers.begin(), _messengers.end(), m);
}

size_t
Finder::messengers() const
{
    return _messengers.size();
}

XrlCmdMap&
Finder::commands()
{
    return _cmds;
}

bool
Finder::add_target(const string& cls,
		   const string& tgt,
		   bool		 singleton,
		   const string& cookie)
{
    validate_finder_classes_and_instances(_classes, _targets);
    //
    // Add instance
    //
    debug_msg("add_target %s / %s / %s\n", tgt.c_str(), cls.c_str(),
	      cookie.c_str());

    TargetTable::const_iterator ci = _targets.find(tgt);
    if (ci != _targets.end()) {
	if (ci->second.messenger() == _active_messenger) {
	    debug_msg("already registered by messenger.\n");
	    return true;
	} else {
	    debug_msg("Fail registered by another messenger.");
	    return false;
	}
    }

    pair<TargetTable::iterator, bool> r =
	_targets.insert(
	    TargetTable::value_type(tgt,
				    FinderTarget(tgt, cls, cookie,
						 _active_messenger)));
    if (r.second == false) {
	debug_msg("Failed to insert target.");
	return false;
    }

    //
    // Add class and instance to class
    //
    if (add_class_instance(cls, tgt, singleton) == false) {
	debug_msg("Failed to register class instance");
	_targets.erase(r.first);
	return false;
    }
    validate_finder_classes_and_instances(_classes, _targets);
    return true;
}

bool
Finder::active_messenger_represents_target(const string& tgt) const
{
    validate_finder_classes_and_instances(_classes, _targets);
    TargetTable::const_iterator i = _targets.find(tgt);
    if (_targets.end() == i) {
	debug_msg("Failed to find target %s\n", tgt.c_str());
	for (TargetTable::const_iterator ci = _targets.begin();
	     ci != _targets.end(); ++ci) {
	    debug_msg("Target \"%s\"\n", ci->first.c_str());
	}
	return false;
    }
    return i->second.messenger() == _active_messenger;
}

void
Finder::remove_target(TargetTable::iterator& i)
{
    validate_finder_classes_and_instances(_classes, _targets);

    FinderTarget& t = i->second;
    debug_msg("Removing target %s / %s / %s\n",
	      t.name().c_str(),
	      t.class_name().c_str(),
	      t.cookie().c_str());

    log_departure_event(t.class_name(), t.name());
    if (primary_instance(t.class_name()) == t.name()) {
	log_departure_event(t.class_name(), t.class_name());
    }
    remove_class_instance(t.class_name(), t.name());
    _targets.erase(i);

    validate_finder_classes_and_instances(_classes, _targets);
}

bool
Finder::remove_target_with_cookie(const string& cookie)
{
    validate_finder_classes_and_instances(_classes, _targets);

    TargetTable::iterator i;
    for (i = _targets.begin(); i != _targets.end(); ++i) {
	if (i->second.cookie() != cookie)
	    continue;
	remove_target(i);
	announce_events_externally();
	validate_finder_classes_and_instances(_classes, _targets);
	return true;
    }
    debug_msg("Failed to find target with cookie %s\n", cookie.c_str());
    return false;
}

bool
Finder::remove_target(const string& target)
{
    validate_finder_classes_and_instances(_classes, _targets);

    TargetTable::iterator i = _targets.find(target);

    if (_targets.end() == i) {
	return false;
    }

    FinderTarget& tgt = i->second;
    if (tgt.messenger() != _active_messenger) {
	// XXX TODO walk list of targets and print out what offending
	// messenger is responsible for + string representation of messenger.
	XLOG_WARNING("Messenger illegally attempted to remove %s\n",
		     target.c_str());
	return false;
    }
    remove_target(i);
    announce_events_externally();
    return true;
}

bool
Finder::set_target_enabled(const string& tgt, bool en)
{
    TargetTable::iterator i = _targets.find(tgt);
    if (_targets.end() == i) {
	return false;
    }
    if (i->second.enabled() == en) {
	return true;
    }
    i->second.set_enabled(en);
    if (en) {
	log_arrival_event(i->second.class_name(), tgt);
    } else {
	log_departure_event(i->second.class_name(), tgt);
    }
    announce_events_externally();

    return true;
}

bool
Finder::target_enabled(const string& tgt, bool& en) const
{
    TargetTable::const_iterator i = _targets.find(tgt);
    if (_targets.end() == i) {
	return false;
    }
    en = i->second.enabled();
    return true;
}

bool
Finder::add_resolution(const string& tgt,
		       const string& key,
		       const string& value)
{
    TargetTable::iterator i = _targets.find(tgt);

    if (_targets.end() == i) {
	return false;
    }

    if (i->second.messenger() != _active_messenger) {
	XLOG_WARNING("Messenger illegally attempted to add to %s\n",
		     tgt.c_str());
	return false;
    }

    FinderTarget& t = i->second;
    return t.add_resolution(key, value);
}

bool
Finder::remove_resolutions(const string& tgt,
			   const string& key)
{
    TargetTable::iterator i = _targets.find(tgt);

    if (_targets.end() == i) {
	return false;
    }

    if (i->second.messenger() != _active_messenger) {
	XLOG_WARNING("Messenger illegally attempted to add to %s\n",
		     tgt.c_str());
	return false;
    }

    FinderTarget& t = i->second;
    if (t.remove_resolutions(key)) {
	announce_xrl_departure(tgt, key);
	return true;
    }
    return false;
}

const Finder::Resolveables*
Finder::resolve(const string& tgt, const string& key)
{
    TargetTable::iterator i = _targets.find(tgt);
    if (_targets.end() == i) {
	return 0;
    }
    return i->second.resolveables(key);
}

void
Finder::log_arrival_event(const string& cls, const string& ins)
{
    _event_queue.push_back(FinderEvent(FinderEvent::TARGET_BIRTH, cls, ins));
}

void
Finder::log_departure_event(const string& cls, const string& ins)
{
    FinderMessengerList::iterator i;

    for (i = _messengers.begin(); i != _messengers.end(); ++i) {
	OutQueueTable::iterator qi = _out_queues.find(*i);
	XLOG_ASSERT(_out_queues.end() != qi);
	FinderXrlCommandQueue& q = qi->second;
	q.enqueue(new FinderSendRemoveXrls(q, ins));
    }
    if (ins == cls)
	return;
    _event_queue.push_back(FinderEvent(FinderEvent::TARGET_DEATH, cls, ins));
}

void
Finder::announce_xrl_departure(const string& tgt, const string& key)
{
    FinderMessengerList::iterator i;

    for (i = _messengers.begin(); i != _messengers.end(); ++i) {
	OutQueueTable::iterator qi = _out_queues.find(*i);
	XLOG_ASSERT(_out_queues.end() != qi);
	FinderXrlCommandQueue& q = qi->second;
	q.enqueue(new FinderSendRemoveXrl(q, tgt, key));
    }
}

bool
Finder::fill_target_list(list<string>& tgt_list) const
{
    TargetTable::const_iterator ci;
    for (ci = _targets.begin(); ci != _targets.end(); ++ci) {
	tgt_list.push_back(ci->first);
    }
    return true;
}

bool
Finder::fill_targets_xrl_list(const string& target,
			      list<string>& xrl_list) const
{
    TargetTable::const_iterator ci = _targets.find(target);
    if (_targets.end() == ci) {
	return false;
    }

    FinderTarget::ResolveMap::const_iterator
	cmi = ci->second.resolve_map().begin();

    const FinderTarget::ResolveMap::const_iterator
	end = ci->second.resolve_map().end();

    while (end != cmi) {
	xrl_list.push_back(cmi->first);
	++cmi;
    }
    return true;
}

void
Finder::start_hello_timer()
{
    _hello = _e.new_periodic_ms(XORP_HELLO_TIMER_MS, callback(this, &Finder::send_hello));
}

bool
Finder::send_hello()
{
    validate_finder_classes_and_instances(_classes, _targets);

    OutQueueTable::iterator oqi = _out_queues.begin();
    debug_msg("Send hello\n");

    if (oqi == _out_queues.end()) {
	return false;
    }

    do {
	FinderXrlCommandQueue& q = oqi->second;
	XLOG_ASSERT(find(_messengers.begin(), _messengers.end(), &q.messenger())
		    != _messengers.end());
	q.enqueue(new FinderSendHelloToClient(q, "oxo"));
	++oqi;
    } while (oqi != _out_queues.end());

    return true;
}

bool
Finder::class_exists(const string& class_name) const
{
    return _classes.find(class_name) != _classes.end();
}

bool
Finder::add_class_instance(const string& class_name,
			   const string& instance,
			   bool		 singleton)
{
    ClassTable::iterator i = _classes.find(class_name);
    if (i == _classes.end()) {
	pair<ClassTable::iterator, bool> r;
	ClassTable::value_type pv(class_name,
				  FinderClass(class_name, singleton));
	r = _classes.insert(pv);
	if (r.second == false) {
	    // Insertion failed
	    debug_msg("insert failed");
	    return false;
	}
	i = r.first;
    }

    if ((singleton || i->second.singleton()) &&
	i->second.instances().empty() == false) {
	debug_msg("blocked by singleton");
	return false;
    }

    return i->second.add_instance(instance);
}

bool
Finder::remove_class_instance(const string& class_name,
			      const string& instance)
{
    ClassTable::iterator i = _classes.find(class_name);
    if (i == _classes.end()) {
	debug_msg("Attempt to remove unknown class %s not found\n",
		  class_name.c_str());
	return false;
    } else if (i->second.remove_instance(instance) == false) {
	debug_msg("Attempt to remove unknown instance (%s) from class %s",
		  instance.c_str(), class_name.c_str());
	return false;
    }
    if (i->second.instances().empty()) {
	_classes.erase(i);
    }
    debug_msg("Removed class instance (%s) from class %s\n",
	      instance.c_str(), class_name.c_str());
    return true;
}

bool
Finder::class_default_instance(const string& class_name,
			       string&	     instance) const
{
    ClassTable::const_iterator ci = _classes.find(class_name);
    if (ci == _classes.end() || ci->second.instances().empty()) {
	return false;
    }
    instance = ci->second.instances().front();
    return true;
}

const string&
Finder::primary_instance(const string& instance_or_class) const
{
    validate_finder_classes_and_instances(_classes, _targets);
    ClassTable::const_iterator ci = _classes.find(instance_or_class);
    if (ci == _classes.end()) {
	return instance_or_class;
    }
    XLOG_ASSERT(ci->second.instances().empty() == false);
    return ci->second.instances().front();
}

bool
Finder::add_class_watch(const string& target,
			const string& class_to_watch,
			string& err_msg)
{
    TargetTable::iterator i = _targets.find(target);
    if (i == _targets.end()) {
	err_msg += c_format("could not find target: %s in Finder::add_class_watch\n",
			   target.c_str());
	return false;
    }

    if (! i->second.add_class_watch(class_to_watch)) {
	XLOG_WARNING("WARNING: Class watch already existed in add_class_watch,"
		     " target: %s  class-to-watch: %s\n",
		     target.c_str(), class_to_watch.c_str());
	// Old code returned false, which perculated back up the stack.
	// Seems we could ignore this since it appears to be there already.
	// --Ben
    }
    announce_class_instances(target, class_to_watch);
    return true;
}

bool
Finder::remove_class_watch(const string& target,
			   const string& class_to_watch)
{
    TargetTable::iterator i = _targets.find(target);
    if (i == _targets.end())
	return false;
    i->second.remove_class_watch(class_to_watch);
    return true;
}

bool
Finder::add_instance_watch(const string& target,
			   const string& instance_to_watch,
			   string& err_msg)
{
    TargetTable::iterator watcher_i = _targets.find(target);
    if (watcher_i == _targets.end()) {
	err_msg += "Could not find target: ";
	err_msg += target;
	return false;	// watcher does not exist
    }

    TargetTable::const_iterator watched_i = _targets.find(instance_to_watch);
    if (watched_i == _targets.end()) {
	err_msg += "Could not find instance-to-watch: ";
	err_msg += instance_to_watch;
	return false;	// watched does not exist
    }

    FinderTarget& watcher = watcher_i->second;
    if (watcher.add_instance_watch(instance_to_watch)) {
	OutQueueTable::iterator oqi = _out_queues.find(watcher.messenger());
	XLOG_ASSERT(oqi != _out_queues.end());
	FinderXrlCommandQueue& out_queue = oqi->second;
	const FinderTarget& watched = watched_i->second;
	announce_new_instance(watcher.name(), out_queue,
			      watched.class_name(),
			      watched.name());
	return true;
    }
    err_msg += "Watcher failed to add_instance_watch.\n";
    return false;
}

bool
Finder::remove_instance_watch(const string& target,
			      const string& instance_to_watch)
{
    TargetTable::iterator i = _targets.find(target);
    if (i == _targets.end())
	return false;
    i->second.remove_instance_watch(instance_to_watch);
    return true;
}

#include "xrl/interfaces/finder_event_observer_xif.hh"

static void
dummy_xrl_cb(const XrlError& e)
{
    XLOG_ASSERT(e == XrlError::OKAY());
}

void
Finder::announce_events_externally()
{
    while (_event_queue.empty() == false) {
	FinderEvent& ev = _event_queue.front();
	TargetTable::iterator i;
	for (i = _targets.begin(); i != _targets.end(); ++i) {
	    FinderTarget& t = i->second;
	    if (t.has_class_watch(ev.class_name()) == false &&
		t.has_instance_watch(ev.instance_name()) == false) {
		// t has neither an instance watch nor a class watch
		// on event entity
		continue;
	    }

	    // Build Xrl to tunnel to FinderClient.

	    string xrl_to_tunnel;
	    XrlFakeSender s(xrl_to_tunnel);
	    XrlFinderEventObserverV0p1Client eo(&s);

	    switch(ev.tag()) {
	    case FinderEvent::TARGET_BIRTH:
		eo.send_xrl_target_birth(t.name().c_str(),
					 ev.class_name(),
					 ev.instance_name(),
					 callback(dummy_xrl_cb));
		break;
	    case FinderEvent::TARGET_DEATH:
		eo.send_xrl_target_death(t.name().c_str(),
					 ev.class_name(),
					 ev.instance_name(),
					 callback(dummy_xrl_cb));
		break;
	    }
	    XLOG_ASSERT(xrl_to_tunnel.empty() == false);

	    //
	    // Message has form of unresolved xrl.  We send resolved form
	    // to make spoofing harder.  This is circuitous.
	    //
	    Xrl x(xrl_to_tunnel.c_str());
	    const Resolveables* r = resolve(t.name(), x.string_no_args());
	    if (r == 0 || r->empty()) {
		XLOG_ERROR("Failed to resolve %s\n", xrl_to_tunnel.c_str());
		continue;
	    }

	    Xrl y(r->front().c_str());
	    Xrl z(x.target(), y.command(), x.args());
	    xrl_to_tunnel = z.str();

	    XLOG_ASSERT(find(_messengers.begin(), _messengers.end(),
			     t.messenger()) != _messengers.end());
	    // Message has been built, now need to find appropriate message
	    // queue and send it.
	    OutQueueTable::iterator oqi = _out_queues.find(t.messenger());
	    if (oqi == _out_queues.end()) {
		debug_msg("OutQueue for \"%s\" no longer exists.",
			  t.name().c_str());
		continue;
	    }
	    FinderXrlCommandQueue& q = oqi->second;
	    q.enqueue(new FinderSendTunneledXrl(q, t.name(), xrl_to_tunnel));
	    debug_msg("Enqueued xrl \"%s\"\n", xrl_to_tunnel.c_str());
	}
	_event_queue.pop_front();
    }
}

void
Finder::announce_class_instances(const string& recv_instance_name,
				 const string& class_name)
{
    ClassTable::const_iterator cti = _classes.find(class_name);
    if (cti == _classes.end())
	return;

    TargetTable::iterator tti = _targets.find(recv_instance_name);
    XLOG_ASSERT(tti != _targets.end());

    OutQueueTable::iterator oqi = _out_queues.find(tti->second.messenger());
    XLOG_ASSERT(oqi != _out_queues.end());

    const list<string>& instances = cti->second.instances();
    for (list<string>::const_iterator cii = instances.begin();
	 cii != instances.end(); cii++) {
	announce_new_instance(recv_instance_name,
			      oqi->second,
			      class_name,
			      *cii);
    }
}

void
Finder::announce_new_instance(const string& recv_instance_name,
			      FinderXrlCommandQueue& oq,
			      const string& class_name,
			      const string& new_instance_name)
{
    string xrl_to_tunnel;
    XrlFakeSender s(xrl_to_tunnel);
    XrlFinderEventObserverV0p1Client eo(&s);

    eo.send_xrl_target_birth(recv_instance_name.c_str(),
			     class_name, new_instance_name,
			     callback(dummy_xrl_cb));
    XLOG_ASSERT(xrl_to_tunnel.empty() == false);

    //
    // Message has form of unresolved xrl.  We send resolved form
    // to make spoofing harder.  This is circuitous.
    //
    Xrl x(xrl_to_tunnel.c_str());
    const Resolveables* r = resolve(recv_instance_name,
				    x.string_no_args());
    if (r == 0 || r->empty()) {
	XLOG_ERROR("Failed to resolve %s\n", xrl_to_tunnel.c_str());
	return;
    }
    Xrl y(r->front().c_str());
    Xrl z(x.target(), y.command(), x.args());
    xrl_to_tunnel = z.str();

    oq.enqueue(new FinderSendTunneledXrl(oq, recv_instance_name,
					 xrl_to_tunnel));
    debug_msg("Enqueued xrl \"%s\"\n", xrl_to_tunnel.c_str());
}

