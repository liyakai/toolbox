/* ospf_mpls_ldp_sync => "mpls ldp-sync" */
DEFUN_CMD_FUNC_DECL(ospf_mpls_ldp_sync)
#define funcdecl_ospf_mpls_ldp_sync static int ospf_mpls_ldp_sync_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)))
funcdecl_ospf_mpls_ldp_sync;
DEFUN_CMD_FUNC_TEXT(ospf_mpls_ldp_sync)
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

	return ospf_mpls_ldp_sync_magic(self, vty, argc, argv);
}

/* no_ospf_mpls_ldp_sync => "no mpls ldp-sync" */
DEFUN_CMD_FUNC_DECL(no_ospf_mpls_ldp_sync)
#define funcdecl_no_ospf_mpls_ldp_sync static int no_ospf_mpls_ldp_sync_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)))
funcdecl_no_ospf_mpls_ldp_sync;
DEFUN_CMD_FUNC_TEXT(no_ospf_mpls_ldp_sync)
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

	return no_ospf_mpls_ldp_sync_magic(self, vty, argc, argv);
}

/* ospf_mpls_ldp_sync_holddown => "mpls ldp-sync holddown (1-10000)" */
DEFUN_CMD_FUNC_DECL(ospf_mpls_ldp_sync_holddown)
#define funcdecl_ospf_mpls_ldp_sync_holddown static int ospf_mpls_ldp_sync_holddown_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long holddown,\
	const char * holddown_str __attribute__ ((unused)))
funcdecl_ospf_mpls_ldp_sync_holddown;
DEFUN_CMD_FUNC_TEXT(ospf_mpls_ldp_sync_holddown)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long holddown = 0;
	const char *holddown_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "holddown")) {
			holddown_str = argv[_i]->arg;
			char *_end;
			holddown = strtol(argv[_i]->arg, &_end, 10);
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
	if (!holddown_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "holddown_str");
		return CMD_WARNING;
	}

	return ospf_mpls_ldp_sync_holddown_magic(self, vty, argc, argv, holddown, holddown_str);
}

/* no_ospf_mpls_ldp_sync_holddown => "no mpls ldp-sync holddown [<(1-10000)>]" */
DEFUN_CMD_FUNC_DECL(no_ospf_mpls_ldp_sync_holddown)
#define funcdecl_no_ospf_mpls_ldp_sync_holddown static int no_ospf_mpls_ldp_sync_holddown_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long holddown,\
	const char * holddown_str __attribute__ ((unused)))
funcdecl_no_ospf_mpls_ldp_sync_holddown;
DEFUN_CMD_FUNC_TEXT(no_ospf_mpls_ldp_sync_holddown)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long holddown = 0;
	const char *holddown_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "holddown")) {
			holddown_str = argv[_i]->arg;
			char *_end;
			holddown = strtol(argv[_i]->arg, &_end, 10);
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

	return no_ospf_mpls_ldp_sync_holddown_magic(self, vty, argc, argv, holddown, holddown_str);
}

/* mpls_ldp_sync => "ip ospf mpls ldp-sync" */
DEFUN_CMD_FUNC_DECL(mpls_ldp_sync)
#define funcdecl_mpls_ldp_sync static int mpls_ldp_sync_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)))
funcdecl_mpls_ldp_sync;
DEFUN_CMD_FUNC_TEXT(mpls_ldp_sync)
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

	return mpls_ldp_sync_magic(self, vty, argc, argv);
}

/* no_mpls_ldp_sync => "no ip ospf mpls ldp-sync" */
DEFUN_CMD_FUNC_DECL(no_mpls_ldp_sync)
#define funcdecl_no_mpls_ldp_sync static int no_mpls_ldp_sync_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)))
funcdecl_no_mpls_ldp_sync;
DEFUN_CMD_FUNC_TEXT(no_mpls_ldp_sync)
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

	return no_mpls_ldp_sync_magic(self, vty, argc, argv);
}

/* mpls_ldp_sync_holddown => "ip ospf mpls ldp-sync holddown (0-10000)" */
DEFUN_CMD_FUNC_DECL(mpls_ldp_sync_holddown)
#define funcdecl_mpls_ldp_sync_holddown static int mpls_ldp_sync_holddown_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long holddown,\
	const char * holddown_str __attribute__ ((unused)))
funcdecl_mpls_ldp_sync_holddown;
DEFUN_CMD_FUNC_TEXT(mpls_ldp_sync_holddown)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long holddown = 0;
	const char *holddown_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "holddown")) {
			holddown_str = argv[_i]->arg;
			char *_end;
			holddown = strtol(argv[_i]->arg, &_end, 10);
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
	if (!holddown_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "holddown_str");
		return CMD_WARNING;
	}

	return mpls_ldp_sync_holddown_magic(self, vty, argc, argv, holddown, holddown_str);
}

/* no_mpls_ldp_sync_holddown => "no ip ospf mpls ldp-sync holddown [<(1-10000)>]" */
DEFUN_CMD_FUNC_DECL(no_mpls_ldp_sync_holddown)
#define funcdecl_no_mpls_ldp_sync_holddown static int no_mpls_ldp_sync_holddown_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long holddown,\
	const char * holddown_str __attribute__ ((unused)))
funcdecl_no_mpls_ldp_sync_holddown;
DEFUN_CMD_FUNC_TEXT(no_mpls_ldp_sync_holddown)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long holddown = 0;
	const char *holddown_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "holddown")) {
			holddown_str = argv[_i]->arg;
			char *_end;
			holddown = strtol(argv[_i]->arg, &_end, 10);
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

	return no_mpls_ldp_sync_holddown_magic(self, vty, argc, argv, holddown, holddown_str);
}

/* show_ip_ospf_mpls_ldp_interface => "show ip ospf mpls ldp-sync [interface <INTERFACE|all>] [json]" */
DEFUN_CMD_FUNC_DECL(show_ip_ospf_mpls_ldp_interface)
#define funcdecl_show_ip_ospf_mpls_ldp_interface static int show_ip_ospf_mpls_ldp_interface_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * interface)
funcdecl_show_ip_ospf_mpls_ldp_interface;
DEFUN_CMD_FUNC_TEXT(show_ip_ospf_mpls_ldp_interface)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *interface = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "interface")) {
			interface = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return show_ip_ospf_mpls_ldp_interface_magic(self, vty, argc, argv, interface);
}

