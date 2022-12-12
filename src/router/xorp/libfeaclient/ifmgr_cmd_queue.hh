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

#ifndef __IFMGR_CMD_QUEUE_HH__
#define __IFMGR_CMD_QUEUE_HH__


#include "libxorp/ref_ptr.hh"

class IfMgrCommandBase;

class IfMgrIfTree;
class IfMgrIfAtom;
class IfMgrVifAtom;
class IfMgrIPv4Atom;
#ifdef HAVE_IPV6
class IfMgrIPv6Atom;
#endif

/**
 * @short Base class for command sinks.
 */
class IfMgrCommandSinkBase {
public:
    typedef ref_ptr<IfMgrCommandBase> Cmd;
public:
    virtual void push(const Cmd& cmd) = 0;
    virtual ~IfMgrCommandSinkBase();
};

/**
 * @short 2-way IfMgr Command Tee.
 *
 * Instances push commands pushed into them into two object derived
 * from @ref IfMgrCommandSinkBase.
 */
class IfMgrCommandTee : public IfMgrCommandSinkBase {
public:
    typedef IfMgrCommandSinkBase::Cmd Cmd;

public:
    IfMgrCommandTee(IfMgrCommandSinkBase& o1, IfMgrCommandSinkBase& o2);
    void push(const Cmd& cmd);

protected:
    IfMgrCommandSinkBase& _o1;
    IfMgrCommandSinkBase& _o2;
};


/**
 * @short N-way IfMgr Command Tee.
 *
 * Instances push commands pushed into them into multiple objects derived
 * from @ref IfMgrCommandSinkBase.
 */
template <typename SinkType = IfMgrCommandSinkBase>
class IfMgrNWayCommandTee : public IfMgrCommandSinkBase {
public:
    typedef IfMgrCommandSinkBase::Cmd Cmd;
    typedef list<SinkType*> SinkList;

public:
    void push(const Cmd& cmd);

    /**
     * Add an additional output for pushed commands.
     * @param sink receiver for commands pushed into instance.
     * @return true if sink is successfully added, false otherwise.
     */
    bool add_sink(SinkType* sink);

    /**
     * Remove an sink for pushed commands.
     * @param sink receiver for commands pushed into instance.
     * @return true if sink is successfully remove, false otherwise.
     */
    bool remove_sink(SinkType* sink);

protected:
    SinkList _sinks;
};

template <typename SinkType>
void
IfMgrNWayCommandTee<SinkType>::push(const Cmd& cmd)
{
    typename SinkList::iterator i;
    for (i = _sinks.begin(); i != _sinks.end(); ++i) {
	(*i)->push(cmd);
    }
}

template <typename SinkType>
bool
IfMgrNWayCommandTee<SinkType>::add_sink(SinkType* o)
{
    if (find(_sinks.begin(), _sinks.end(), o) != _sinks.end())
	return false;
    _sinks.push_back(o);
    return true;
}

template <typename SinkType>
bool
IfMgrNWayCommandTee<SinkType>::remove_sink(SinkType* o)
{
    typename SinkList::iterator i = find(_sinks.begin(), _sinks.end(), o);
    if (i == _sinks.end())
	return false;
    _sinks.erase(i);
    return true;
}


/**
 * @short Class to dispatch Interface Manager Commands.
 *
 * This class buffers exactly one Interface Manager Command (@ref
 * IfMgrCommandBase) and applies it to an Interface Manager
 * Configuration Tree (@ref IfMgrIfTree) when it's execute() method is
 * called.
 */
class IfMgrCommandDispatcher : public IfMgrCommandSinkBase {
public:
    typedef IfMgrCommandSinkBase::Cmd Cmd;

public:
    /**
     * Constructor
     * @param tree configuration tree to apply commands to.
     */
    IfMgrCommandDispatcher(IfMgrIfTree& tree);

    /**
     * Push a command into local storage ready for execution.
     */
    void push(const Cmd& cmd);

    /**
     * Execute command.
     *
     * @return command return status (true indicates success, false failure).
     */
    virtual bool execute();

protected:
    Cmd		 _cmd;
    IfMgrIfTree& _iftree;
};

/**
 * @short Base class for Command Queue classes.
 */
class IfMgrCommandQueueBase : public IfMgrCommandSinkBase {
public:
    typedef IfMgrCommandSinkBase::Cmd Cmd;
public:
    /**
     * Add an item to the queue.
     */
    virtual void push(const Cmd& cmd) = 0;

    /**
     * @return true if queue has no items, false otherwise.
     */
    virtual bool empty() const = 0;

    /**
     * Accessor for front item from queue.
     *
     * @return reference to front item if queue is not empty, junk otherwise.
     */
    virtual Cmd& front() = 0;

    /**
     * Accessor for front item from queue.
     *
     * @return reference to front item if queue is not empty, junk otherwise.
     */
    virtual const Cmd& front() const = 0;

    /**
     * Pop the front item from queue.
     */
    virtual void pop_front() = 0;
};

/**
 * @short FIFO Queue for command objects.
 */
