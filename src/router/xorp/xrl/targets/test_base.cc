/*
 * obj/x86_64-unknown-linux-gnu/xrl/targets/test_base.cc
 * vim:set sts=4 ts=8 ft=cpp:
 *
 * Copyright (c) 2001-2011 XORP, Inc and Others
 * See the XORP LICENSE.lgpl file for licensing, conditions, and warranties
 * on use.
 *
 * DO NOT EDIT THIS FILE - IT IS PROGRAMMATICALLY GENERATED
 *
 * Generated by 'tgt-gen'.
 */


#include "test_base.hh"


const struct XrlTestTargetBase::handler_table XrlTestTargetBase::handlers[] = {
    { "common/0.1/get_target_name",
      &XrlTestTargetBase::handle_common_0_1_get_target_name },
    { "common/0.1/get_version",
      &XrlTestTargetBase::handle_common_0_1_get_version },
    { "common/0.1/get_status",
      &XrlTestTargetBase::handle_common_0_1_get_status },
    { "common/0.1/shutdown",
      &XrlTestTargetBase::handle_common_0_1_shutdown },
    { "common/0.1/startup",
      &XrlTestTargetBase::handle_common_0_1_startup },
    { "test/1.0/print_hello_world",
      &XrlTestTargetBase::handle_test_1_0_print_hello_world },
    { "test/1.0/print_hello_world_and_message",
      &XrlTestTargetBase::handle_test_1_0_print_hello_world_and_message },
    { "test/1.0/get_greeting_count",
      &XrlTestTargetBase::handle_test_1_0_get_greeting_count },
    { "test/1.0/get_greeting",
      &XrlTestTargetBase::handle_test_1_0_get_greeting },
    { "test/1.0/shoot_foot",
      &XrlTestTargetBase::handle_test_1_0_shoot_foot },
    { "test/1.0/float_my_point",
      &XrlTestTargetBase::handle_test_1_0_float_my_point },
};

const size_t XrlTestTargetBase::num_handlers = (sizeof(XrlTestTargetBase::handlers) / sizeof(XrlTestTargetBase::handlers[0]));


XrlTestTargetBase::XrlTestTargetBase(XrlCmdMap* cmds)
    : _cmds(cmds)
{
    if (_cmds)
	add_handlers();
}

XrlTestTargetBase::~XrlTestTargetBase()
{
    if (_cmds)
	remove_handlers();
}

bool
XrlTestTargetBase::set_command_map(XrlCmdMap* cmds)
{
    if (_cmds == 0 && cmds) {
        _cmds = cmds;
        add_handlers();
        return true;
    }
    if (_cmds && cmds == 0) {
	remove_handlers();
        _cmds = cmds;
        return true;
    }
    return false;
}


#ifdef XORP_ENABLE_ASYNC_SERVER
void
XrlTestTargetBase::callback_common_0_1_get_target_name
    (const XrlCmdError &e,
     const string* rarg_name,
     XrlRespCallback c_b)
{
    if (e != XrlCmdError::OKAY()) {
	XLOG_WARNING("Handling method for %s failed: %s",
		     "common/0.1/get_target_name", e.str().c_str());
	return c_b->dispatch(e, NULL);
    } else {
	XrlArgs out;

        /* Marshall return values */
        try {
	    out.add("name", *rarg_name);
        } catch (const XrlArgs::XrlAtomFound& ) {
	    XLOG_FATAL("Duplicate atom name"); /* XXX Should never happen */
        }

        return c_b->dispatch(e, &out);
    }
}


void
XrlTestTargetBase::async_common_0_1_get_target_name(
	Common01GetTargetNameCB c_b)
{

    /* Return value declarations */
    string rarg_name;
    XrlCmdError e = common_0_1_get_target_name(
        rarg_name);
    return c_b->dispatch(e,
        &rarg_name);
}
#endif

XrlCmdRT
XrlTestTargetBase::handle_common_0_1_get_target_name(const XrlArgs& xa_inputs, XrlCmdOT pxa_outputs)
{
    if (xa_inputs.size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != %u) handling %s",
            XORP_UINT_CAST(0), XORP_UINT_CAST(xa_inputs.size()), "common/0.1/get_target_name");
	XRL_CMD_RETURN_ERROR(pxa_outputs, XrlCmdError::BAD_ARGS());
    }

