/* ospf_router_id => "ospf router-id A.B.C.D" */
DEFUN_CMD_FUNC_DECL(ospf_router_id)
#define funcdecl_ospf_router_id static int ospf_router_id_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	struct in_addr router_id,\
	const char * router_id_str __attribute__ ((unused)))
funcdecl_ospf_router_id;
DEFUN_CMD_FUNC_TEXT(ospf_router_id)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	struct in_addr router_id = { INADDR_ANY };
	const char *router_id_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "router_id")) {
			router_id_str = argv[_i]->arg;
			_fail = !inet_aton(argv[_i]->arg, &router_id);
		}
#if 1 /* anything that can fail? */
		if (_fail)
			vty_out (vty, "%% invalid input for %s: %s\n",
				   argv[_i]->varname, argv[_i]->arg);
		_failcnt += _fail;
#endif
	}
#if 1 /* anything that can fail? */
	if (_failcnt)
		return CMD_WARNING;
#endif
#endif
	if (!router_id_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "router_id_str");
		return CMD_WARNING;
	}

	return ospf_router_id_magic(self, vty, argc, argv, router_id, router_id_str);
}

/* no_ospf_router_id => "no ospf router-id [A.B.C.D]" */
DEFUN_CMD_FUNC_DECL(no_ospf_router_id)
#define funcdecl_no_ospf_router_id static int no_ospf_router_id_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	struct in_addr router_id,\
	const char * router_id_str __attribute__ ((unused)))
funcdecl_no_ospf_router_id;
DEFUN_CMD_FUNC_TEXT(no_ospf_router_id)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	struct in_addr router_id = { INADDR_ANY };
	const char *router_id_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "router_id")) {
			router_id_str = argv[_i]->arg;
			_fail = !inet_aton(argv[_i]->arg, &router_id);
		}
#if 1 /* anything that can fail? */
		if (_fail)
			vty_out (vty, "%% invalid input for %s: %s\n",
				   argv[_i]->varname, argv[_i]->arg);
		_failcnt += _fail;
#endif
	}
#if 1 /* anything that can fail? */
	if (_failcnt)
		return CMD_WARNING;
#endif
#endif

	return no_ospf_router_id_magic(self, vty, argc, argv, router_id, router_id_str);
}

/* ospf_send_extra_data => "[no] ospf send-extra-data zebra" */
DEFUN_CMD_FUNC_DECL(ospf_send_extra_data)
#define funcdecl_ospf_send_extra_data static int ospf_send_extra_data_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * no)
funcdecl_ospf_send_extra_data;
DEFUN_CMD_FUNC_TEXT(ospf_send_extra_data)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *no = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "no")) {
			no = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
#if 0 /* anything that can fail? */
		if (_fail)
			vty_out (vty, "%% invalid input for %s: %s\n",
				   argv[_i]->varname, argv[_i]->arg);
		_failcnt += _fail;
#endif
	}
#if 0 /* anything that can fail? */
	if (_failcnt)
		return CMD_WARNING;
#endif
#endif

	return ospf_send_extra_data_magic(self, vty, argc, argv, no);
}

/* show_ip_ospf_neighbor_id => "show ip ospf neighbor A.B.C.D$router_id [detail$detail] [json$json]" */
DEFUN_CMD_FUNC_DECL(show_ip_ospf_neighbor_id)
#define funcdecl_show_ip_ospf_neighbor_id static int show_ip_ospf_neighbor_id_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	struct in_addr router_id,\
	const char * router_id_str __attribute__ ((unused)),\
	const char * detail,\
	const char * json)
