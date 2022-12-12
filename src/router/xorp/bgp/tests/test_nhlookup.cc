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



#include "bgp_module.h"

#include "libxorp/xorp.h"
#include "libxorp/xorpfd.hh"
#include "libxorp/eventloop.hh"
#include "libxorp/xlog.h"
#include "libxorp/test_main.hh"

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#include "bgp.hh"
#include "route_table_nhlookup.hh"
#include "route_table_debug.hh"
#include "path_attribute.hh"
#include "local_data.hh"


template <class A>
class DummyResolver : public NextHopResolver<A> {
public:
    DummyResolver(EventLoop& eventloop, BGPMain& bgp) 
	: NextHopResolver<A>(0, eventloop, bgp) 
    {
	_ofile = NULL;
	_response = false;
    }
    void set_output_file(FILE *file) { _ofile = file;}
    void set_response(bool response) {_response = response;}
    bool register_nexthop(A nexthop, IPNet<A> net, 
			  NhLookupTable<A> */*requester*/) {
	fprintf(_ofile, "[NH REGISTER]\n");
	fprintf(_ofile, "NextHop: %s\n", nexthop.str().c_str());
	fprintf(_ofile, "Net: %s\n", net.str().c_str());
	return _response;
    }
    void deregister_nexthop(A nexthop, IPNet<A> net, 
			    NhLookupTable<A> */*requester*/) {
	fprintf(_ofile, "[NH DEREGISTER]\n");
	fprintf(_ofile, "NextHop: %s\n", nexthop.str().c_str());
	fprintf(_ofile, "Net: %s\n", net.str().c_str());
    }
private:
    FILE *_ofile;
    bool _response;
};