#ifndef XORP_ENABLE_ASYNC_SERVER
    if (pxa_outputs == 0) {
	XLOG_FATAL("Return list empty");
	return XrlCmdError::BAD_ARGS();
    }
#endif

#ifdef XORP_ENABLE_ASYNC_SERVER
    try {
	Common01GetTargetNameRF mycb =
	    callback(this, &XrlTestTargetBase::callback_common_0_1_get_target_name, pxa_outputs);
	async_common_0_1_get_target_name( mycb);
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return pxa_outputs->dispatch(XrlCmdError::BAD_ARGS(e.str()), NULL);
    }
#else

    /* Return value declarations */
    string r_name;
    try {
	XrlCmdError e = common_0_1_get_target_name(
	    r_name);
	if (e != XrlCmdError::OKAY()) {
	    XLOG_WARNING("Handling method for %s failed: %s",
            		 "common/0.1/get_target_name", e.str().c_str());
	    return e;
        }
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return XrlCmdError::BAD_ARGS(e.str());
    }

    /* Marshall return values */
    try {
	pxa_outputs->add("name", r_name);
    } catch (const XrlArgs::XrlAtomFound& ) {
	XLOG_FATAL("Duplicate atom name"); /* XXX Should never happen */
    }
    return XrlCmdError::OKAY();
#endif
}


#ifdef XORP_ENABLE_ASYNC_SERVER
void
XrlTestTargetBase::callback_common_0_1_get_version
    (const XrlCmdError &e,
     const string* rarg_version,
     XrlRespCallback c_b)
{
    if (e != XrlCmdError::OKAY()) {
	XLOG_WARNING("Handling method for %s failed: %s",
		     "common/0.1/get_version", e.str().c_str());
	return c_b->dispatch(e, NULL);
    } else {
	XrlArgs out;

        /* Marshall return values */
        try {
	    out.add("version", *rarg_version);
        } catch (const XrlArgs::XrlAtomFound& ) {
	    XLOG_FATAL("Duplicate atom name"); /* XXX Should never happen */
        }

        return c_b->dispatch(e, &out);
    }
}


void
XrlTestTargetBase::async_common_0_1_get_version(
	Common01GetVersionCB c_b)
{

    /* Return value declarations */
    string rarg_version;
    XrlCmdError e = common_0_1_get_version(
        rarg_version);
    return c_b->dispatch(e,
        &rarg_version);
}
#endif

XrlCmdRT
XrlTestTargetBase::handle_common_0_1_get_version(const XrlArgs& xa_inputs, XrlCmdOT pxa_outputs)
{
    if (xa_inputs.size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != %u) handling %s",
            XORP_UINT_CAST(0), XORP_UINT_CAST(xa_inputs.size()), "common/0.1/get_version");
	XRL_CMD_RETURN_ERROR(pxa_outputs, XrlCmdError::BAD_ARGS());
    }

#ifndef XORP_ENABLE_ASYNC_SERVER
    if (pxa_outputs == 0) {
	XLOG_FATAL("Return list empty");
	return XrlCmdError::BAD_ARGS();
    }
#endif

#ifdef XORP_ENABLE_ASYNC_SERVER
    try {
	Common01GetVersionRF mycb =
	    callback(this, &XrlTestTargetBase::callback_common_0_1_get_version, pxa_outputs);
	async_common_0_1_get_version( mycb);
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return pxa_outputs->dispatch(XrlCmdError::BAD_ARGS(e.str()), NULL);
    }
#else

    /* Return value declarations */
    string r_version;
    try {
	XrlCmdError e = common_0_1_get_version(
	    r_version);
	if (e != XrlCmdError::OKAY()) {
	    XLOG_WARNING("Handling method for %s failed: %s",
            		 "common/0.1/get_version", e.str().c_str());
	    return e;
        }
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return XrlCmdError::BAD_ARGS(e.str());
    }

    /* Marshall return values */
    try {
	pxa_outputs->add("version", r_version);
    } catch (const XrlArgs::XrlAtomFound& ) {
	XLOG_FATAL("Duplicate atom name"); /* XXX Should never happen */
    }
    return XrlCmdError::OKAY();