funcdecl_show_ip_ospf_neighbor_id;
DEFUN_CMD_FUNC_TEXT(show_ip_ospf_neighbor_id)
{
#if 3 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	struct in_addr router_id = { INADDR_ANY };
	const char *router_id_str = NULL;
	const char *detail = NULL;
	const char *json = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "router_id")) {
			router_id_str = argv[_i]->arg;
			_fail = !inet_aton(argv[_i]->arg, &router_id);
		}
		if (!strcmp(argv[_i]->varname, "detail")) {
			detail = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "json")) {
			json = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
#if 1 /* anything that can fail? */
		if (_fail)
			vty_out (vty, "%% invalid input for %s: %s\n",
				   argv[_i]->varname, argv[_i]->arg);
		_failcnt += _fail;
#endif
	}
#if 1 /* anything that can fail? */
	if (_failcnt)
		return CMD_WARNING;
#endif
#endif
	if (!router_id_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "router_id_str");
		return CMD_WARNING;
	}

	return show_ip_ospf_neighbor_id_magic(self, vty, argc, argv, router_id, router_id_str, detail, json);
}

/* show_ip_ospf_instance_neighbor_id => "show ip ospf (1-65535)$instance neighbor A.B.C.D$router_id [detail$detail] [json$json]" */
DEFUN_CMD_FUNC_DECL(show_ip_ospf_instance_neighbor_id)
#define funcdecl_show_ip_ospf_instance_neighbor_id static int show_ip_ospf_instance_neighbor_id_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long instance,\
	const char * instance_str __attribute__ ((unused)),\
	struct in_addr router_id,\
	const char * router_id_str __attribute__ ((unused)),\
	const char * detail,\
	const char * json)
funcdecl_show_ip_ospf_instance_neighbor_id;
DEFUN_CMD_FUNC_TEXT(show_ip_ospf_instance_neighbor_id)
{
#if 4 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long instance = 0;
	const char *instance_str = NULL;
	struct in_addr router_id = { INADDR_ANY };
	const char *router_id_str = NULL;
	const char *detail = NULL;
	const char *json = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "instance")) {
			instance_str = argv[_i]->arg;
			char *_end;
			instance = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "router_id")) {
			router_id_str = argv[_i]->arg;
			_fail = !inet_aton(argv[_i]->arg, &router_id);
		}
		if (!strcmp(argv[_i]->varname, "detail")) {
			detail = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "json")) {
			json = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
#if 1 /* anything that can fail? */
		if (_fail)
			vty_out (vty, "%% invalid input for %s: %s\n",
				   argv[_i]->varname, argv[_i]->arg);
		_failcnt += _fail;
#endif
	}
#if 1 /* anything that can fail? */
	if (_failcnt)
		return CMD_WARNING;
#endif
#endif
	if (!instance_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "instance_str");
		return CMD_WARNING;
	}
	if (!router_id_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "router_id_str");
		return CMD_WARNING;
	}

	return show_ip_ospf_instance_neighbor_id_magic(self, vty, argc, argv, instance, instance_str, router_id, router_id_str, detail, json);
}

/* ospf_gr_helper_enable => "graceful-restart helper enable [A.B.C.D$address]" */
DEFUN_CMD_FUNC_DECL(ospf_gr_helper_enable)
#define funcdecl_ospf_gr_helper_enable static int ospf_gr_helper_enable_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	struct in_addr address,\
	const char * address_str __attribute__ ((unused)))
funcdecl_ospf_gr_helper_enable;
DEFUN_CMD_FUNC_TEXT(ospf_gr_helper_enable)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	struct in_addr address = { INADDR_ANY };
	const char *address_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "address")) {
			address_str = argv[_i]->arg;
			_fail = !inet_aton(argv[_i]->arg, &address);
		}
#if 1 /* anything that can fail? */
		if (_fail)
			vty_out (vty, "%% invalid input for %s: %s\n",
				   argv[_i]->varname, argv[_i]->arg);
		_failcnt += _fail;
#endif
	}
#if 1 /* anything that can fail? */
	if (_failcnt)
		return CMD_WARNING;
#endif
#endif

	return ospf_gr_helper_enable_magic(self, vty, argc, argv, address, address_str);
}

