/*
 * obj/x86_64-unknown-linux-gnu/xrl/interfaces/test_xrls_xif.cc
 * vim:set sts=4 ts=8 ft=cpp:
 *
 * Copyright (c) 2001-2011 XORP, Inc and Others
 * See the XORP LICENSE.lgpl file for licensing, conditions, and warranties
 * on use.
 *
 * DO NOT EDIT THIS FILE - IT IS PROGRAMMATICALLY GENERATED
 *
 * Generated by 'clnt-gen'.
 */

#include "test_xrls_xif.hh"

bool
XrlTestXrlsV0p1Client::send_start_transmission(
	const char*	dst_xrl_target_name,
	const StartTransmissionCB&	cb
)
{
    Xrl* x = ap_xrl_start_transmission.get();

    if (!x) {
        x = new Xrl(dst_xrl_target_name, "test_xrls/0.1/start_transmission");
        ap_xrl_start_transmission.reset(x);
    }

    x->set_target(dst_xrl_target_name);


    return _sender->send(*x, callback(&XrlTestXrlsV0p1Client::unmarshall_start_transmission, cb));
}


/* Unmarshall start_transmission */
void
XrlTestXrlsV0p1Client::unmarshall_start_transmission(
	const XrlError&	e,
	XrlArgs*	a,
	StartTransmissionCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e);
	return;
    } else if (a && a->size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(0));
	cb->dispatch(XrlError::BAD_ARGS());
	return;
    }
    cb->dispatch(e);
}

bool
XrlTestXrlsV0p1Client::send_end_transmission(
	const char*	dst_xrl_target_name,
	const EndTransmissionCB&	cb
)
{
    Xrl* x = ap_xrl_end_transmission.get();

    if (!x) {
        x = new Xrl(dst_xrl_target_name, "test_xrls/0.1/end_transmission");
        ap_xrl_end_transmission.reset(x);
    }

    x->set_target(dst_xrl_target_name);


    return _sender->send(*x, callback(&XrlTestXrlsV0p1Client::unmarshall_end_transmission, cb));
}


/* Unmarshall end_transmission */
void
XrlTestXrlsV0p1Client::unmarshall_end_transmission(
	const XrlError&	e,
	XrlArgs*	a,
	EndTransmissionCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e);
	return;
    } else if (a && a->size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(0));
	cb->dispatch(XrlError::BAD_ARGS());
	return;
    }
    cb->dispatch(e);
}

bool
XrlTestXrlsV0p1Client::send_add_xrl0(
	const char*	dst_xrl_target_name,
	const AddXrl0CB&	cb
)
{
    Xrl* x = ap_xrl_add_xrl0.get();

    if (!x) {
        x = new Xrl(dst_xrl_target_name, "test_xrls/0.1/add_xrl0");
        ap_xrl_add_xrl0.reset(x);
    }

    x->set_target(dst_xrl_target_name);


    return _sender->send(*x, callback(&XrlTestXrlsV0p1Client::unmarshall_add_xrl0, cb));
}


/* Unmarshall add_xrl0 */
void
XrlTestXrlsV0p1Client::unmarshall_add_xrl0(
	const XrlError&	e,
	XrlArgs*	a,
	AddXrl0CB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e);
	return;
    } else if (a && a->size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(0));
	cb->dispatch(XrlError::BAD_ARGS());
	return;
    }
    cb->dispatch(e);
}

bool
XrlTestXrlsV0p1Client::send_add_xrl1(
	const char*	dst_xrl_target_name,
	const string&	data1,
	const AddXrl1CB&	cb
)
{
    Xrl* x = ap_xrl_add_xrl1.get();

    if (!x) {
        x = new Xrl(dst_xrl_target_name, "test_xrls/0.1/add_xrl1");
        x->args().add("data1", data1);
        ap_xrl_add_xrl1.reset(x);
    }

    x->set_target(dst_xrl_target_name);

    x->args().set_arg(0, data1);

    return _sender->send(*x, callback(&XrlTestXrlsV0p1Client::unmarshall_add_xrl1, cb));
}


/* Unmarshall add_xrl1 */
void
XrlTestXrlsV0p1Client::unmarshall_add_xrl1(
	const XrlError&	e,
	XrlArgs*	a,
	AddXrl1CB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e);
	return;
    } else if (a && a->size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(0));
	cb->dispatch(XrlError::BAD_ARGS());
	return;
    }
    cb->dispatch(e);
}