#endif
}


#ifdef XORP_ENABLE_ASYNC_SERVER
void
XrlTestTargetBase::callback_common_0_1_get_status
    (const XrlCmdError &e,
     const uint32_t* rarg_status,
     const string* rarg_reason,
     XrlRespCallback c_b)
{
    if (e != XrlCmdError::OKAY()) {
	XLOG_WARNING("Handling method for %s failed: %s",
		     "common/0.1/get_status", e.str().c_str());
	return c_b->dispatch(e, NULL);
    } else {
	XrlArgs out;

        /* Marshall return values */
        try {
	    out.add("status", *rarg_status);
	    out.add("reason", *rarg_reason);
        } catch (const XrlArgs::XrlAtomFound& ) {
	    XLOG_FATAL("Duplicate atom name"); /* XXX Should never happen */
        }

        return c_b->dispatch(e, &out);
    }
}


void
XrlTestTargetBase::async_common_0_1_get_status(
	Common01GetStatusCB c_b)
{

    /* Return value declarations */
    uint32_t rarg_status;
    string rarg_reason;
    XrlCmdError e = common_0_1_get_status(
        rarg_status,
        rarg_reason);
    return c_b->dispatch(e,
        &rarg_status,
        &rarg_reason);
}
#endif

XrlCmdRT
XrlTestTargetBase::handle_common_0_1_get_status(const XrlArgs& xa_inputs, XrlCmdOT pxa_outputs)
{
    if (xa_inputs.size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != %u) handling %s",
            XORP_UINT_CAST(0), XORP_UINT_CAST(xa_inputs.size()), "common/0.1/get_status");
	XRL_CMD_RETURN_ERROR(pxa_outputs, XrlCmdError::BAD_ARGS());
    }

#ifndef XORP_ENABLE_ASYNC_SERVER
    if (pxa_outputs == 0) {
	XLOG_FATAL("Return list empty");
	return XrlCmdError::BAD_ARGS();
    }
#endif

#ifdef XORP_ENABLE_ASYNC_SERVER
    try {
	Common01GetStatusRF mycb =
	    callback(this, &XrlTestTargetBase::callback_common_0_1_get_status, pxa_outputs);
	async_common_0_1_get_status( mycb);
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return pxa_outputs->dispatch(XrlCmdError::BAD_ARGS(e.str()), NULL);
    }
#else

    /* Return value declarations */
    uint32_t r_status;
    string r_reason;
    try {
	XrlCmdError e = common_0_1_get_status(
	    r_status,
	    r_reason);
	if (e != XrlCmdError::OKAY()) {
	    XLOG_WARNING("Handling method for %s failed: %s",
            		 "common/0.1/get_status", e.str().c_str());
	    return e;
        }
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return XrlCmdError::BAD_ARGS(e.str());
    }

    /* Marshall return values */
    try {
	pxa_outputs->add("status", r_status);
	pxa_outputs->add("reason", r_reason);
    } catch (const XrlArgs::XrlAtomFound& ) {
	XLOG_FATAL("Duplicate atom name"); /* XXX Should never happen */
    }
    return XrlCmdError::OKAY();
#endif
}


#ifdef XORP_ENABLE_ASYNC_SERVER
void
XrlTestTargetBase::callback_common_0_1_shutdown
    (const XrlCmdError &e,
     XrlRespCallback c_b)
{
    if (e != XrlCmdError::OKAY()) {
	XLOG_WARNING("Handling method for %s failed: %s",
		     "common/0.1/shutdown", e.str().c_str());
	return c_b->dispatch(e, NULL);
    } else {
	XrlArgs out;
        return c_b->dispatch(e, &out);
    }
}


void
XrlTestTargetBase::async_common_0_1_shutdown(
	Common01ShutdownCB c_b)
{

    /* Return value declarations */
    XrlCmdError e = common_0_1_shutdown();
    return c_b->dispatch(e);
}
#endif