/* no_ospf_gr_helper_enable => "no graceful-restart helper enable [A.B.C.D$address]" */
DEFUN_CMD_FUNC_DECL(no_ospf_gr_helper_enable)
#define funcdecl_no_ospf_gr_helper_enable static int no_ospf_gr_helper_enable_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	struct in_addr address,\
	const char * address_str __attribute__ ((unused)))
funcdecl_no_ospf_gr_helper_enable;
DEFUN_CMD_FUNC_TEXT(no_ospf_gr_helper_enable)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	struct in_addr address = { INADDR_ANY };
	const char *address_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "address")) {
			address_str = argv[_i]->arg;
			_fail = !inet_aton(argv[_i]->arg, &address);
		}
#if 1 /* anything that can fail? */
		if (_fail)
			vty_out (vty, "%% invalid input for %s: %s\n",
				   argv[_i]->varname, argv[_i]->arg);
		_failcnt += _fail;
#endif
	}
#if 1 /* anything that can fail? */
	if (_failcnt)
		return CMD_WARNING;
#endif
#endif

	return no_ospf_gr_helper_enable_magic(self, vty, argc, argv, address, address_str);
}

/* ospf_gr_helper_enable_lsacheck => "graceful-restart helper strict-lsa-checking" */
DEFUN_CMD_FUNC_DECL(ospf_gr_helper_enable_lsacheck)
#define funcdecl_ospf_gr_helper_enable_lsacheck static int ospf_gr_helper_enable_lsacheck_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)))
funcdecl_ospf_gr_helper_enable_lsacheck;
DEFUN_CMD_FUNC_TEXT(ospf_gr_helper_enable_lsacheck)
{
#if 0 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

#if 0 /* anything that can fail? */
		if (_fail)
			vty_out (vty, "%% invalid input for %s: %s\n",
				   argv[_i]->varname, argv[_i]->arg);
		_failcnt += _fail;
#endif
	}
#if 0 /* anything that can fail? */
	if (_failcnt)
		return CMD_WARNING;
#endif
#endif

	return ospf_gr_helper_enable_lsacheck_magic(self, vty, argc, argv);
}

/* no_ospf_gr_helper_enable_lsacheck => "no graceful-restart helper strict-lsa-checking" */
DEFUN_CMD_FUNC_DECL(no_ospf_gr_helper_enable_lsacheck)
#define funcdecl_no_ospf_gr_helper_enable_lsacheck static int no_ospf_gr_helper_enable_lsacheck_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)))
funcdecl_no_ospf_gr_helper_enable_lsacheck;
DEFUN_CMD_FUNC_TEXT(no_ospf_gr_helper_enable_lsacheck)
{
#if 0 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

#if 0 /* anything that can fail? */
		if (_fail)
			vty_out (vty, "%% invalid input for %s: %s\n",
				   argv[_i]->varname, argv[_i]->arg);
		_failcnt += _fail;
#endif
	}
#if 0 /* anything that can fail? */
	if (_failcnt)
		return CMD_WARNING;
#endif
#endif

	return no_ospf_gr_helper_enable_lsacheck_magic(self, vty, argc, argv);
}

/* ospf_gr_helper_supported_grace_time => "graceful-restart helper supported-grace-time (10-1800)$interval" */
DEFUN_CMD_FUNC_DECL(ospf_gr_helper_supported_grace_time)
#define funcdecl_ospf_gr_helper_supported_grace_time static int ospf_gr_helper_supported_grace_time_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long interval,\
	const char * interval_str __attribute__ ((unused)))
funcdecl_ospf_gr_helper_supported_grace_time;
DEFUN_CMD_FUNC_TEXT(ospf_gr_helper_supported_grace_time)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long interval = 0;
	const char *interval_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "interval")) {
			interval_str = argv[_i]->arg;
			char *_end;
			interval = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
#if 1 /* anything that can fail? */
		if (_fail)
			vty_out (vty, "%% invalid input for %s: %s\n",
				   argv[_i]->varname, argv[_i]->arg);
		_failcnt += _fail;
#endif
	}