bool
XrlTestXrlsV0p1Client::send_add_xrl2(
	const char*	dst_xrl_target_name,
	const string&	data1,
	const string&	data2,
	const AddXrl2CB&	cb
)
{
    Xrl* x = ap_xrl_add_xrl2.get();

    if (!x) {
        x = new Xrl(dst_xrl_target_name, "test_xrls/0.1/add_xrl2");
        x->args().add("data1", data1);
        x->args().add("data2", data2);
        ap_xrl_add_xrl2.reset(x);
    }

    x->set_target(dst_xrl_target_name);

    x->args().set_arg(0, data1);
    x->args().set_arg(1, data2);

    return _sender->send(*x, callback(&XrlTestXrlsV0p1Client::unmarshall_add_xrl2, cb));
}


/* Unmarshall add_xrl2 */
void
XrlTestXrlsV0p1Client::unmarshall_add_xrl2(
	const XrlError&	e,
	XrlArgs*	a,
	AddXrl2CB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e);
	return;
    } else if (a && a->size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(0));
	cb->dispatch(XrlError::BAD_ARGS());
	return;
    }
    cb->dispatch(e);
}

bool
XrlTestXrlsV0p1Client::send_add_xrl9(
	const char*	dst_xrl_target_name,
	const bool&	data1,
	const int32_t&	data2,
	const IPv4&	data3,
	const IPv4Net&	data4,
	const IPv6&	data5,
	const IPv6Net&	data6,
	const Mac&	data7,
	const string&	data8,
	const vector<uint8_t>&	data9,
	const AddXrl9CB&	cb
)
{
    Xrl* x = ap_xrl_add_xrl9.get();

    if (!x) {
        x = new Xrl(dst_xrl_target_name, "test_xrls/0.1/add_xrl9");
        x->args().add("data1", data1);
        x->args().add("data2", data2);
        x->args().add("data3", data3);
        x->args().add("data4", data4);
        x->args().add("data5", data5);
        x->args().add("data6", data6);
        x->args().add("data7", data7);
        x->args().add("data8", data8);
        x->args().add("data9", data9);
        ap_xrl_add_xrl9.reset(x);
    }

    x->set_target(dst_xrl_target_name);

    x->args().set_arg(0, data1);
    x->args().set_arg(1, data2);
    x->args().set_arg(2, data3);
    x->args().set_arg(3, data4);
    x->args().set_arg(4, data5);
    x->args().set_arg(5, data6);
    x->args().set_arg(6, data7);
    x->args().set_arg(7, data8);
    x->args().set_arg(8, data9);

    return _sender->send(*x, callback(&XrlTestXrlsV0p1Client::unmarshall_add_xrl9, cb));
}


/* Unmarshall add_xrl9 */
void
XrlTestXrlsV0p1Client::unmarshall_add_xrl9(
	const XrlError&	e,
	XrlArgs*	a,
	AddXrl9CB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e);
	return;
    } else if (a && a->size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(0));
	cb->dispatch(XrlError::BAD_ARGS());
	return;
    }
    cb->dispatch(e);
}

bool
XrlTestXrlsV0p1Client::send_add_xrlx(
	const char*	dst_xrl_target_name,
	const XrlAtomList&	data,
	const AddXrlxCB&	cb
)
{
    Xrl* x = ap_xrl_add_xrlx.get();

    if (!x) {
        x = new Xrl(dst_xrl_target_name, "test_xrls/0.1/add_xrlx");
        x->args().add("data", data);
        ap_xrl_add_xrlx.reset(x);
    }

    x->set_target(dst_xrl_target_name);

    x->args().set_arg(0, data);

    return _sender->send(*x, callback(&XrlTestXrlsV0p1Client::unmarshall_add_xrlx, cb));
}


/* Unmarshall add_xrlx */
void
XrlTestXrlsV0p1Client::unmarshall_add_xrlx(
	const XrlError&	e,
	XrlArgs*	a,
	AddXrlxCB		cb
)
{
    if (e != XrlError::OKAY()) {
	cb->dispatch(e);
	return;
    } else if (a && a->size() != 0) {
	XLOG_ERROR("Wrong number of arguments (%u != %u)", XORP_UINT_CAST(a->size()), XORP_UINT_CAST(0));
	cb->dispatch(XrlError::BAD_ARGS());
	return;
    }
    cb->dispatch(e);
}