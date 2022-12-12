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



#include "libxorp/xorp.h"
#include "register_operations.hh"
#include "dispatcher.hh"
#include "element.hh"
#include "elem_set.hh"
#include "elem_null.hh"
#include "elem_bgp.hh"
#include "operator.hh"
#include "element_factory.hh"
#include "policy_utils.hh"


namespace operations {

// Unary operations
Element* 
op_not(const ElemBool& x)
{
    return new ElemBool(!x.val());
}

Element* 
op_head(const ElemStr& x)
{
    const string& str = x.val();

    string::size_type pos = str.find(',', 0);

    // try again
    if (pos == string::npos)
	pos = str.find(' ', 0);

    // return whole thing... [if space and comma not found]

    return new ElemStr(str.substr(0, pos));
}

// operations directly on element value
#define DEFINE_BINOP(name,op) \
template<class Result, class Left, class Right> \
Element* name(const Left& x, const Right& y) \
{ \
    return new Result(x.val() op y.val()); \
}

// We'd like partial template specialization for functions, but it's not
// standard.  We special case when a bool is returned and optimize returning
// true / false.  We return one of these global objects rather than creating a
// new one each time.
ElemBool _true(true);
ElemBool _false(false);

Element*
return_bool(bool x)
{
    Element* r = x ? &_true : &_false;

    XLOG_ASSERT(r->refcount() > 1);

    return r;
}

#define DEFINE_BINOP_BOOL(name, op) \
template<class Unused, class Left, class Right> \
Element* name(const Left& x, const Right& y) \
{ \
    bool val = x.val() op y.val(); \
    \
    return return_bool(val); \
}

DEFINE_BINOP_BOOL(op_and, &&)
DEFINE_BINOP_BOOL(op_or, ||)
DEFINE_BINOP_BOOL(op_xor, ^)

DEFINE_BINOP_BOOL(op_eq, ==)
DEFINE_BINOP_BOOL(op_ne, !=)

DEFINE_BINOP_BOOL(op_lt, <)
DEFINE_BINOP_BOOL(op_gt, >)
DEFINE_BINOP_BOOL(op_le, <=)
DEFINE_BINOP_BOOL(op_ge, >=)

DEFINE_BINOP(op_add,+)
DEFINE_BINOP(op_sub,-)
DEFINE_BINOP(op_mul,*)
DEFINE_BINOP(op_div,/)
DEFINE_BINOP(op_lshift,<<)
DEFINE_BINOP(op_rshift,>>)
DEFINE_BINOP(op_bit_and,&);
DEFINE_BINOP(op_bit_or,|);
DEFINE_BINOP(op_bit_xor,^);

// Operations for which .val() is not needed. [operation performed on element
// itself].
#define DEFINE_BINOP_NOVAL(name,op) \
template <class Result, class Left, class Right> \
Element* \
name(const Left& x, const Right& y) { \
    return new Result(x op y); \
}

DEFINE_BINOP_NOVAL(op_eq_nv,==)
DEFINE_BINOP_NOVAL(op_ne_nv,!=)

DEFINE_BINOP_NOVAL(op_lt_nv,<)
DEFINE_BINOP_NOVAL(op_gt_nv,>)
DEFINE_BINOP_NOVAL(op_le_nv,<=)
DEFINE_BINOP_NOVAL(op_ge_nv,>=)

// useful for equivalences where we want one class to deal with the operation.
// For example we want sets to know about elements, but not elements to know
// about sets.
// so Element < Set will normally be interpreted as Element::operator< but we do
// not want that... so we switch the parameters and obtain
// Set::operator>(Element)
#define DEFINE_BINOP_SWITCHPARAMS(name,op) \
template <class Result, class Left, class Right> \
Element* \
name(const Left& x, const Right& y) \
{ \
    return new Result(y op x); \
}

DEFINE_BINOP_SWITCHPARAMS(op_eq_sw,==)
DEFINE_BINOP_SWITCHPARAMS(op_ne_sw,!=)

DEFINE_BINOP_SWITCHPARAMS(op_lt_sw,>)
DEFINE_BINOP_SWITCHPARAMS(op_gt_sw,<)

DEFINE_BINOP_SWITCHPARAMS(op_le_sw,>=)
DEFINE_BINOP_SWITCHPARAMS(op_ge_sw,<=)

//
// Network related operators
//
template<class Result, class Left, class Right>
Element*
op_lt_net(const Left& x, const Right& y)
{
    bool result;

    result = (y.val().contains(x.val()) && (y.val() != x.val()));

    return return_bool(result);
}

template<class Result, class Left, class Right>
Element*
op_gt_net(const Left& x, const Right& y)
{
    bool result;

    result = (x.val().contains(y.val()) && (x.val() != y.val()));

    return return_bool(result);
}

template<class Result, class Left, class Right>
Element*
op_le_net(const Left& x, const Right& y)
{
    bool result;

    result = y.val().contains(x.val());

    return return_bool(result);
}

template<class Result, class Left, class Right>
Element*
op_ge_net(const Left& x, const Right& y)
{
    bool result;

    result = x.val().contains(y.val());

    return return_bool(result);
}

// 2 template parameters because U can be T or ElemSetAny<T>
template <class T, class U>
Element* 
set_add(const ElemSetAny<T>& s, const U& e)
{
    ElemSetAny<T>* es = new ElemSetAny<T>();

    es->insert(s);
    es->insert(e);

    return es;
}

template <class T>
Element*
set_del(const ElemSetAny<T>& s, const ElemSetAny<T>& r)
{
    ElemSetAny<T>* es = new ElemSetAny<T>();
    es->insert(s);
    es->erase(r);
    return es;
}

template <class T>
Element* 
set_ne_int(const ElemSetAny<T>& l, const ElemSetAny<T>& r)
{
    return new ElemBool(l.nonempty_intersection(r));
}

Element* 
str_add(const ElemStr& left, const ElemStr& right)
{
    return new ElemStr(left.val() + right.val());
}

Element* 
str_mul(const ElemStr& left, const ElemU32& right)
{
    string str = left.val();
    string res = "";
    uint32_t times = right.val();

    for (uint32_t i = 0; i < times; ++i)
	res.append(str);

    return new ElemStr(res);
}

Element* 
ctr_base(const ElemStr& type, const string& arg_str)
{
    ElementFactory ef;

    return ef.create(type.val(), arg_str.c_str());
}

Element* 
ctr(const ElemStr& type, const Element& arg)
{
    return ctr_base(type, arg.str());
}

template <class T>
Element* 
ctr(const ElemStr& type, const T& arg)
{
    return ctr_base(type, arg.str());
}

Element*
str_regex(const ElemStr& left, const ElemStr& right)
{
    return new ElemBool(policy_utils::regex(left.val(), right.val()));
}

Element*
str_setregex(const ElemStr& left, const ElemSetStr& right)
{
    string str = left.val();

    // go through all regexps...
    // only 1 needs to match...
    for (ElemSetStr::const_iterator i = right.begin(); i != right.end(); ++i) {
	const ElemStr& re = *i;

	if (policy_utils::regex(str, re.val()))
	    return new ElemBool(true);
    }

    return new ElemBool(false);
}

Element*
aspath_prepend(const ElemU32& left, const ElemASPath& right)
{
    ASPath* path = new ASPath(right.val());
    path->prepend_as(AsNum(left.val()));

    ElemASPath* ret = new ElemASPath(*path, true);

    return ret;
}

Element*
aspath_expand(const ElemU32& left, const ElemASPath& right)
{
    ASPath* path = new ASPath(right.val());

    if (path->path_length()) {
        const AsNum& head = path->first_asnum();

	unsigned times = left.val();
        for (unsigned i = 0; i < times; ++i)
	   path->prepend_as(head);
    }	   

    ElemASPath* ret = new ElemASPath(*path, true);
    return ret;
}

Element*
aspath_contains(const ElemASPath& left, const ElemU32& right)
{
    return new ElemBool(left.val().contains(AsNum(right.val())));
}

Element*
aspath_regex(const ElemASPath& left, const ElemStr& right)
{
    return new ElemBool(policy_utils::regex(left.val().short_str(), right.val()));
}

Element*
aspath_regex(const ElemASPath& left, const ElemSetStr& right)
{
    string str = left.val().short_str();

    // go through all regexps...
    // only 1 needs to match...
    for (ElemSetStr::const_iterator i = right.begin(); i != right.end(); ++i) {
	const ElemStr& re = *i;

	if (policy_utils::regex(str, re.val()))
	    return new ElemBool(true);
    }

    return new ElemBool(false);
}

template<class A>
bool
net_match(const ElemNet<A>& left, const ElemNet<A>& right)
{
    static Dispatcher d;
    Element* r;

    r = d.run(right.op(), left, right);

    if (r == &_true)
	return true;
    else if (r == &_false)
	return false;
    else
	abort();
}

template<class A>
Element*
net_set_match(const ElemNet<A>& left, const ElemSetAny<ElemNet<A> >& right)
{
    bool ret = false;

    for (typename ElemSetAny<ElemNet<A> >::const_iterator i = right.begin();
         i != right.end(); ++i) {

	const ElemNet<A>& r = *i;

	if (net_match(left, r)) {
	    ret = true;
	    break;
	}
    }

    return return_bool(ret);
}

// register callbacks
RegisterOperations _regops;

} // namespace

