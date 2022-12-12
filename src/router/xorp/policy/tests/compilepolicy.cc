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



/*
 * EXIT CODES:
 *
 * 4 -- other errors.
 *
 * 3 -- bad arguments to program
 *
 * 2 -- parse error
 * 1 -- policy exception
 *
 * 0 -- no error
 *
 */

#include "policy/policy_module.h"

#include "libxorp/xorp.h"




#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "policy/configuration.hh"
#include "policy/common/policy_utils.hh"

#include "process_watch_fake.hh"
#include "filter_manager_fake.hh"


using namespace policy_utils;

ProcessWatchFake pw;
FilterManagerFake fm;

Configuration _yy_configuration(pw);

typedef Configuration::CodeMap CodeMap;
typedef Configuration::TagMap TagMap;

ofstream* code_out = NULL;



int do_parsing(const string& conf, string& outerr);




void print_code(Code& c) {
    ostream* os = &cout;

    if(code_out)
	os = code_out;

    
    *os << c.code();

    const set<string>& set_names = c.referenced_set_names();

    for(set<string>::const_iterator i = set_names.begin();
	i != set_names.end(); ++i) {
    
	*os << "SET " << *i << " \"";

	const Element& s = _yy_configuration.sets().getSet(*i);

	*os << s.str() << "\"\n";
    }	

}

void print_codemap(CodeMap& cmap) {
    for(CodeMap::iterator i = cmap.begin(); i != cmap.end(); ++i) {
	Code* c = (*i).second;

	cout << "Printing code for " << (*i).first << endl;
	print_code(*c);
    }
}

void print_sets() {
    cout << _yy_configuration.sets().str();
}

void print_tagmap() {
    TagMap& tm = _yy_configuration.tagmap();

    for(TagMap::iterator i = tm.begin(); i != tm.end(); ++i) {
	TagSet* ts = (*i).second;

	cout << "Protocol " << (*i).first << ":";

	for(TagSet::iterator iter = ts->begin();
	    iter != ts->end(); ++iter) 
	    
	    cout << " " << *iter;
	cout << endl;	
    }
}

void print_target(int,const string&);

void print_target(const string& protocol) {
    print_target(1,protocol);
    print_target(2,protocol);
    print_target(4,protocol);
}

CodeMap& get_codemap(int filterid) {
    switch(filterid) {
	case 1:
	    return _yy_configuration.import_filters();
	
	case 2:
	    return _yy_configuration.sourcematch_filters();

	case 4:
	    return _yy_configuration.export_filters();
	
	default:
	    cout << "Filterid " << filterid << " Unknown\n";
	    abort();
    }
}

void print_target(int filterid) {
    CodeMap& cm = get_codemap(filterid);

    for(CodeMap::iterator i = cm.begin();
	i != cm.end(); ++i) {
    
	Code* c = (*i).second;

	print_code(*c);
    }	
}

void print_target(int filterid, const string& protocol) {

    if(filterid == -1) {
	print_target(protocol);
	return;
    }	

    if(protocol.empty()) {
	print_target(filterid);
	return;
    }	

    CodeMap& cm = get_codemap(filterid);

    CodeMap::iterator i = cm.find(protocol);
    if(i == cm.end()) {
	cout << "No code for protocol: " << protocol << " filterid: " 
	     << filterid << endl;
	return;
    }

    Code* c = (*i).second;

    print_code(*c);
}

void configure_varmap(const string& conf) {
    istringstream iss(conf);

    unsigned state = 0;

    // protocol, variable, type, access, id
    string tokens[5];

    while(!iss.eof()) {
	string token;

	// lex =D
	iss >> token;

	if(!token.length())
	    continue;
	
	tokens[state] = token;

	state++;
	
	// yacc =D
	if(state == 5) {
	    _yy_configuration.add_varmap(tokens[0], tokens[1], 
					 tokens[2], tokens[3],
					 atoi(tokens[4].c_str()));
	    state = 0;
	}
    }
}