#if 1 /* anything that can fail? */
	if (_failcnt)
		return CMD_WARNING;
#endif
#endif
	if (!interval_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "interval_str");
		return CMD_WARNING;
	}

	return ospf_gr_helper_supported_grace_time_magic(self, vty, argc, argv, interval, interval_str);
}

/* no_ospf_gr_helper_supported_grace_time => "no graceful-restart helper supported-grace-time (10-1800)$interval" */
DEFUN_CMD_FUNC_DECL(no_ospf_gr_helper_supported_grace_time)
#define funcdecl_no_ospf_gr_helper_supported_grace_time static int no_ospf_gr_helper_supported_grace_time_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long interval,\
	const char * interval_str __attribute__ ((unused)))
funcdecl_no_ospf_gr_helper_supported_grace_time;
DEFUN_CMD_FUNC_TEXT(no_ospf_gr_helper_supported_grace_time)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long interval = 0;
	const char *interval_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "interval")) {
			interval_str = argv[_i]->arg;
			char *_end;
			interval = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
#if 1 /* anything that can fail? */
		if (_fail)
			vty_out (vty, "%% invalid input for %s: %s\n",
				   argv[_i]->varname, argv[_i]->arg);
		_failcnt += _fail;
#endif
	}
#if 1 /* anything that can fail? */
	if (_failcnt)
		return CMD_WARNING;
#endif
#endif
	if (!interval_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "interval_str");
		return CMD_WARNING;
	}

	return no_ospf_gr_helper_supported_grace_time_magic(self, vty, argc, argv, interval, interval_str);
}

/* ospf_gr_helper_planned_only => "graceful-restart helper planned-only" */
DEFUN_CMD_FUNC_DECL(ospf_gr_helper_planned_only)
#define funcdecl_ospf_gr_helper_planned_only static int ospf_gr_helper_planned_only_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)))
funcdecl_ospf_gr_helper_planned_only;
DEFUN_CMD_FUNC_TEXT(ospf_gr_helper_planned_only)
{
#if 0 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

#if 0 /* anything that can fail? */
		if (_fail)
			vty_out (vty, "%% invalid input for %s: %s\n",
				   argv[_i]->varname, argv[_i]->arg);
		_failcnt += _fail;
#endif
	}
#if 0 /* anything that can fail? */
	if (_failcnt)
		return CMD_WARNING;
#endif
#endif

	return ospf_gr_helper_planned_only_magic(self, vty, argc, argv);
}

/* no_ospf_gr_helper_planned_only => "no graceful-restart helper planned-only" */
DEFUN_CMD_FUNC_DECL(no_ospf_gr_helper_planned_only)
#define funcdecl_no_ospf_gr_helper_planned_only static int no_ospf_gr_helper_planned_only_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)))
funcdecl_no_ospf_gr_helper_planned_only;
DEFUN_CMD_FUNC_TEXT(no_ospf_gr_helper_planned_only)
{
#if 0 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

#if 0 /* anything that can fail? */
		if (_fail)
			vty_out (vty, "%% invalid input for %s: %s\n",
				   argv[_i]->varname, argv[_i]->arg);
		_failcnt += _fail;
#endif
	}
#if 0 /* anything that can fail? */
	if (_failcnt)
		return CMD_WARNING;
#endif
#endif

	return no_ospf_gr_helper_planned_only_magic(self, vty, argc, argv);
}

/* show_ip_ospf_gr_helper => "show ip ospf [vrf <NAME|all>] graceful-restart helper [detail] [json]" */
DEFUN_CMD_FUNC_DECL(show_ip_ospf_gr_helper)
#define funcdecl_show_ip_ospf_gr_helper static int show_ip_ospf_gr_helper_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * vrf)
funcdecl_show_ip_ospf_gr_helper;
DEFUN_CMD_FUNC_TEXT(show_ip_ospf_gr_helper)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *vrf = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "vrf")) {
			vrf = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
#if 0 /* anything that can fail? */
		if (_fail)
			vty_out (vty, "%% invalid input for %s: %s\n",
				   argv[_i]->varname, argv[_i]->arg);
		_failcnt += _fail;
#endif
	}