// XXX: hack to compile on 2.95.x [may not use &operation::op... with templates]
using namespace operations;

// FIXME: no comment =D
// macro ugliness to specify possible operations
RegisterOperations::RegisterOperations()
{
    Dispatcher disp;

    // prevent these from being deleted
    _true.ref();
    _false.ref();

#define ADD_BINOP(result,left,right,funct,oper)				\
do {									\
	disp.add<left,right,&funct<result,left,right> >(Op##oper());	\
} while (0)

// EQUAL AND NOT EQUAL
#define ADD_EQOP(arg)							\
do {									\
    ADD_BINOP(ElemBool,arg,arg,op_eq,Eq);				\
    ADD_BINOP(ElemBool,arg,arg,op_ne,Ne);				\
} while (0)

#define ADD_EQOP2(argl,argr)						\
do {									\
    ADD_BINOP(ElemBool,argl,argr,op_eq,Eq);				\
    ADD_BINOP(ElemBool,argl,argr,op_ne,Ne);				\
} while (0)

// RELATIONAL OPERATORS
#define ADD_RELOP(arg)							\
do {									\
    ADD_BINOP(ElemBool,arg,arg,op_lt,Lt);				\
    ADD_BINOP(ElemBool,arg,arg,op_gt,Gt);				\
    ADD_BINOP(ElemBool,arg,arg,op_le,Le);				\
    ADD_BINOP(ElemBool,arg,arg,op_ge,Ge);				\
} while (0)

#define ADD_RELOP_SPECIALIZED(arg, suffix)				\
do {									\
    ADD_BINOP(ElemBool,arg,arg,op_lt_##suffix,Lt);			\
    ADD_BINOP(ElemBool,arg,arg,op_gt_##suffix,Gt);			\
    ADD_BINOP(ElemBool,arg,arg,op_le_##suffix,Le);			\
    ADD_BINOP(ElemBool,arg,arg,op_ge_##suffix,Ge);			\
} while (0)

#define ADD_RELOP2(argl,argr)						\
do {									\
    ADD_BINOP(ElemBool,argl,argr,op_lt,Lt);				\
    ADD_BINOP(ElemBool,argl,argr,op_gt,Gt);				\
    ADD_BINOP(ElemBool,argl,argr,op_le,Le);				\
    ADD_BINOP(ElemBool,argl,argr,op_ge,Ge);				\
} while (0)

#define ADD_ALL_RELOP2(argl, argr)					\
do {									\
    ADD_RELOP2(argl, argr);						\
    ADD_EQOP2(argl, argr);						\
} while (0)

// MATH OPERATORS
#define ADD_MATHOP(arg)							\
do {									\
    ADD_BINOP(arg,arg,arg,op_add,Add);					\
    ADD_BINOP(arg,arg,arg,op_sub,Sub);					\
    ADD_BINOP(arg,arg,arg,op_mul,Mul);					\
    ADD_BINOP(arg,arg,arg,op_div,Div);					\
    ADD_BINOP(arg,arg,arg,op_lshift,LShift);				\
    ADD_BINOP(arg,arg,arg,op_rshift,RShift);				\
    ADD_BINOP(arg,arg,arg,op_bit_and,BitAnd);				\
    ADD_BINOP(arg,arg,arg,op_bit_or,BitOr);				\
    ADD_BINOP(arg,arg,arg,op_bit_xor,BitXor);					\
} while (0)

    disp.add<ElemBool,&operations::op_not>(OpNot());
    disp.add<ElemStr, &operations::op_head>(OpHead());

    // boolean logic
    ADD_BINOP(ElemBool,ElemBool,ElemBool,op_and,And);
    ADD_BINOP(ElemBool,ElemBool,ElemBool,op_or,Or);
    ADD_BINOP(ElemBool,ElemBool,ElemBool,op_xor,Xor);
    ADD_EQOP(ElemBool);

    // SET ADDITION [used for policy tags] -- insert an element in the set.
    disp.add<ElemSetU32, ElemU32, operations::set_add>(OpAdd());
    disp.add<ElemSetCom32, ElemCom32, operations::set_add>(OpAdd());
   
    // SET operations [used for communities in BGP for example].
    disp.add<ElemSetU32, ElemSetU32, operations::set_ne_int>(OpNEInt());
    disp.add<ElemSetU32, ElemSetU32, operations::set_add>(OpAdd());
    disp.add<ElemSetU32, ElemSetU32, operations::set_del>(OpSub());
    disp.add<ElemSetCom32, ElemSetCom32, operations::set_ne_int>(OpNEInt());
    disp.add<ElemSetCom32, ElemSetCom32, operations::set_add>(OpAdd());
    disp.add<ElemSetCom32, ElemSetCom32, operations::set_del>(OpSub());

    //
    // The "ctr" operator.
    // It takes 2 arguments. A string representing the type to be
    // constructed, and any type of element to construct it from.
    // Think of it as a cast---powerful but dangerous.
    //
    disp.add<ElemStr, ElemInt32, operations::ctr>(OpCtr());
    disp.add<ElemStr, ElemU32, operations::ctr>(OpCtr());
    disp.add<ElemStr, ElemCom32, operations::ctr>(OpCtr());
    disp.add<ElemStr, ElemStr, operations::ctr>(OpCtr());
    disp.add<ElemStr, ElemBool, operations::ctr>(OpCtr());
    disp.add<ElemStr, ElemIPv4, operations::ctr>(OpCtr());
    disp.add<ElemStr, ElemIPv6, operations::ctr>(OpCtr());
    disp.add<ElemStr, ElemIPv4Range, operations::ctr>(OpCtr());
    disp.add<ElemStr, ElemIPv6Range, operations::ctr>(OpCtr());
    disp.add<ElemStr, ElemIPv4Net, operations::ctr>(OpCtr());
    disp.add<ElemStr, ElemIPv6Net, operations::ctr>(OpCtr());
    disp.add<ElemStr, ElemU32Range, operations::ctr>(OpCtr());

    // ASPATH operations
    disp.add<ElemU32, ElemASPath, operations::aspath_prepend>(OpAdd());
    disp.add<ElemU32, ElemASPath, operations::aspath_expand>(OpMul());
    disp.add<ElemASPath, ElemU32, operations::aspath_contains>(OpNEInt());
    disp.add<ElemASPath, ElemStr, operations::aspath_regex>(OpRegex());
    disp.add<ElemASPath, ElemSetStr, operations::aspath_regex>(OpRegex());


#define ADD_LSETBINOP(set, arg)						\
do {									\
    ADD_BINOP(ElemBool,set,arg,op_eq_nv,Eq);			        \
    ADD_BINOP(ElemBool,set,arg,op_ne_nv,Ne);   			        \
    ADD_BINOP(ElemBool,set,arg,op_lt_nv,Lt);				\
    ADD_BINOP(ElemBool,set,arg,op_gt_nv,Gt);				\
    ADD_BINOP(ElemBool,set,arg,op_le_nv,Le);				\
    ADD_BINOP(ElemBool,set,arg,op_ge_nv,Ge);				\
} while(0)

    ADD_LSETBINOP(ElemSetU32, ElemSetU32);
    ADD_LSETBINOP(ElemSetCom32, ElemSetCom32);

#define ADD_SETBINOP(set, arg)                                          \
do {                                                                    \
    ADD_LSETBINOP(set, arg);                                            \
    ADD_BINOP(ElemBool,arg,set,op_eq_sw,Eq);				\
    ADD_BINOP(ElemBool,arg,set,op_ne_sw,Ne);				\
    ADD_BINOP(ElemBool,arg,set,op_lt_sw,Lt);				\
    ADD_BINOP(ElemBool,arg,set,op_gt_sw,Gt);				\
    ADD_BINOP(ElemBool,arg,set,op_le_sw,Le);				\
    ADD_BINOP(ElemBool,arg,set,op_ge_sw,Ge);				\
} while (0)

    // i32
    ADD_EQOP(ElemInt32);
    ADD_RELOP(ElemInt32);
    ADD_MATHOP(ElemInt32);
//    ADD_SETBINOP(ElemInt32);

    // u32
    ADD_EQOP(ElemU32);
    ADD_RELOP(ElemU32);
    ADD_MATHOP(ElemU32);
    ADD_SETBINOP(ElemSetU32, ElemU32);

    // u32range
    ADD_EQOP2(ElemU32,ElemU32Range);
    ADD_RELOP2(ElemU32,ElemU32Range);

    // com32
    ADD_EQOP(ElemCom32);
//  ADD_RELOP(ElemCom32);
//  ADD_MATHOP(ElemCom32);
    ADD_SETBINOP(ElemSetCom32, ElemCom32);

    // strings
    ADD_EQOP(ElemStr);
//    ADD_SETBINOP(ElemStr);
    disp.add<ElemStr, ElemStr, operations::str_add>(OpAdd());
    disp.add<ElemStr, ElemU32, operations::str_mul>(OpMul());
    disp.add<ElemStr, ElemStr, operations::str_regex>(OpRegex());
    disp.add<ElemStr, ElemSetStr, operations::str_setregex>(OpRegex());

    // IPV4
    ADD_EQOP(ElemIPv4);
    ADD_EQOP2(ElemIPv4, ElemIPv4Range);
    ADD_RELOP(ElemIPv4);
    ADD_RELOP2(ElemIPv4, ElemIPv4Range);
//  ADD_SETBINOP(ElemIPv4);
    ADD_ALL_RELOP2(ElemIPv4NextHop, ElemIPv4Range);

    // IPV4NET
    ADD_EQOP(ElemIPv4Net);
    ADD_EQOP2(ElemIPv4Net, ElemU32Range);
    ADD_RELOP_SPECIALIZED(ElemIPv4Net, net);
    ADD_RELOP2(ElemIPv4Net, ElemU32Range);
    ADD_SETBINOP(ElemSetIPv4Net, ElemIPv4Net);
    // Special case set operation to make use of modifiers
    disp.add<ElemIPv4Net, ElemSetIPv4Net, operations::net_set_match>(OpLe());

    // IPV6
    ADD_EQOP(ElemIPv6);
    ADD_EQOP2(ElemIPv6,ElemIPv6Range);
    ADD_RELOP(ElemIPv6);
    ADD_RELOP2(ElemIPv6,ElemIPv6Range);
//    ADD_SETBINOP(ElemIPv6);
    ADD_ALL_RELOP2(ElemIPv6NextHop, ElemIPv6Range);

    // IPV6NET
    ADD_EQOP(ElemIPv6Net);
    ADD_EQOP2(ElemIPv6Net,ElemU32Range);
    ADD_RELOP_SPECIALIZED(ElemIPv6Net, net);
    ADD_RELOP2(ElemIPv6Net,ElemU32Range);
    ADD_SETBINOP(ElemSetIPv6Net, ElemIPv6Net);
    // Special case set operation to make use of modifiers
    disp.add<ElemIPv6Net, ElemSetIPv6Net, operations::net_set_match>(OpLe());
}