XrlCmdRT
XrlTestTargetBase::handle_common_0_1_shutdown(const XrlArgs& xa_inputs, XrlCmdOT pxa_outputs)
{
    if (xa_inputs.size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != %u) handling %s",
            XORP_UINT_CAST(0), XORP_UINT_CAST(xa_inputs.size()), "common/0.1/shutdown");
	XRL_CMD_RETURN_ERROR(pxa_outputs, XrlCmdError::BAD_ARGS());
    }

#ifndef XORP_ENABLE_ASYNC_SERVER
    UNUSED(pxa_outputs);
#endif

#ifdef XORP_ENABLE_ASYNC_SERVER
    try {
	Common01ShutdownRF mycb =
	    callback(this, &XrlTestTargetBase::callback_common_0_1_shutdown, pxa_outputs);
	async_common_0_1_shutdown( mycb);
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return pxa_outputs->dispatch(XrlCmdError::BAD_ARGS(e.str()), NULL);
    }
#else

    /* Return value declarations */
    try {
	XrlCmdError e = common_0_1_shutdown();
	if (e != XrlCmdError::OKAY()) {
	    XLOG_WARNING("Handling method for %s failed: %s",
            		 "common/0.1/shutdown", e.str().c_str());
	    return e;
        }
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return XrlCmdError::BAD_ARGS(e.str());
    }
    return XrlCmdError::OKAY();
#endif
}


#ifdef XORP_ENABLE_ASYNC_SERVER
void
XrlTestTargetBase::callback_common_0_1_startup
    (const XrlCmdError &e,
     XrlRespCallback c_b)
{
    if (e != XrlCmdError::OKAY()) {
	XLOG_WARNING("Handling method for %s failed: %s",
		     "common/0.1/startup", e.str().c_str());
	return c_b->dispatch(e, NULL);
    } else {
	XrlArgs out;
        return c_b->dispatch(e, &out);
    }
}


void
XrlTestTargetBase::async_common_0_1_startup(
	Common01StartupCB c_b)
{

    /* Return value declarations */
    XrlCmdError e = common_0_1_startup();
    return c_b->dispatch(e);
}
#endif

XrlCmdRT
XrlTestTargetBase::handle_common_0_1_startup(const XrlArgs& xa_inputs, XrlCmdOT pxa_outputs)
{
    if (xa_inputs.size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != %u) handling %s",
            XORP_UINT_CAST(0), XORP_UINT_CAST(xa_inputs.size()), "common/0.1/startup");
	XRL_CMD_RETURN_ERROR(pxa_outputs, XrlCmdError::BAD_ARGS());
    }

#ifndef XORP_ENABLE_ASYNC_SERVER
    UNUSED(pxa_outputs);
#endif

#ifdef XORP_ENABLE_ASYNC_SERVER
    try {
	Common01StartupRF mycb =
	    callback(this, &XrlTestTargetBase::callback_common_0_1_startup, pxa_outputs);
	async_common_0_1_startup( mycb);
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return pxa_outputs->dispatch(XrlCmdError::BAD_ARGS(e.str()), NULL);
    }
#else

    /* Return value declarations */
    try {
	XrlCmdError e = common_0_1_startup();
	if (e != XrlCmdError::OKAY()) {
	    XLOG_WARNING("Handling method for %s failed: %s",
            		 "common/0.1/startup", e.str().c_str());
	    return e;
        }
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return XrlCmdError::BAD_ARGS(e.str());
    }
    return XrlCmdError::OKAY();
#endif
}


#ifdef XORP_ENABLE_ASYNC_SERVER
void
XrlTestTargetBase::callback_test_1_0_print_hello_world
    (const XrlCmdError &e,
     XrlRespCallback c_b)
{
    if (e != XrlCmdError::OKAY()) {
	XLOG_WARNING("Handling method for %s failed: %s",
		     "test/1.0/print_hello_world", e.str().c_str());
	return c_b->dispatch(e, NULL);
    } else {
	XrlArgs out;
        return c_b->dispatch(e, &out);
    }
}


void
XrlTestTargetBase::async_test_1_0_print_hello_world(
	Test10PrintHelloWorldCB c_b)
{

    /* Return value declarations */
    XrlCmdError e = test_1_0_print_hello_world();
    return c_b->dispatch(e);
}
#endif

