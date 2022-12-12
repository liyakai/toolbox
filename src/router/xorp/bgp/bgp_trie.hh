// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-

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

// $XORP: xorp/bgp/bgp_trie.hh,v 1.22 2008/11/08 06:14:36 mjh Exp $

#ifndef __BGP_BGP_TRIE_HH__
#define __BGP_BGP_TRIE_HH__

#include "subnet_route.hh"

#include "libxorp/ref_trie.hh"

template <class A>
class Path_Att_Ptr_Cmp {
public:
    bool operator() (const PAListRef<A> a,
		     const PAListRef<A> b) const {
	return a < b;
    }
};

template<class A>
class ChainedSubnetRoute : public SubnetRoute<A> {
public:
    ChainedSubnetRoute(const IPNet<A> &net,
		       const PAListRef<A> attributes) :
	SubnetRoute<A>(net, attributes), _prev(0), _next(0) {}

    ChainedSubnetRoute(const SubnetRoute<A>& route,
		       const ChainedSubnetRoute<A>* prev);

    ChainedSubnetRoute(const ChainedSubnetRoute& csr);

    const ChainedSubnetRoute<A> *prev() const { return _prev; }
    const ChainedSubnetRoute<A> *next() const { return _next; }

    bool unchain() const;

protected:
    void set_next(const ChainedSubnetRoute<A> *next) const { _next = next; }

    void set_prev(const ChainedSubnetRoute<A> *prev) const { _prev = prev; }

    ChainedSubnetRoute& operator=(const ChainedSubnetRoute& csr); // Not impl.

private:
    //The destructor is private because it's not supposed to be called
    //directly. Instead, unref() should be used which will only delete
    //the SubnetRoute when the reference count reaches zero.
    friend class SubnetRoute<A>; //shut the compiler up.
    ~ChainedSubnetRoute() {}

    // it looks odd to have these be mutable and the methods to set
    // them be const, but that's because the chaining is really
    // conceptually part of the container, not the payload.  It these
    // aren't mutable we can't modify the chaining and have the payload
    // be const.
    mutable const ChainedSubnetRoute<A> *_prev;
    mutable const ChainedSubnetRoute<A> *_next;
};

/**
 * Template specialization of the RefTrieNode, so that the payload is
 * not immediately deleted unless the SubnetRoute reference count is
 * zero. 
 */
template<>
inline void
RefTrieNode<IPv4, const ChainedSubnetRoute<IPv4> >::delete_payload(const ChainedSubnetRoute<IPv4>* p)
{
    p->unref();
}

template<>
inline void
RefTrieNode<IPv6, const ChainedSubnetRoute<IPv6> >::delete_payload(const ChainedSubnetRoute<IPv6>* p)
{
    p->unref();
}

/**
 * The BgpTrie is an augmented, specialized trie that allows us to
 * lookup by network address or by path attribute list.  We need this
 * because we can't efficiently extract entries with the same path
 * attribute list from a regular trie.  Each set of nodes with the same
 * path attribute pointer are linked together into a chain (a circular
 * doubly-linked list).  The BgpTrie holds a pointer to any one of
 * those nodes.
 * 
 * XXX mjh: I've changed the pathmap to be a nexthop map.  This will still
 * allow the code to find the next route when the igp distance failed,
 * but is much faster.  The downside is we may not dump routes with
 * the same PA list together.  Hopefully this can be remedied when we
 * change to a pull-based model.
 */
template<class A>
class BgpTrie : public RefTrie<A, const ChainedSubnetRoute<A> > {
public:
    typedef ::IPNet<A> IPNet;
    typedef ::ChainedSubnetRoute<A> ChainedSubnetRoute;
    typedef map<const PAListRef<A>,
		const ChainedSubnetRoute*,
		Path_Att_Ptr_Cmp<A> > PathmapType;
    typedef RefTrie<A, const ChainedSubnetRoute> RouteTrie;
    typedef typename RouteTrie::iterator iterator;

    BgpTrie();
    ~BgpTrie();

    iterator insert(const IPNet& net, const SubnetRoute<A>& route);

    void erase(const IPNet& net);

    void delete_all_nodes();

    const PathmapType& pathmap() const { return _pathmap; }

private:
    PathmapType	_pathmap;
};

#endif // __BGP_BGP_TRIE_HH__