class IfMgrCommandFifoQueue : public IfMgrCommandQueueBase {
public:
    typedef IfMgrCommandQueueBase::Cmd Cmd;

public:
    void 	push(const Cmd& cmd);
    bool	empty() const;
    Cmd&	front();
    const Cmd&	front() const;
    void	pop_front();

protected:
    list<Cmd> _fifo;
};

/**
 * @short Interface Command Clustering Queue.
 *
 * This Queue attempts to cluster commands based on their interface
 * name.  Only command objects derived from IfMgrIfCommandBase may be
 * placed in the queue, command objects not falling in this category will
 * give rise to assertion failures.
 */
class IfMgrCommandIfClusteringQueue : public IfMgrCommandQueueBase {
public:
    typedef IfMgrCommandQueueBase::Cmd Cmd;
    typedef list<Cmd> CmdList;

public:
    IfMgrCommandIfClusteringQueue();

    void 	push(const Cmd& cmd);
    bool	empty() const;
    Cmd&	front();
    const Cmd&	front() const;
    void	pop_front();

protected:
    void	change_active_interface();

protected:
    string		_current_ifname;	// ifname of commands current
    						// commands queue
    						// last command output
    CmdList		_future_cmds;		// commands with ifname not
						// matching _current_ifname
    CmdList		_current_cmds;
};


/**
 * @short Class to convert an IfMgrIfTree object into a sequence of commands.
 */
class IfMgrIfTreeToCommands {
public:
    /**
     * Constructor
     */
    IfMgrIfTreeToCommands(const IfMgrIfTree& tree)
	: _tree(tree)
    {}

    /**
     * Convert the entire contents of IfMgrIfTree object to a sequence of
     * configuration commands.
     *
     * @param sink output target for commands that would generate tree.
     */
    void convert(IfMgrCommandSinkBase& sink) const;

protected:
    const IfMgrIfTree& _tree;
};

/**
 * @short Class to convert an IfMgrIfAtom object into a sequence of commands.
 */
class IfMgrIfAtomToCommands {
public:
    /**
     * Constructor
     */
    IfMgrIfAtomToCommands(const IfMgrIfAtom& interface)
	: _i(interface)
    {}

    /**
     * Convert the entire contents of IfMgrIfTree object to a sequence of
     * configuration commands.
     *
     * @param sink output target for commands that would generate
     * interface state.
     */
    void convert(IfMgrCommandSinkBase& sink) const;

protected:
    const IfMgrIfAtom& _i;
};

/**
 * @short Class to convert an IfMgrVifAtom object into a sequence of commands.
 */
class IfMgrVifAtomToCommands {
public:
    /**
     * Constructor
     *
     * @param ifn the name of the interface the vif belongs to.
     * @param vif the vif to be converted into a sequence of commands.
     */
    IfMgrVifAtomToCommands(const string& ifn, const IfMgrVifAtom& vif)
	: _ifn(ifn), _v(vif)
    {}

    /**
     * Convert the entire contents of IfMgrIfTree object to a sequence of
     * configuration commands.
     *
     * @param sink output target for commands that would generate
     * virtual interface state.
     */
    void convert(IfMgrCommandSinkBase& sink) const;

protected:
    const string&	_ifn;		// Interface name
    const IfMgrVifAtom&	_v;
};

/**
 * @short Class to convert an IfMgrIPv4Atom object into a sequence of commands.
 */
class IfMgrIPv4AtomToCommands {
public:
    /**
     * Constructor
     *
     * @param ifn the name of the interface the vif owning the address
     *            belongs to.
     * @param vifn the name of the vif owning the address.
     * @param a address atom to be converted into a sequence of commands.
     */
    IfMgrIPv4AtomToCommands(const string& ifn,
			    const string& vifn,
			    const IfMgrIPv4Atom& a)
	: _ifn(ifn), _vifn(vifn), _a(a)
    {}

    /**
     * Convert the entire contents of IfMgrIfTree object to a sequence of
     * configuration commands.
     *
     * @param sink output target for commands that would generate
     * interface state.
     */
    void convert(IfMgrCommandSinkBase& sink) const;

protected:
    const string& _ifn;
    const string& _vifn;
    const IfMgrIPv4Atom& _a;
};

#ifdef HAVE_IPV6
/**
 * @short Class to convert an IfMgrIPv6Atom object into a sequence of commands.
 */
class IfMgrIPv6AtomToCommands {
public:
    /**
     * Constructor
     *
     * @param ifn the name of the interface the vif owning the address
     *            belongs to.
     * @param vifn the name of the vif owning the address.
     * @param a address atom to be converted into a sequence of commands.
     */
    IfMgrIPv6AtomToCommands(const string& ifn,
			    const string& vifn,
			    const IfMgrIPv6Atom& a)
	: _ifn(ifn), _vifn(vifn), _a(a)
    {}

    /**
     * Convert the entire contents of IfMgrIfTree object to a sequence of
     * configuration commands.
     *
     * @param sink output target for commands that would generate
     * interface state.
     */
    void convert(IfMgrCommandSinkBase& sink) const;

protected:
    const string&	 _ifn;
    const string&	 _vifn;
    const IfMgrIPv6Atom& _a;
};
#endif //ipv6

#endif // __IFMGR_CMD_QUEUE_HH__