XrlCmdRT
XrlTestTargetBase::handle_test_1_0_print_hello_world(const XrlArgs& xa_inputs, XrlCmdOT pxa_outputs)
{
    if (xa_inputs.size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != %u) handling %s",
            XORP_UINT_CAST(0), XORP_UINT_CAST(xa_inputs.size()), "test/1.0/print_hello_world");
	XRL_CMD_RETURN_ERROR(pxa_outputs, XrlCmdError::BAD_ARGS());
    }

#ifndef XORP_ENABLE_ASYNC_SERVER
    UNUSED(pxa_outputs);
#endif

#ifdef XORP_ENABLE_ASYNC_SERVER
    try {
	Test10PrintHelloWorldRF mycb =
	    callback(this, &XrlTestTargetBase::callback_test_1_0_print_hello_world, pxa_outputs);
	async_test_1_0_print_hello_world( mycb);
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return pxa_outputs->dispatch(XrlCmdError::BAD_ARGS(e.str()), NULL);
    }
#else

    /* Return value declarations */
    try {
	XrlCmdError e = test_1_0_print_hello_world();
	if (e != XrlCmdError::OKAY()) {
	    XLOG_WARNING("Handling method for %s failed: %s",
            		 "test/1.0/print_hello_world", e.str().c_str());
	    return e;
        }
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return XrlCmdError::BAD_ARGS(e.str());
    }
    return XrlCmdError::OKAY();
#endif
}


#ifdef XORP_ENABLE_ASYNC_SERVER
void
XrlTestTargetBase::callback_test_1_0_print_hello_world_and_message
    (const XrlCmdError &e,
     XrlRespCallback c_b)
{
    if (e != XrlCmdError::OKAY()) {
	XLOG_WARNING("Handling method for %s failed: %s",
		     "test/1.0/print_hello_world_and_message", e.str().c_str());
	return c_b->dispatch(e, NULL);
    } else {
	XrlArgs out;
        return c_b->dispatch(e, &out);
    }
}


void
XrlTestTargetBase::async_test_1_0_print_hello_world_and_message(
	const string&	arg_msg,
	Test10PrintHelloWorldAndMessageCB c_b)
{

    /* Return value declarations */
    XrlCmdError e = test_1_0_print_hello_world_and_message(
        arg_msg);
    return c_b->dispatch(e);
}
#endif

XrlCmdRT
XrlTestTargetBase::handle_test_1_0_print_hello_world_and_message(const XrlArgs& xa_inputs, XrlCmdOT pxa_outputs)
{
    if (xa_inputs.size() != 1) {
	XLOG_ERROR("Wrong number of arguments (%u != %u) handling %s",
            XORP_UINT_CAST(1), XORP_UINT_CAST(xa_inputs.size()), "test/1.0/print_hello_world_and_message");
	XRL_CMD_RETURN_ERROR(pxa_outputs, XrlCmdError::BAD_ARGS());
    }

#ifndef XORP_ENABLE_ASYNC_SERVER
    UNUSED(pxa_outputs);
#endif

#ifdef XORP_ENABLE_ASYNC_SERVER
    try {
	Test10PrintHelloWorldAndMessageRF mycb =
	    callback(this, &XrlTestTargetBase::callback_test_1_0_print_hello_world_and_message, pxa_outputs);
	async_test_1_0_print_hello_world_and_message(
	    xa_inputs.get(0, "msg").text(), mycb);
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return pxa_outputs->dispatch(XrlCmdError::BAD_ARGS(e.str()), NULL);
    }
#else

    /* Return value declarations */
    try {
	XrlCmdError e = test_1_0_print_hello_world_and_message(
	    xa_inputs.get(0, "msg").text());
	if (e != XrlCmdError::OKAY()) {
	    XLOG_WARNING("Handling method for %s failed: %s",
            		 "test/1.0/print_hello_world_and_message", e.str().c_str());
	    return e;
        }
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return XrlCmdError::BAD_ARGS(e.str());
    }
    return XrlCmdError::OKAY();
#endif
}