bool
test_nhlookup(TestInfo& /*info*/)
{
#ifndef HOST_OS_WINDOWS
    struct passwd *pwd = getpwuid(getuid());
    string filename = "/tmp/test_nhlookup.";
    filename += pwd->pw_name;
#else
    char *tmppath = (char *)malloc(256);
    GetTempPathA(256, tmppath);
    string filename = string(tmppath) + "test_nhlookup";
    free(tmppath);
#endif

    EventLoop eventloop;
    BGPMain bgpmain(eventloop);
    LocalData localdata(bgpmain.eventloop());
    Iptuple iptuple;
    BGPPeerData *pd1 = new BGPPeerData(localdata, iptuple, AsNum(0), IPv4(),0);
    BGPPeer peer1(&localdata, pd1, NULL, &bgpmain);
    PeerHandler handler1("test1", &peer1, NULL, NULL);
    BGPPeerData *pd2 = new BGPPeerData(localdata, iptuple, AsNum(0), IPv4(),0);
    BGPPeer peer2(&localdata, pd2, NULL, &bgpmain);
    PeerHandler handler2("test2", &peer2, NULL, NULL);

    DummyResolver<IPv4> nh_resolver(bgpmain.eventloop(), bgpmain);

    //trivial plumbing
    NhLookupTable<IPv4> *nhlookup_table
	= new NhLookupTable<IPv4>("NHLOOKUP", SAFI_UNICAST, 
				  &nh_resolver, NULL);
    DebugTable<IPv4>* debug_table
	 = new DebugTable<IPv4>("D1", (BGPRouteTable<IPv4>*)nhlookup_table);
    nhlookup_table->set_next_table(debug_table);

    debug_table->set_output_file(filename);
    debug_table->set_canned_response(ADD_USED);

    //use the same output file as debug table
    nh_resolver.set_output_file(debug_table->output_file());

    //create a load of attributes 
    IPNet<IPv4> net1("1.0.1.0/24");
    IPNet<IPv4> net2("1.0.2.0/24");

    IPv4 nexthop1("2.0.0.1");
    NextHopAttribute<IPv4> nhatt1(nexthop1);

    IPv4 nexthop2("2.0.0.2");
    NextHopAttribute<IPv4> nhatt2(nexthop2);

    IPv4 nexthop3("2.0.0.3");
    NextHopAttribute<IPv4> nhatt3(nexthop3);

    OriginAttribute igp_origin_att(IGP);

    ASPath aspath1;
    aspath1.prepend_as(AsNum(1));
    aspath1.prepend_as(AsNum(2));
    aspath1.prepend_as(AsNum(3));
    ASPathAttribute aspathatt1(aspath1);

    ASPath aspath2;
    aspath2.prepend_as(AsNum(4));
    aspath2.prepend_as(AsNum(5));
    aspath2.prepend_as(AsNum(6));
    ASPathAttribute aspathatt2(aspath2);

    ASPath aspath3;
    aspath3.prepend_as(AsNum(7));
    aspath3.prepend_as(AsNum(8));
    aspath3.prepend_as(AsNum(9));
    ASPathAttribute aspathatt3(aspath3);

    FPAList4Ref fpalist1 =
	new FastPathAttributeList<IPv4>(nhatt1, aspathatt1, igp_origin_att);
    PAListRef<IPv4> palist1 = new PathAttributeList<IPv4>(fpalist1);

    FPAList4Ref fpalist2 =
	new FastPathAttributeList<IPv4>(nhatt2, aspathatt2, igp_origin_att);
    PAListRef<IPv4> palist2 = new PathAttributeList<IPv4>(fpalist2);

    FPAList4Ref fpalist3 =
	new FastPathAttributeList<IPv4>(nhatt3, aspathatt3, igp_origin_att);
    PAListRef<IPv4> palist3 = new PathAttributeList<IPv4>(fpalist3);

    SubnetRoute<IPv4> *sr1, *sr2, *sr3;
    InternalMessage<IPv4> *msg, *msg2;

    //================================================================
    //Test1: trivial add and delete, nexthop resolves
    //================================================================
    //add a route
    debug_table->write_comment("TEST 1");
    debug_table->write_comment("ADD AND DELETE, NEXTHOP RESOLVES");

    //pretend we have already resolved the nexthop
    nh_resolver.set_response(true);

    sr1 = new SubnetRoute<IPv4>(net1, palist1, NULL);
    msg = new InternalMessage<IPv4>(sr1, &handler1, 0);
    nhlookup_table->add_route(*msg, NULL);
    sr1->unref();
    delete msg;

    debug_table->write_separator();

    //delete the route
    sr1 = new SubnetRoute<IPv4>(net1, palist1, NULL);
    msg = new InternalMessage<IPv4>(sr1, &handler1, 0);
    nhlookup_table->delete_route(*msg, NULL);

    debug_table->write_separator();
    sr1->unref();
    delete msg;


    //================================================================
    //Test1a: trivial add and delete, nexthop doesn't yet resolve
    //================================================================
    //add a route
    debug_table->write_comment("TEST 1a");
    debug_table->write_comment("ADD AND DELETE, NEXTHOP NOT YET RESOLVED");

    //pretend we have already resolved the nexthop
    nh_resolver.set_response(false);

    sr1 = new SubnetRoute<IPv4>(net1, palist1, NULL);
    msg = new InternalMessage<IPv4>(sr1, &handler1, 0);
    nhlookup_table->add_route(*msg, NULL);
    delete msg;
    sr1->unref();

    debug_table->write_separator();

    //delete the route
    sr1 = new SubnetRoute<IPv4>(net1, palist1, NULL);
    msg = new InternalMessage<IPv4>(sr1, &handler1, 0);
    nhlookup_table->delete_route(*msg, NULL);

    debug_table->write_separator();
    delete msg;
    sr1->unref();

    //================================================================
    //Test1b: trivial add and delete, nexthop doesn't yet resolve
    //================================================================
    //add a route
    debug_table->write_comment("TEST 1b");
    debug_table->write_comment("ADD AND DELETE, NEXTHOP NOT YET RESOLVED");

    //pretend we have not yet resolved the nexthop
    nh_resolver.set_response(false);

    sr1 = new SubnetRoute<IPv4>(net1, palist1, NULL);
    msg = new InternalMessage<IPv4>(sr1, &handler1, 0);
    nhlookup_table->add_route(*msg, NULL);
    //Note: don't sr1->unref() here - the NhLookupTable assumes the
    //subnet route is still held in the RIB-In.
    delete msg;

    debug_table->write_separator();

    debug_table->write_comment("THEN NEXTHOP RESOLVES");

    set <IPNet<IPv4> > nets;
    nets.insert(net1);
    nhlookup_table->RIB_lookup_done(nexthop1, nets, true);
    nets.erase(nets.begin());
    assert(nets.empty());
    
    debug_table->write_separator();

    debug_table->write_comment("NOW DELETE THE ROUTE");
    //delete the route
    msg = new InternalMessage<IPv4>(sr1, &handler1, 0);
    nhlookup_table->delete_route(*msg, NULL);

    debug_table->write_separator();
    sr1->unref();
    delete msg;

    //================================================================
    //Test1c: trivial add and delete, nexthop doesn't yet resolve.  
    // Resolve two routes that use the same nexthop
    //================================================================
    debug_table->write_comment("TEST 1c");
    debug_table->write_comment("ADDx2 AND DELETE, NEXTHOP NOT YET RESOLVED");

    //pretend we have not yet resolved the nexthop
    nh_resolver.set_response(false);

    sr1 = new SubnetRoute<IPv4>(net1, palist1, NULL);
    msg = new InternalMessage<IPv4>(sr1, &handler1, 0);
    nhlookup_table->add_route(*msg, NULL);
    //Note: don't delete sr1 here - the NhLookupTable assumes the
    //subnet route is still held in the RIB-In.
    delete msg;

    sr2 = new SubnetRoute<IPv4>(net2, palist1, NULL);
    msg = new InternalMessage<IPv4>(sr2, &handler1, 0);
    nhlookup_table->add_route(*msg, NULL);
    //Note: don't delete sr2 here - the NhLookupTable assumes the
    //subnet route is still held in the RIB-In.
    delete msg;

    debug_table->write_separator();

    debug_table->write_comment("THEN NEXTHOP RESOLVES");

    //tell NhLookupTable the nexthop now resolves
    nets.insert(net1);
    nets.insert(net2);
    nhlookup_table->RIB_lookup_done(nexthop1, nets, true);
    while (!nets.empty())
	nets.erase(nets.begin());
    
    debug_table->write_separator();

    debug_table->write_comment("NOW DELETE THE ROUTES");
    //delete the routes
    msg = new InternalMessage<IPv4>(sr1, &handler1, 0);
    nhlookup_table->delete_route(*msg, NULL);
    sr1->unref();
    delete msg;

    msg = new InternalMessage<IPv4>(sr2, &handler1, 0);
    nhlookup_table->delete_route(*msg, NULL);
    sr2->unref();
    delete msg;

    debug_table->write_separator();

    //================================================================
    //Test2: add, nexthop doesn't yet resolve, replace, resolve, delete
    //================================================================
    debug_table->write_comment("TEST 2");
    debug_table->write_comment("ADD, REPLACE, RESOLVE, DELETE");

    //pretend we have not yet resolved the nexthop
    nh_resolver.set_response(false);

    sr1 = new SubnetRoute<IPv4>(net1, palist1, NULL);
    msg = new InternalMessage<IPv4>(sr1, &handler1, 0);
    debug_table->write_comment("ADD");
    nhlookup_table->add_route(*msg, NULL);
    //Note: don't delete sr1 here - the NhLookupTable assumes the
    //subnet route is still held in the RIB-In.
    delete msg;

    debug_table->write_separator();
 
    msg = new InternalMessage<IPv4>(sr1, &handler1, 0);
    sr2 = new SubnetRoute<IPv4>(net1, palist2, NULL);
    msg2 = new InternalMessage<IPv4>(sr2, &handler1, 0); 
    debug_table->write_comment("REPLACE");
    nhlookup_table->replace_route(*msg, *msg2, NULL);
    //Note: don't delete sr2 here - the NhLookupTable assumes the
    //subnet route is still held in the RIB-In.
    sr1->unref();
    delete msg;
    delete msg2;

    debug_table->write_separator();

    debug_table->write_comment("THEN NEXTHOP RESOLVES");

    nets.insert(net1);
    nhlookup_table->RIB_lookup_done(nexthop2, nets, true);
    nets.erase(nets.begin());
    assert(nets.empty());
    
    debug_table->write_separator();

    debug_table->write_comment("NOW DELETE THE ROUTE");
    //delete the route
    msg = new InternalMessage<IPv4>(sr2, &handler1, 0);
    nhlookup_table->delete_route(*msg, NULL);

    debug_table->write_separator();
    sr2->unref();
    delete msg;

    //================================================================
    //Test2b: add, nexthop doesn't yet resolve, replace, resolve, delete
    //================================================================
    debug_table->write_comment("TEST 2b");
    debug_table->write_comment("ADD, REPLACE (OLD IS DELETED), RESOLVE, DELETE");

    //pretend we have not yet resolved the nexthop
    nh_resolver.set_response(false);

    sr1 = new SubnetRoute<IPv4>(net1, palist1, NULL);
    msg = new InternalMessage<IPv4>(sr1, &handler1, 0);
    debug_table->write_comment("ADD");
    nhlookup_table->add_route(*msg, NULL);
    //Note: don't sr1->unref() here - the NhLookupTable assumes the
    //subnet route is still held in the RIB-In.
    delete msg;

    debug_table->write_separator();
 
    sr3 = new SubnetRoute<IPv4>(net1, palist1, NULL);
    msg = new InternalMessage<IPv4>(sr3, &handler1, 0);
    //delete the old route, as might happen with a replace route on RibIn
    sr1->unref();
    sr2 = new SubnetRoute<IPv4>(net1, palist2, NULL);
    msg2 = new InternalMessage<IPv4>(sr2, &handler1, 0); 
    debug_table->write_comment("REPLACE");
    nhlookup_table->replace_route(*msg, *msg2, NULL);
    //Note: don't sr2->unref() here - the NhLookupTable assumes the
    //subnet route is still held in the RIB-In.
    sr3->unref();
    delete msg;
    delete msg2;

    debug_table->write_separator();

    debug_table->write_comment("THEN NEXTHOP RESOLVES");

    nets.insert(net1);
    nhlookup_table->RIB_lookup_done(nexthop2, nets, true);
    nets.erase(nets.begin());
    assert(nets.empty());
    
    debug_table->write_separator();

    debug_table->write_comment("NOW DELETE THE ROUTE");
    //delete the route
    msg = new InternalMessage<IPv4>(sr2, &handler1, 0);
    nhlookup_table->delete_route(*msg, NULL);

    debug_table->write_separator();
    sr2->unref();
    delete msg;

    //================================================================
    //Test3: replace, nexthop doesn't yet resolve, replace, resolve, delete
    //================================================================
    //add a route
    debug_table->write_comment("TEST 3");
    debug_table->write_comment("REPLACE, REPLACE, RESOLVE, DELETE");

    //We'll start off by sending a replace.  This is so we can get the
    //replace into the queue, and then send a replace to replace the
    //replace.  But it also tests the case where the previous nexthop
    //did resolve and now the new one doesn't.

    nh_resolver.set_response(false);

    sr1 = new SubnetRoute<IPv4>(net1, palist1, NULL);
    sr2 = new SubnetRoute<IPv4>(net1, palist2, NULL);
    msg = new InternalMessage<IPv4>(sr1, &handler1, 0);
    msg2 = new InternalMessage<IPv4>(sr2, &handler1, 0);
    debug_table->write_comment("FIRST REPLACE");
    nhlookup_table->replace_route(*msg, *msg2, NULL);
    //Note: don't sr2->unref() here - the NhLookupTable assumes the
    //subnet route is still held in the RIB-In.
    delete msg;
    delete msg2;
    sr1->unref();
    sr2->unref();

    debug_table->write_separator();
 
    sr1 = new SubnetRoute<IPv4>(net1, palist2, NULL);
    sr3 = new SubnetRoute<IPv4>(net1, palist3, NULL);
    msg = new InternalMessage<IPv4>(sr1, &handler1, 0);
    msg2 = new InternalMessage<IPv4>(sr3, &handler1, 0); 
    debug_table->write_comment("SECOND REPLACE");
    nhlookup_table->replace_route(*msg, *msg2, NULL);
    //Note: don't sr2->unref() here - the NhLookupTable assumes the
    //subnet route is still held in the RIB-In.
    sr1->unref();
    delete msg;
    delete msg2;

    debug_table->write_separator();

    debug_table->write_comment("THEN NEXTHOP RESOLVES");

    nets.insert(net1);
    nhlookup_table->RIB_lookup_done(nexthop3, nets, true);
    nets.erase(nets.begin());
    assert(nets.empty());
    
    debug_table->write_separator();

    debug_table->write_comment("NOW DELETE THE ROUTE");
    //delete the route
    msg = new InternalMessage<IPv4>(sr3, &handler1, 0);
    nhlookup_table->delete_route(*msg, NULL);

    debug_table->write_separator();
    sr3->unref();
    delete msg;

    //================================================================
    //Test4: checking push is propagated properly.
    // trivial add and delete, nexthop doesn't yet resolve.  
    // Resolve two routes that use the same nexthop
    //================================================================
    debug_table->write_comment("TEST 4");
    debug_table->write_comment("TESTING PUSH");
    debug_table->write_comment("ADDx2 AND DELETE, NEXTHOP NOT YET RESOLVED");

    //pretend we have not yet resolved the nexthop
    nh_resolver.set_response(false);

    sr1 = new SubnetRoute<IPv4>(net1, palist1, NULL);
    msg = new InternalMessage<IPv4>(sr1, &handler1, 0);
    msg->set_push();
    nhlookup_table->add_route(*msg, NULL);
    //Note: don't delete sr1 here - the NhLookupTable assumes the
    //subnet route is still held in the RIB-In.
    delete msg;

    sr2 = new SubnetRoute<IPv4>(net2, palist1, NULL);
    msg = new InternalMessage<IPv4>(sr2, &handler1, 0);
    nhlookup_table->add_route(*msg, NULL);
    //Note: don't delete sr2 here - the NhLookupTable assumes the
    //subnet route is still held in the RIB-In.
    delete msg;

    debug_table->write_comment("SEND PUSH");
    nhlookup_table->push(NULL);

    debug_table->write_separator();

    debug_table->write_comment("THEN NEXTHOP RESOLVES");

    //tell NhLookupTable the nexthop now resolves
    nets.insert(net1);
    nets.insert(net2);
    nhlookup_table->RIB_lookup_done(nexthop1, nets, true);
    while (!nets.empty())
	nets.erase(nets.begin());
    
    debug_table->write_separator();

    debug_table->write_comment("NOW DELETE THE ROUTES");
    //delete the routes
    msg = new InternalMessage<IPv4>(sr1, &handler1, 0);
    nhlookup_table->delete_route(*msg, NULL);
    sr1->unref();
    delete msg;

    msg = new InternalMessage<IPv4>(sr2, &handler1, 0);
    msg->set_push();
    nhlookup_table->delete_route(*msg, NULL);
    sr2->unref();
    delete msg;

    debug_table->write_comment("SEND PUSH AGAIN");
    nhlookup_table->push(NULL);

    debug_table->write_separator();

    //================================================================
    //Test5: trivial add when resolves, replace with one that doesn't
    //resolve, delete
    //================================================================
    //add a route
    debug_table->write_comment("TEST 5");
    debug_table->write_comment("ADD, REPLACE WITH UNRESOLVABLE, DELETE");

    //pretend we have resolved the nexthop
    nh_resolver.set_response(true);

    sr1 = new SubnetRoute<IPv4>(net1, palist1, NULL);
    msg = new InternalMessage<IPv4>(sr1, &handler1, 0);
    nhlookup_table->add_route(*msg, NULL);
    delete msg;

    debug_table->write_separator();

    //pretend it now doesn't resolve
    nh_resolver.set_response(false);

    msg = new InternalMessage<IPv4>(sr1, &handler1, 0);
    sr2 = new SubnetRoute<IPv4>(net1, palist2, NULL);
    msg2 = new InternalMessage<IPv4>(sr2, &handler1, 0); 
    debug_table->write_comment("REPLACE");
    nhlookup_table->replace_route(*msg, *msg2, NULL);
    delete msg;
    delete msg2;
    //delete sr1 now, as that would have now been deleted from the
    //RibIn.  sr2 must remain, as the nexthop code assumes the route
    //is still stored upstream.
    sr1->unref();

    debug_table->write_separator();

    debug_table->write_comment("NOW DELETE THE ROUTE");
    //delete the route
    msg = new InternalMessage<IPv4>(sr2, &handler1, 0);
    nhlookup_table->delete_route(*msg, NULL);

    debug_table->write_separator();
    sr2->unref();
    delete msg;

    //================================================================
    //Check debug output against reference
    //================================================================

    debug_table->write_separator();
    debug_table->write_comment("SHUTDOWN AND CLEAN UP");
    delete nhlookup_table;
    delete debug_table;
    palist1.release();
    palist2.release();
    palist3.release();
    fpalist1 = 0;
    fpalist2 = 0;
    fpalist3 = 0;


    FILE *file = fopen(filename.c_str(), "r");
    if (file == NULL) {
	fprintf(stderr, "Failed to read %s\n", filename.c_str());
	fprintf(stderr, "TEST NHLOOKUP FAILED\n");
	fclose(file);
	return false;
    }
#define BUFSIZE 8192
    char testout[BUFSIZE];
    memset(testout, 0, BUFSIZE);
    int bytes1 = fread(testout, 1, BUFSIZE, file);
    if (bytes1 == BUFSIZE) {
	fprintf(stderr, "Output too long for buffer\n");
	fprintf(stderr, "TEST NHLOOKUP FAILED\n");
	fclose(file);
	return false;
    }
    fclose(file);

    string ref_filename;
    const char* srcdir = getenv("srcdir");
    if (srcdir) {
	ref_filename = string(srcdir); 
    } else {
	ref_filename = ".";
    }
    ref_filename += "/test_nhlookup.reference";
    file = fopen(ref_filename.c_str(), "r");
    if (file == NULL) {
	fprintf(stderr, "Failed to read %s\n", ref_filename.c_str());
	fprintf(stderr, "TEST NHLOOKUP FAILED\n");
	fclose(file);
	return false;
    }

    char refout[BUFSIZE];
    memset(refout, 0, BUFSIZE);
    int bytes2 = fread(refout, 1, BUFSIZE, file);
    if (bytes2 == BUFSIZE) {
	fprintf(stderr, "Output too long for buffer\n");
	fprintf(stderr, "TEST NHLOOKUP FAILED\n");
	fclose(file);
	return false;
    }
    fclose(file);
    
    if ((bytes1 != bytes2) || (memcmp(testout, refout, bytes1)!= 0)) {
	fprintf(stderr, "Output in %s doesn't match reference output\n",
		filename.c_str());
	fprintf(stderr, "TEST NHLOOKUP FAILED\n");
	return false;
	
    }
#ifndef HOST_OS_WINDOWS
    unlink(filename.c_str());
#else
    DeleteFileA(filename.c_str());
#endif
    return true;
}