#if 0 /* anything that can fail? */
	if (_failcnt)
		return CMD_WARNING;
#endif
#endif

	return show_ip_ospf_gr_helper_magic(self, vty, argc, argv, vrf);
}

/* show_ip_ospf_instance_route_orr => "show ip ospf (1-65535)$instance route orr [WORD$orr_group]" */
DEFUN_CMD_FUNC_DECL(show_ip_ospf_instance_route_orr)
#define funcdecl_show_ip_ospf_instance_route_orr static int show_ip_ospf_instance_route_orr_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long instance,\
	const char * instance_str __attribute__ ((unused)),\
	const char * orr_group)
funcdecl_show_ip_ospf_instance_route_orr;
DEFUN_CMD_FUNC_TEXT(show_ip_ospf_instance_route_orr)
{
#if 2 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long instance = 0;
	const char *instance_str = NULL;
	const char *orr_group = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "instance")) {
			instance_str = argv[_i]->arg;
			char *_end;
			instance = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "orr_group")) {
			orr_group = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
#if 1 /* anything that can fail? */
		if (_fail)
			vty_out (vty, "%% invalid input for %s: %s\n",
				   argv[_i]->varname, argv[_i]->arg);
		_failcnt += _fail;
#endif
	}
#if 1 /* anything that can fail? */
	if (_failcnt)
		return CMD_WARNING;
#endif
#endif
	if (!instance_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "instance_str");
		return CMD_WARNING;
	}

	return show_ip_ospf_instance_route_orr_magic(self, vty, argc, argv, instance, instance_str, orr_group);
}

/* show_ip_ospf_route_orr => "show ip ospf [vrf <NAME$vrf_name|all$all_vrf>] route orr [WORD$orr_group]" */
DEFUN_CMD_FUNC_DECL(show_ip_ospf_route_orr)
#define funcdecl_show_ip_ospf_route_orr static int show_ip_ospf_route_orr_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * vrf_name,\
	const char * all_vrf,\
	const char * orr_group)
funcdecl_show_ip_ospf_route_orr;
DEFUN_CMD_FUNC_TEXT(show_ip_ospf_route_orr)
{
#if 3 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *vrf_name = NULL;
	const char *all_vrf = NULL;
	const char *orr_group = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "vrf_name")) {
			vrf_name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "all_vrf")) {
			all_vrf = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "orr_group")) {
			orr_group = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
#if 0 /* anything that can fail? */
		if (_fail)
			vty_out (vty, "%% invalid input for %s: %s\n",
				   argv[_i]->varname, argv[_i]->arg);
		_failcnt += _fail;
#endif
	}
#if 0 /* anything that can fail? */
	if (_failcnt)
		return CMD_WARNING;
#endif
#endif

	return show_ip_ospf_route_orr_magic(self, vty, argc, argv, vrf_name, all_vrf, orr_group);
}

/* clear_ip_ospf_neighbor => "clear ip ospf [(1-65535)]$instance neighbor [A.B.C.D$nbr_id]" */
DEFUN_CMD_FUNC_DECL(clear_ip_ospf_neighbor)
#define funcdecl_clear_ip_ospf_neighbor static int clear_ip_ospf_neighbor_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long instance,\
	const char * instance_str __attribute__ ((unused)),\
	struct in_addr nbr_id,\
	const char * nbr_id_str __attribute__ ((unused)))
funcdecl_clear_ip_ospf_neighbor;
DEFUN_CMD_FUNC_TEXT(clear_ip_ospf_neighbor)
{
#if 2 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long instance = 0;
	const char *instance_str = NULL;
	struct in_addr nbr_id = { INADDR_ANY };
	const char *nbr_id_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "instance")) {
			instance_str = argv[_i]->arg;
			char *_end;
			instance = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "nbr_id")) {
			nbr_id_str = argv[_i]->arg;
			_fail = !inet_aton(argv[_i]->arg, &nbr_id);
		}