#ifdef XORP_ENABLE_ASYNC_SERVER
void
XrlTestTargetBase::callback_test_1_0_get_greeting_count
    (const XrlCmdError &e,
     const int32_t* rarg_num_msgs,
     XrlRespCallback c_b)
{
    if (e != XrlCmdError::OKAY()) {
	XLOG_WARNING("Handling method for %s failed: %s",
		     "test/1.0/get_greeting_count", e.str().c_str());
	return c_b->dispatch(e, NULL);
    } else {
	XrlArgs out;

        /* Marshall return values */
        try {
	    out.add("num_msgs", *rarg_num_msgs);
        } catch (const XrlArgs::XrlAtomFound& ) {
	    XLOG_FATAL("Duplicate atom name"); /* XXX Should never happen */
        }

        return c_b->dispatch(e, &out);
    }
}


void
XrlTestTargetBase::async_test_1_0_get_greeting_count(
	Test10GetGreetingCountCB c_b)
{

    /* Return value declarations */
    int32_t rarg_num_msgs;
    XrlCmdError e = test_1_0_get_greeting_count(
        rarg_num_msgs);
    return c_b->dispatch(e,
        &rarg_num_msgs);
}
#endif

XrlCmdRT
XrlTestTargetBase::handle_test_1_0_get_greeting_count(const XrlArgs& xa_inputs, XrlCmdOT pxa_outputs)
{
    if (xa_inputs.size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != %u) handling %s",
            XORP_UINT_CAST(0), XORP_UINT_CAST(xa_inputs.size()), "test/1.0/get_greeting_count");
	XRL_CMD_RETURN_ERROR(pxa_outputs, XrlCmdError::BAD_ARGS());
    }

#ifndef XORP_ENABLE_ASYNC_SERVER
    if (pxa_outputs == 0) {
	XLOG_FATAL("Return list empty");
	return XrlCmdError::BAD_ARGS();
    }
#endif

#ifdef XORP_ENABLE_ASYNC_SERVER
    try {
	Test10GetGreetingCountRF mycb =
	    callback(this, &XrlTestTargetBase::callback_test_1_0_get_greeting_count, pxa_outputs);
	async_test_1_0_get_greeting_count( mycb);
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return pxa_outputs->dispatch(XrlCmdError::BAD_ARGS(e.str()), NULL);
    }
#else

    /* Return value declarations */
    int32_t r_num_msgs;
    try {
	XrlCmdError e = test_1_0_get_greeting_count(
	    r_num_msgs);
	if (e != XrlCmdError::OKAY()) {
	    XLOG_WARNING("Handling method for %s failed: %s",
            		 "test/1.0/get_greeting_count", e.str().c_str());
	    return e;
        }
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return XrlCmdError::BAD_ARGS(e.str());
    }

    /* Marshall return values */
    try {
	pxa_outputs->add("num_msgs", r_num_msgs);
    } catch (const XrlArgs::XrlAtomFound& ) {
	XLOG_FATAL("Duplicate atom name"); /* XXX Should never happen */
    }
    return XrlCmdError::OKAY();
#endif
}


#ifdef XORP_ENABLE_ASYNC_SERVER
void
XrlTestTargetBase::callback_test_1_0_get_greeting
    (const XrlCmdError &e,
     const string* rarg_greeting,
     XrlRespCallback c_b)
{
    if (e != XrlCmdError::OKAY()) {
	XLOG_WARNING("Handling method for %s failed: %s",
		     "test/1.0/get_greeting", e.str().c_str());
	return c_b->dispatch(e, NULL);
    } else {
	XrlArgs out;

        /* Marshall return values */
        try {
	    out.add("greeting", *rarg_greeting);
        } catch (const XrlArgs::XrlAtomFound& ) {
	    XLOG_FATAL("Duplicate atom name"); /* XXX Should never happen */
        }

        return c_b->dispatch(e, &out);
    }
}


void
XrlTestTargetBase::async_test_1_0_get_greeting(
	const int32_t&	arg_greeting_num,
	Test10GetGreetingCB c_b)
{

    /* Return value declarations */
    string rarg_greeting;
    XrlCmdError e = test_1_0_get_greeting(
        arg_greeting_num,
        rarg_greeting);
    return c_b->dispatch(e,
        &rarg_greeting);
}
#endif

