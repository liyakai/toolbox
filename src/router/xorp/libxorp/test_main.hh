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

// $XORP: xorp/libxorp/test_main.hh,v 1.21 2008/10/02 21:57:34 bms Exp $

#ifndef __LIBXORP_TEST_MAIN_HH__
#define __LIBXORP_TEST_MAIN_HH__




#include "xorp.h"
#include "callback.hh"

/**
 * Macro to use to generate debugging output when the verbose flag is
 * enabled.
 */
#define DOUT(info)						\
		if (info.verbose()) 				\
			info.out() << __FUNCTION__ << ":"	\
				   << __LINE__ << ":"		\
				   << info.test_name() << ": "


/**
 * Only generate debugging output if the verbose_level is equal to or above
 * threshold.
 */
#define DOUT_LEVEL(info, level)						\
		if (info.verbose() && info.verbose_level() >= level)	\
			info.out() << __FUNCTION__ << ":"		\
				   << __LINE__ << ":"			\
				   << info.test_name() << ": "

/**
 * This class is passed as the first argument to each test function/method.
 */
class TestInfo {
public:
    TestInfo(string myname, bool verbose, int verbose_level, ostream& o) :
	_myname(myname), _verbose(verbose), _verbose_level(verbose_level),
	_ostream(o)
    {
    }

    TestInfo(const TestInfo& rhs)
	: _myname(rhs._myname),  _verbose(rhs._verbose),
	  _verbose_level(rhs._verbose_level), _ostream(rhs._ostream)
    {
    }

    /*
     * @return The name of the current test.
     */
    string test_name()
    {
	return _myname;
    }

    /*
     * @return True if the verbose flag has been enabled.
     */
    bool
    verbose()
    {
	return _verbose;
    }
    
    /*
     * @return The verbose level.
     */
    int
    verbose_level()
    {
	return _verbose_level;
    }

    /*
     * @return The stream to which output should be sent.
     */
    ostream& out()
    {
	return _ostream;
    }
private:
    string _myname;
    bool _verbose;
    int _verbose_level;
    ostream& _ostream;
};

/**
 * A helper class for test programs.
 *
 * This class is used to parse the command line arguments and return
 * the exit status from the test functions/methods. An example of how
 * to use this class can be found in test_test_main.cc.
 *
 */
class TestMain {
public:
    /**
     * Start the parsing of command line arguments.
     */
    TestMain(int argc, char * const argv[]) :
	_verbose(false), _verbose_level(0), _exit_status(true)
    {
	_progname = argv[0];

	for (int i = 1; i < argc; i++) {
	    string argname;
	    string argvalue = "";
	    Arg a;
	    // Argument flag
	    if (argv[i][0] == '-') {
		// Long form argument.
		if (argv[i][1] == '-') {
		    argname = argv[i];
		} else {
		    argname = argv[i][1];
		    argname = "-" + argname;
		    if ('\0' != argv[i][2]) {
			argvalue = &argv[i][2];
		    }
		}
		// Try and get the argument value if we don't already
		// have it.
		if ("" == argvalue && (i + 1) < argc) {
		    if (argv[i + 1][0] != '-') {
			i++;
			argvalue = argv[i];
		    }
		}
		if ("" == argvalue)
		    a = Arg(Arg::FLAG, argname);
		else
		    a = Arg(Arg::VALUE, argname, argvalue);
	    } else {
		a = Arg(Arg::REST, argv[i]);
	    }
	    _args.push_back(a);
	}
    }

    /**
     * Get an optional argument from the command line.
     *
     * @param short_form The short form of the argument e.g. "-t".
     * @param long_form The long form of the argument
     * e.g. "--testing".
     * @param description The description of this argument that will
     * be used in the usage message.
     * @return the argument or "" if not found or an error occured in
     * previous parsing of the arguments.
     */
    string
    get_optional_args(const string &short_form, const string &long_form,
		      const string &description)
    {
	_usage += short_form + "|" + long_form + " arg\t" + description + "\n";

	if (false == _exit_status)
	    return "";
	list<Arg>::iterator i;
	for (i = _args.begin(); i != _args.end(); i++) {
	    if (short_form == i->name() || long_form == i->name()) {
		bool has_value;
		string value;
		value = i->value(has_value);
		if (!has_value) {
		    _exit_status = false;
		    return "";
		}
		_args.erase(i);
		return value;
	    }
	}
	return "";
    }

