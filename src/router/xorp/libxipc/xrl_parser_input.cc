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



#include "xrl_module.h"
#include "libxorp/xorp.h"

#include "libxorp/c_format.hh"
#include "libxorp/utils.hh"

#include "xrl_parser_input.hh"

// CPP # directives that are supported here
static const int OPENING_FILE 	   = 1;
static const int RETURNING_TO_FILE = 2;

bool
XrlParserFileInput::eof() const
{
    return _stack[0].input()->eof() && _inserted_lines.empty();
}

bool
XrlParserFileInput::slurp_line(string& line)
    throw (XrlParserInputException)
{
    // Check if we need to step down a stack level
    if (stack_top().input()->eof()) {
	if (stack_depth() > 1) {
	    close_input(stack_top().input());
	    pop_stack();
	    line = c_format("# %d \"%s\" %d", stack_top().line(),
			 stack_top().filename(), RETURNING_TO_FILE);
	    _inserted_lines.push_back("");
	    return true;
	} else {
	    line = "";
	    return false;
	}
    }

    stack_top().incr_line();
#ifdef XORP_USE_USTL
    {
	// Lame as can be, but uSTL doesn't have working getline(), so
	// fake one out here..
	line.clear();
	char c[2];
	while (stack_top().input()->remaining() > 0) {
	    stack_top().input()->read(&c[0], 1);
	    c[1] = 0;
	    if (c[0] == '\n')
		break;
	    line.append(c);
	}
    }
#else
    std::getline(*stack_top().input(), line);
#endif
    // Okay, got line see if it's a pre-processor directive
    for (string::const_iterator c = line.begin();
	 c != line.end(); c++) {
	if (xorp_isspace(*c)) continue;
	if ('#' == *c) {
	    // The following may throw an exception
	    line = try_include(c, line.end());
	}
	break;
    }
    return true;
}

static string
chomp(const string& input, const string& exclude = string(" \t"))
{
    string r;
    for (string::const_iterator i = input.begin(); i != input.end(); ++i) {
	if (exclude.find(*i) == string::npos) {
	    r += *i;
	}
    }
    return r;
}

string
XrlParserFileInput::try_include(string::const_iterator& begin,
				 const string::const_iterator& end)
    throw (XrlParserInputException)
{
    static const string h("#include");
    for (string::const_iterator hi = h.begin(); hi != h.end(); hi++, begin++) {
	if (begin == end || *begin != *hi)
	    xorp_throw(XrlParserInputException, "Unsupported # directive");
    }

    // Okay found include directive skip space
    while (begin != end && xorp_isspace(*begin))
	begin++;

    // Find quote
    char qc = 0;
    string::const_iterator fn_start;
    for (fn_start = begin; fn_start <= end; fn_start++) {
	if (*fn_start == '"') {
	    fn_start++;
	    qc = '"';
	    break;
	} else if (*fn_start == '<') {
	    fn_start++;
	    qc = '>';
	    break;
	}
    }

    // Find ending quote
    string::const_iterator fn_end;
    for (fn_end = fn_start; fn_end <= end; fn_end++) {
	if (*fn_end == qc) {
	    break;
	}
    }

    if (fn_end >= end) {
	xorp_throw(XrlParserInputException, "Malformed #include directive");
    }

    // Check for junk following end of filename

    for (string::const_iterator junk = fn_end + 1; junk < end; junk++) {
	if (!xorp_isspace(*junk)) {
	    xorp_throw (XrlParserInputException,
			"Junk following filename in #include directive");
	}
    }

    string filename(fn_start, fn_end);
    push_stack(FileState(path_open_input(filename.c_str()), filename.c_str()));
    return c_format("# %d \"%s\" %d", 1, filename.c_str(), OPENING_FILE);
}

string
XrlParserFileInput::stack_trace() const
{
    string s;
    for (size_t i = 0; i < _stack.size(); i++) {
	s += string("  ", i);
	s += c_format("From file \"%s\" line %d\n",
		      _stack[i].filename(), _stack[i].line());
    }
    return s;
}

ifstream*
XrlParserFileInput::path_open_input(const char* filename)
    throw (XrlParserInputException)
{
    // XXX We could check for recursive includes here
    
    if (filename == 0)
	return 0;

    if (is_absolute_path(filename)) {
	ifstream* pif = new ifstream(filename);
	if (pif->good()) {
	    return pif;
	}
	delete pif;
    } else {
	// A relative path has been specified, therefore try to locate
	// the file under one of our search paths.
	string filename_s = unix_path_to_native(filename);

	for (list<string>::const_iterator pi = _path.begin();
	     pi != _path.end(); pi++) {
	    const string& path = *pi;

	    if (path.size() == 0)
		continue;

	    string path_file;

	    // Eliminate trailing slash from appended path, if any.
	    if (path[path.size() - 1] == PATH_DELIMITER_CHAR) {
		path_file = path + filename_s;
	    } else {
		path_file = path + PATH_DELIMITER_STRING + filename_s;
	    }

	    ifstream* pif = new ifstream(path_file.c_str());
	    if (pif->good()) {
		return pif;
	    }
	    delete pif;
	}
    }
    xorp_throw(XrlParserInputException, c_format("Could not open \"%s\": %s",
					  filename, strerror(errno)));
    return 0;
}

void
XrlParserFileInput::close_input(istream* pif)
{
    delete pif;
}

XrlParserFileInput::XrlParserFileInput(istream* input, const char* fname)
    throw (XrlParserInputException)
    :  _own_bottom(false), _current_mode(NORMAL)
{
    initialize_path();
    push_stack(FileState(input, fname));
    _inserted_lines.push_back(c_format("# 1 \"%s\"", fname));
}