XrlCmdRT
XrlTestTargetBase::handle_test_1_0_get_greeting(const XrlArgs& xa_inputs, XrlCmdOT pxa_outputs)
{
    if (xa_inputs.size() != 1) {
	XLOG_ERROR("Wrong number of arguments (%u != %u) handling %s",
            XORP_UINT_CAST(1), XORP_UINT_CAST(xa_inputs.size()), "test/1.0/get_greeting");
	XRL_CMD_RETURN_ERROR(pxa_outputs, XrlCmdError::BAD_ARGS());
    }

#ifndef XORP_ENABLE_ASYNC_SERVER
    if (pxa_outputs == 0) {
	XLOG_FATAL("Return list empty");
	return XrlCmdError::BAD_ARGS();
    }
#endif

#ifdef XORP_ENABLE_ASYNC_SERVER
    try {
	Test10GetGreetingRF mycb =
	    callback(this, &XrlTestTargetBase::callback_test_1_0_get_greeting, pxa_outputs);
	async_test_1_0_get_greeting(
	    xa_inputs.get(0, "greeting_num").int32(), mycb);
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return pxa_outputs->dispatch(XrlCmdError::BAD_ARGS(e.str()), NULL);
    }
#else

    /* Return value declarations */
    string r_greeting;
    try {
	XrlCmdError e = test_1_0_get_greeting(
	    xa_inputs.get(0, "greeting_num").int32(),
	    r_greeting);
	if (e != XrlCmdError::OKAY()) {
	    XLOG_WARNING("Handling method for %s failed: %s",
            		 "test/1.0/get_greeting", e.str().c_str());
	    return e;
        }
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return XrlCmdError::BAD_ARGS(e.str());
    }

    /* Marshall return values */
    try {
	pxa_outputs->add("greeting", r_greeting);
    } catch (const XrlArgs::XrlAtomFound& ) {
	XLOG_FATAL("Duplicate atom name"); /* XXX Should never happen */
    }
    return XrlCmdError::OKAY();
#endif
}


#ifdef XORP_ENABLE_ASYNC_SERVER
void
XrlTestTargetBase::callback_test_1_0_shoot_foot
    (const XrlCmdError &e,
     XrlRespCallback c_b)
{
    if (e != XrlCmdError::OKAY()) {
	XLOG_WARNING("Handling method for %s failed: %s",
		     "test/1.0/shoot_foot", e.str().c_str());
	return c_b->dispatch(e, NULL);
    } else {
	XrlArgs out;
        return c_b->dispatch(e, &out);
    }
}


void
XrlTestTargetBase::async_test_1_0_shoot_foot(
	Test10ShootFootCB c_b)
{

    /* Return value declarations */
    XrlCmdError e = test_1_0_shoot_foot();
    return c_b->dispatch(e);
}
#endif

XrlCmdRT
XrlTestTargetBase::handle_test_1_0_shoot_foot(const XrlArgs& xa_inputs, XrlCmdOT pxa_outputs)
{
    if (xa_inputs.size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != %u) handling %s",
            XORP_UINT_CAST(0), XORP_UINT_CAST(xa_inputs.size()), "test/1.0/shoot_foot");
	XRL_CMD_RETURN_ERROR(pxa_outputs, XrlCmdError::BAD_ARGS());
    }

#ifndef XORP_ENABLE_ASYNC_SERVER
    UNUSED(pxa_outputs);
#endif

#ifdef XORP_ENABLE_ASYNC_SERVER
    try {
	Test10ShootFootRF mycb =
	    callback(this, &XrlTestTargetBase::callback_test_1_0_shoot_foot, pxa_outputs);
	async_test_1_0_shoot_foot( mycb);
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return pxa_outputs->dispatch(XrlCmdError::BAD_ARGS(e.str()), NULL);
    }
