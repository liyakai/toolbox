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

// $XORP: xorp/policy/policy_list.hh,v 1.10 2008/10/02 21:57:59 bms Exp $

#ifndef __POLICY_POLICY_LIST_HH__
#define __POLICY_POLICY_LIST_HH__



#include "code_list.hh"
#include "set_map.hh"
#include "var_map.hh"
#include "policy_map.hh"
#include "visitor_semantic.hh"

/**
 * @short The list of policies associated with a protocol.
 *
 * This relates to the import/export directives of protocols.
 *
 * Depending on what protocols support, they will normally have a single
 * import/export policy list associated with them.
 *
 * Each policy list is an instantiation of a policy, and thus it hold the
 * specific code for this instantiation.
 */
class PolicyList :
    public NONCOPYABLE
{
public:
    typedef set<uint32_t> TagSet;
    typedef map<string,TagSet*> TagMap;

    enum PolicyType {
	IMPORT,
	EXPORT
    };

    /**
     * @param p protocol for which this list applies.
     * @param pt the type of policy list [import/export].
     * @param pmap the container of all policies.
     * @param smap the container of all sets.
     * @param vmap the VarMap.
     */
    PolicyList(const string& p, PolicyType pt, 
	       PolicyMap& pmap,
	       SetMap& smap, VarMap& vmap, string mod);

    ~PolicyList();

    /**
     * Append a policy to the list.
     *
     * @param policyname the name of the policy
     */
    void push_back(const string& policyname);

    /**
     * Compiles a specific policy.
     *
     * Throws an exception on semantic / compile errors.
     *
     * @param ps policy to compile.
     * @param mod set filled with targets which are modified by compilation.
     * @param tagstart first policy tag available.
     */
    void compile_policy(PolicyStatement& ps,
			Code::TargetSet& mod, 
			uint32_t& tagstart,
			map<string, set<uint32_t> >& ptags);

    /**
     * Compile all policies which were not previously compiled.
     *
     * Throws an exception on semantic / compile errors.
     *
     * @param mod set filled with targets which are modified by compilation.
     * @param tagstart first policy tag available.
     */
    void compile(Code::TargetSet& mod, uint32_t& tagstart, map<string, set<uint32_t> >& ptags);
   
    /**
     * @return string representation of list
     */
    string str();

    /**
     * Link the all the code avialable in this policy list with code supplied.
     *
     * The code supplied will normally contain a target, so only relevant code
     * is linked.
     *
     * @param ret code to link with.
     */
    void link_code(Code& ret);

    /**
     * Return all targets in this policy list.
     *
     * @param targets set filled with all targets in this policy instantiation.
     */
    void get_targets(Code::TargetSet& targets);

    /**
     * Return the policy tags used by a specific protocol for route
     * redistribution.
     *
     * @param protocol protocol for which tags are requested.
     * @param ts set filled with policy-tags used by the protocol.
     */
    void get_redist_tags(const string& protocol, Code::TagSet& ts);

private:
    typedef pair<string,CodeList*>  PolicyCode;
    typedef list<PolicyCode>	    PolicyCodeList;
    typedef set<string>		    POLICIES;

    /**
     * Semantically check the policy for this instantiation.
     *
     * Throws an exception on error.
     *
     * @param ps policy to check.
     * @param type type of policy [import/export].
     */
    void semantic_check(PolicyStatement& ps, VisitorSemantic::PolicyType type);

    /**
     * Compile an import policy.
     *
     * @param iter position in code list which needs to be replaced with code.
     * @param ps policy to compile.
     * @param modified_targets set filled with targets modified by compilation.
     */
    void compile_import(PolicyCodeList::iterator& iter, PolicyStatement& ps,
			Code::TargetSet& modified_targets);

    /**
     * Compile an export policy.
     *
     * @param iter position in code list which needs to be replaced with code.
     * @param ps policy to compile.
     * @param modified_targets set filled with targets modified by compilation.
     * @param tagstart first policy tag available.
     */
    void compile_export(PolicyCodeList::iterator& iter, PolicyStatement& ps,
			Code::TargetSet& modified_targets, uint32_t& tagstart,
			map<string, set<uint32_t> > & ptags);

    Term* create_mod(Term::BLOCKS block);
    void  add_policy_expression(const string& exp);

    string	    _protocol;
    PolicyType	    _type;
    PolicyCodeList  _policies;
    PolicyMap&	    _pmap;
    SetMap&	    _smap;
    VarMap&	    _varmap;
    string	    _mod;
    Term*	    _mod_term;
    Term*	    _mod_term_import;
    Term*	    _mod_term_export;
    static uint32_t _pe;
    POLICIES	    _pe_policies;
};

#endif // __POLICY_POLICY_LIST_HH__