XrlParserFileInput::XrlParserFileInput(const char* fname)
    throw (XrlParserInputException)
    :  _own_bottom(true), _current_mode(NORMAL)
{
    initialize_path();
    push_stack(FileState(path_open_input(fname), fname));
    _inserted_lines.push_back(c_format("# 1 \"%s\"", fname));
}

XrlParserFileInput::~XrlParserFileInput()
{
    while (stack_depth() > 1) {
	close_input(stack_top().input());
	pop_stack();
    }
    if (_own_bottom) {
	close_input(stack_top().input());
    }
}

void
XrlParserFileInput::initialize_path()
{
    _path.push_back(".");
}

void
XrlParserFileInput::push_stack(const FileState& fs)
    throw (XrlParserInputException)
{
    if (fs.input()->good() == false) {
	xorp_throw (XrlParserInputException, "Bad ifstream, rejected by stack");
    }
    _stack.push_back(fs);
}

void
XrlParserFileInput::pop_stack()
{
    if (_stack.size())
	_stack.pop_back();
}

XrlParserFileInput::FileState&
XrlParserFileInput::stack_top()
{
    assert(_stack.size() != 0);
    return _stack.back();
}

size_t
XrlParserFileInput::stack_depth() const
{
    return _stack.size();
}

/* Return true if caller should try getline() again to get useful
 * line data, false otherwise.
 */
bool
XrlParserFileInput::getline(string& line) throw (XrlParserInputException)
{
    line.clear();

    if (_inserted_lines.empty() == false) {
	line = _inserted_lines.front();
	_inserted_lines.erase(_inserted_lines.begin());
	return true;
    } else if (eof()) {
	return false;
    }

    string r;
    while (slurp_line(r)) {
	if (filter_line(line, r) == false) {
	    break;
	}
    }

    for (size_t i = 0; i < line.size(); i++) {
	if (xorp_isspace(line[i]) == false)
	    return false;	// Non blank line
    }

    line.clear();
    return true;
}

/** @return true if terminating squote found, false otherwise.
 * @sideeffect advance i to one past terminating squote.
 */
bool
advance_to_terminating_squote(string::const_iterator& i,
			      const string::const_iterator& end)
{
    while (i != end) {
	if (*i == '\'') {
	    i++;
	    return true;
	}
	i++;
    }
    return false;
}

/** @return true if terminating dquote found, false otherwise.
 * @sideeffect advance i to one past terminating squote.
 */
bool
advance_to_terminating_dquote(string::const_iterator& i,
			      const string::const_iterator& end)
{
    if (*i == '"') {
	i++;
	return true;
    }
    const string::const_iterator last_start = end - 1;
    while (i != last_start) {
	if (*i != '\\' && *(i + 1) == '"') {
	    i += 2;
	    return true;
	}
	i++;
    }
    i = end; // didn't find dquote goto end of line
    return false;
}

/** @return true if terminating comment found, false otherwise.
 * @sideeffect advance i to one past terminating squote.
 */
bool
advance_to_terminating_c_comment(string::const_iterator& i,
				 const string::const_iterator& end)
{
    const string::const_iterator last_start = end - 1;
    while (i != last_start) {
	if (*i == '*' && *(i + 1) == '/') {
	    i += 2;
	    return true;
	}
	i++;
    }
    i = end; // didn't find comment goto end of line
    return false;
}

/** @return true if another line is required because of a continuation,
 *  false otherwise.
 */
bool
XrlParserFileInput::filter_line(string& output, const string& input)
{
    string::const_iterator ci = input.begin();
    while (ci != input.end()) {
	switch (_current_mode) {
	case NORMAL: {
	    string::const_iterator began = ci;
	    while (ci != input.end()) {
		if (*ci == '\"') {
		    _current_mode = IN_DQUOTE;
		    output += chomp(string(began, ++ci));
		    began = ci;
		    break;
		} else if (*ci == '\'') {
		    _current_mode = IN_SQUOTE;
		    output += chomp(string(began, ++ci));
		    began = ci;
		    break;
		} else if (input.end() - ci >= 2) {
		    if (*ci == '/' && *(ci + 1) == '*') {
			// Found a C comment beginning
			_current_mode = IN_C_COMMENT;
			output += chomp(string(began, ci)) + string(" ");
			ci += 2;
			began = ci;
			break;
		    } else if (*ci == '/' && *(ci + 1) == '/' &&
			       (ci == input.begin() ||
				xorp_isspace(*(ci - 1)))) {
			// Found a c++ comment
			began = ci;
			break;
		    }
		} else if (input.end() - ci == 1 && *ci == '\\') {
		    output += chomp(string(began, ci));
		    return true;
		}
		ci++;
	    }
	    // do a copy from began to ci
	    output += chomp(string(began, ci));
	    break;
	}
	case IN_SQUOTE: {
	    string::const_iterator began = ci;
	    if (advance_to_terminating_squote(ci, input.end())) {
		_current_mode = NORMAL;
	    }
	    output += string(began, ci);
	    break;
	}
	case IN_DQUOTE: {
	    string::const_iterator began = ci;
	    if (advance_to_terminating_dquote(ci, input.end())) {
		_current_mode = NORMAL;
		output += string(began, ci);
	    } else if (ci == input.end() && *(ci - 1) == '\\') {
		output += string(began, ci - 1);
		return true;
	    }
	    break;
	}
	case IN_C_COMMENT:
	    if (advance_to_terminating_c_comment(ci, input.end())) {
		_current_mode = NORMAL;
	    }
	    /* do not copy output */
	    break;
	}
    }
    return false;
}