void go(const string& fsrc, const string& fvarmap, 
	int filterid, const string& protocol) {

    string src;
    string varmapconf;

    try {
	read_file(fsrc,src);
	read_file(fvarmap,varmapconf);
    } catch(const PolicyException& e) {
	cout << "Unable to read file: " << e.str() << endl;
	exit(4);
    }
   
    _yy_configuration.set_filter_manager(fm); 
    configure_varmap(varmapconf);

    string err;

    if(do_parsing(src,err)) {
	cout << err << endl;
	exit(2);
    }

    _yy_configuration.commit(0);


    if(filterid != -1 || !protocol.empty()) {
	print_target(filterid,protocol);
	return;
    }	

    // else print it all
    cout << "Printing import filters..." << endl;
    print_codemap(_yy_configuration.import_filters());
    cout << "Printing source_match filters..." << endl;
    print_codemap(_yy_configuration.sourcematch_filters());
    cout << "Printing export filters..." << endl;
    print_codemap(_yy_configuration.export_filters());

    cout << "Printing sets..." << endl;
    print_sets();
    cout << "Printing tagmap..." << endl;
    print_tagmap();
}

void usage(const char* x) {
    cout << "Usage: " << x << " <opts>\n";
    
    cout << "-h\t\tthis help message\n";
    cout << "-s <file>\tsource file of policy\n";
    cout << "-m <varmap>\tfile with policy variables mapping\n";
    cout << "-f <filterid>\ttarget filter to produce code for\n";
    cout << "-p <protocol>\ttarget protocol to produce code for\n";
    cout << "-o <outfile>\tfile where generated code should be stored\n";
    exit(3);
}

int main(int argc, char *argv[]) {
    int filterid = -1;
    string protocol("");
    string source_file("");
    string policy_var_map_file("");

    if(argc < 2) 
	usage(argv[0]);


    int ch;

    while( (ch = getopt(argc,argv,"hs:m:p:f:o:")) != -1) {
	switch(ch) {
	    case 's':
		source_file = optarg;
		break;

	    case 'm':
		policy_var_map_file = optarg;
		break;
	    
	    case 'p':
		protocol = optarg;
		break;
	
	    case 'f':
		filterid = atoi(optarg);

		switch(filterid) {
		    case 1:
		    case 2:
		    case 4:
			break;
		    
		    default:
			cout << "Invalid filter id: " << filterid << endl;
			usage(argv[0]);
		}
		break;

	    // yea its a hack...
	    case 'o':
		if(code_out) {
		    cout << "out file already specified\n";
		    exit(4);
		}

		code_out = new ofstream(optarg);
		if(!code_out->is_open()) {
		    cout << "Unable to open file for writing: " 
			 << optarg << endl;
		    
		    exit(4);
		}
		break;

	    case 'h':
		usage(argv[0]);

	    default:
		usage(argv[0]);
	}
    }

    

    xlog_init(argv[0], 0);
    xlog_set_verbose(XLOG_VERBOSE_HIGH);
    xlog_add_default_output();
    xlog_start();

    if(source_file.empty()) {
	cout << "No source file specified\n\n";
	usage(argv[0]);
    }

    if (policy_var_map_file.empty()) {
	cout << "No source file specified for mapping of policy variables\n\n";
	usage(argv[0]);
    }
   
    TimeVal start;
    SystemClock sc;
    TimerList timerlist(&sc);
    TimerList::system_gettimeofday(&start); 

    try {
	go(source_file,policy_var_map_file,filterid,protocol);
    } catch(const PolicyException& e) {
	cout << "Compile FAILED" << endl;
	cout << "PolicyException: " << e.str() << endl;
	exit(1);
    }

    TimeVal elapsed;
    TimerList::system_gettimeofday(&elapsed);
    elapsed -= start; 

    if(code_out) {
	code_out->close();
	delete code_out;
    }	

    xlog_stop();
    xlog_exit();

    printf("Compile successful in %d milliseconds\n", elapsed.to_ms());

    exit(0);
}