    /**
     * Get an optional flag from the command line.
     *
     * @param short_form The short form of the argument e.g. "-t".
     * @param long_form The long form of the argument
     * e.g. "--testing".
     * @param description The description of this argument that will
     * be used in the usage message.
     * @return true if the flag is present or false found or an error
     * occured in previous parsing of the arguments.
     */
    bool
    get_optional_flag(const string &short_form, const string &long_form,
		      const string &description)
    {
	_usage += short_form + "|" + long_form + " arg\t" + description + "\n";

	if (false == _exit_status)
	    return false;
	list<Arg>::iterator i;
	for (i = _args.begin(); i != _args.end(); i++) {
	    if (short_form == i->name() || long_form == i->name()) {
		_args.erase(i);
		return true;
	    }
	}
	return false;
    }

    /**
     * Complete parsing the arguments.
     *
     * Process generic arguments and verify that there are no
     * arguments left unprocessed.
     */
    void
    complete_args_parsing()
    {
	_verbose = get_optional_flag("-v", "--verbose", "Verbose");

	string level = get_optional_args("-l",
					 "--verbose-level","Verbose level");
	if ("" != level)
	    _verbose_level = atoi(level.c_str());

	bool h = get_optional_flag("-h", "--help","Print help information");
	bool q = get_optional_flag("-?", "--help","Print help information");

	if (h || q) {
	    cerr << usage();
	    ::exit(-1);
	}

	if (!_args.empty()) {
	    list<Arg>::iterator i;
	    for (i = _args.begin(); i != _args.end(); i++) {
		cerr << "Unused argument: " << i->name() << endl;
	    }
	    cerr << usage();
	    _exit_status = false;
	}
    }

    /**
     * Get the state of the verbose flag. Used by test programs that
     * don't use the run method to run tests.
     */
    bool get_verbose() const { return _verbose; }

    /**
     * Get the the verbose level, should only be used if get_verbose()
     * is true. Used by test programs that don't use the run method to
     * run the tests.
     */
    int get_verbose_level() const { return _verbose_level; }

    /**
     * Run a test function/method. The test function/method is passed
     * a TestInfo. The test function/method should return
     * true for success and "false for
     * failure.
     *
     * To run a function call "test":
     * run("test", callback(test));
     *
     * @param test_name The name of the test.
     * @param cb Callback object.
     */
    void
    run(string test_name, XorpCallback1<bool, TestInfo&>::RefPtr cb)
    {
 	if (false == _exit_status)
 	    return;
//  	if (_verbose)
	    cout << "Running: " << test_name << endl;
	TestInfo info(test_name, _verbose, _verbose_level, cout);
	if (!cb->dispatch(info)) {
	    _exit_status = false;
	    cerr << "Test Failed: " << test_name << endl;
	}
	else {
	    cout << "Test Passed: " << test_name << endl;
	}
    }

    /**
     * @return The usage string.
     */
    const string
    usage()
    {
	return "Usage " + _progname + ":\n" + _usage;
    }

    /**
     * Mark the tests as having failed. Used for setting an error
     * condition from outside a test.
     *
     * @param error Error string.
     */
    void
    failed(string error)
    {
	_error_string += error;
	_exit_status = false;
    }

    /**
     * Must be called at the end of the tests.
     *
     * @return The status of the tests. Should be passed to exit().
     */
    int
    exit()
    {
	if ("" != _error_string)
	    cerr << _error_string;

	return _exit_status ? 0 : -1;
    }

private:
    class Arg;
    string _progname;
    list<Arg> _args;
    bool _verbose;
    int _verbose_level;
    bool _exit_status;
    string _error_string;
    string _usage;

    class Arg {
    public:
	typedef enum {FLAG, VALUE, REST} arg_type;
	Arg() {}

	Arg(arg_type a, string name, string value = "")
	    : _arg_type(a), _name(name), _value(value)
	{
	    // 	debug_msg("Argument type = %d flag name = %s value = %s\n",
	    // 		  a, name.c_str(), value.c_str());
	}

	Arg(const Arg& rhs)
	{
	    copy(rhs);
	}

	Arg& operator=(const Arg& rhs)
	{
	    if (&rhs == this)
		return *this;
	    copy(rhs);

	    return *this;
	}
    
	void
	copy(const Arg& rhs)
	{
	    _arg_type = rhs._arg_type;
	    _name = rhs._name;
	    _value = rhs._value;
	}

	const string&
	name()
	{
	    return _name;
	}

	const string&
	value(bool& has_value)
	{
	    if (VALUE != _arg_type) {
		cerr << "Argument " << _name <<
		    " was not provided with a value\n";
		has_value = false;
	    } else
		has_value = true;

	    return _value;
	}

    private:
	arg_type _arg_type;
	string _name;
	string _value;
    };
};

#endif // __LIBXORP_TEST_MAIN_HH__