#else

    /* Return value declarations */
    try {
	XrlCmdError e = test_1_0_shoot_foot();
	if (e != XrlCmdError::OKAY()) {
	    XLOG_WARNING("Handling method for %s failed: %s",
            		 "test/1.0/shoot_foot", e.str().c_str());
	    return e;
        }
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return XrlCmdError::BAD_ARGS(e.str());
    }
    return XrlCmdError::OKAY();
#endif
}


#ifdef XORP_ENABLE_ASYNC_SERVER
void
XrlTestTargetBase::callback_test_1_0_float_my_point
    (const XrlCmdError &e,
     const fp64_t* rarg_output,
     XrlRespCallback c_b)
{
    if (e != XrlCmdError::OKAY()) {
	XLOG_WARNING("Handling method for %s failed: %s",
		     "test/1.0/float_my_point", e.str().c_str());
	return c_b->dispatch(e, NULL);
    } else {
	XrlArgs out;

        /* Marshall return values */
        try {
	    out.add("output", *rarg_output);
        } catch (const XrlArgs::XrlAtomFound& ) {
	    XLOG_FATAL("Duplicate atom name"); /* XXX Should never happen */
        }

        return c_b->dispatch(e, &out);
    }
}


void
XrlTestTargetBase::async_test_1_0_float_my_point(
	const fp64_t&	arg_input,
	Test10FloatMyPointCB c_b)
{

    /* Return value declarations */
    fp64_t rarg_output;
    XrlCmdError e = test_1_0_float_my_point(
        arg_input,
        rarg_output);
    return c_b->dispatch(e,
        &rarg_output);
}
#endif

XrlCmdRT
XrlTestTargetBase::handle_test_1_0_float_my_point(const XrlArgs& xa_inputs, XrlCmdOT pxa_outputs)
{
    if (xa_inputs.size() != 1) {
	XLOG_ERROR("Wrong number of arguments (%u != %u) handling %s",
            XORP_UINT_CAST(1), XORP_UINT_CAST(xa_inputs.size()), "test/1.0/float_my_point");
	XRL_CMD_RETURN_ERROR(pxa_outputs, XrlCmdError::BAD_ARGS());
    }

#ifndef XORP_ENABLE_ASYNC_SERVER
    if (pxa_outputs == 0) {
	XLOG_FATAL("Return list empty");
	return XrlCmdError::BAD_ARGS();
    }
#endif

#ifdef XORP_ENABLE_ASYNC_SERVER
    try {
	Test10FloatMyPointRF mycb =
	    callback(this, &XrlTestTargetBase::callback_test_1_0_float_my_point, pxa_outputs);
	async_test_1_0_float_my_point(
	    xa_inputs.get(0, "input").fp64(), mycb);
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return pxa_outputs->dispatch(XrlCmdError::BAD_ARGS(e.str()), NULL);
    }
#else

    /* Return value declarations */
    fp64_t r_output;
    try {
	XrlCmdError e = test_1_0_float_my_point(
	    xa_inputs.get(0, "input").fp64(),
	    r_output);
	if (e != XrlCmdError::OKAY()) {
	    XLOG_WARNING("Handling method for %s failed: %s",
            		 "test/1.0/float_my_point", e.str().c_str());
	    return e;
        }
    } catch (const XrlArgs::BadArgs& e) {
	XLOG_ERROR("Error decoding the arguments: %s", e.str().c_str());
	return XrlCmdError::BAD_ARGS(e.str());
    }

    /* Marshall return values */
    try {
	pxa_outputs->add("output", r_output);
    } catch (const XrlArgs::XrlAtomFound& ) {
	XLOG_FATAL("Duplicate atom name"); /* XXX Should never happen */
    }
    return XrlCmdError::OKAY();
#endif
}

void
XrlTestTargetBase::add_handlers()
{
    for (size_t i = 0; i < num_handlers; ++i) {
        if (!_cmds->add_handler(handlers[i].name,
                                callback(this, handlers[i].method))) {
            XLOG_ERROR("Failed to register xrl handler finder://%s/%s", "test", handlers[i].name);
        }
    }
    _cmds->finalize();
}

void
XrlTestTargetBase::remove_handlers()
{
     for (size_t i = 0; i < num_handlers; ++i) {
         _cmds->remove_handler(handlers[i].name);
     }
}