#if 1 /* anything that can fail? */
		if (_fail)
			vty_out (vty, "%% invalid input for %s: %s\n",
				   argv[_i]->varname, argv[_i]->arg);
		_failcnt += _fail;
#endif
	}
#if 1 /* anything that can fail? */
	if (_failcnt)
		return CMD_WARNING;
#endif
#endif

	return clear_ip_ospf_neighbor_magic(self, vty, argc, argv, instance, instance_str, nbr_id, nbr_id_str);
}

/* clear_ip_ospf_process => "clear ip ospf [(1-65535)]$instance process" */
DEFUN_CMD_FUNC_DECL(clear_ip_ospf_process)
#define funcdecl_clear_ip_ospf_process static int clear_ip_ospf_process_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long instance,\
	const char * instance_str __attribute__ ((unused)))
funcdecl_clear_ip_ospf_process;
DEFUN_CMD_FUNC_TEXT(clear_ip_ospf_process)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long instance = 0;
	const char *instance_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "instance")) {
			instance_str = argv[_i]->arg;
			char *_end;
			instance = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
#if 1 /* anything that can fail? */
		if (_fail)
			vty_out (vty, "%% invalid input for %s: %s\n",
				   argv[_i]->varname, argv[_i]->arg);
		_failcnt += _fail;
#endif
	}
#if 1 /* anything that can fail? */
	if (_failcnt)
		return CMD_WARNING;
#endif
#endif

	return clear_ip_ospf_process_magic(self, vty, argc, argv, instance, instance_str);
}

/* ospf_lsa_refresh_timer => "[no$no] ospf lsa-refresh [(120-1800)]$value" */
DEFUN_CMD_FUNC_DECL(ospf_lsa_refresh_timer)
#define funcdecl_ospf_lsa_refresh_timer static int ospf_lsa_refresh_timer_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * no,\
	long value,\
	const char * value_str __attribute__ ((unused)))
funcdecl_ospf_lsa_refresh_timer;
DEFUN_CMD_FUNC_TEXT(ospf_lsa_refresh_timer)
{
#if 2 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *no = NULL;
	long value = 0;
	const char *value_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "no")) {
			no = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "value")) {
			value_str = argv[_i]->arg;
			char *_end;
			value = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
#if 1 /* anything that can fail? */
		if (_fail)
			vty_out (vty, "%% invalid input for %s: %s\n",
				   argv[_i]->varname, argv[_i]->arg);
		_failcnt += _fail;
#endif
	}
#if 1 /* anything that can fail? */
	if (_failcnt)
		return CMD_WARNING;
#endif
#endif

	return ospf_lsa_refresh_timer_magic(self, vty, argc, argv, no, value, value_str);
}

/* ospf_maxage_delay_timer => "[no$no] ospf maxage-delay [(0-60)]$value" */
DEFUN_CMD_FUNC_DECL(ospf_maxage_delay_timer)
#define funcdecl_ospf_maxage_delay_timer static int ospf_maxage_delay_timer_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * no,\
	long value,\
	const char * value_str __attribute__ ((unused)))
funcdecl_ospf_maxage_delay_timer;
DEFUN_CMD_FUNC_TEXT(ospf_maxage_delay_timer)
{
#if 2 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *no = NULL;
	long value = 0;
	const char *value_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "no")) {
			no = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "value")) {
			value_str = argv[_i]->arg;
			char *_end;
			value = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
#if 1 /* anything that can fail? */
		if (_fail)
			vty_out (vty, "%% invalid input for %s: %s\n",
				   argv[_i]->varname, argv[_i]->arg);
		_failcnt += _fail;
#endif
	}
#if 1 /* anything that can fail? */
	if (_failcnt)
		return CMD_WARNING;
#endif
#endif

	return ospf_maxage_delay_timer_magic(self, vty, argc, argv, no, value, value_str);
}

