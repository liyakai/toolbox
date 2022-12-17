/* show_ip_prefix_list => "show ip prefix-list [WORD [seq$dseq (1-4294967295)$arg]] [json$uj]" */
DEFUN_CMD_FUNC_DECL(show_ip_prefix_list)
#define funcdecl_show_ip_prefix_list static int show_ip_prefix_list_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * prefix_list,\
	const char * dseq,\
	long arg,\
	const char * arg_str __attribute__ ((unused)),\
	const char * uj)
funcdecl_show_ip_prefix_list;
DEFUN_CMD_FUNC_TEXT(show_ip_prefix_list)
{
#if 4 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *prefix_list = NULL;
	const char *dseq = NULL;
	long arg = 0;
	const char *arg_str = NULL;
	const char *uj = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "prefix_list")) {
			prefix_list = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "dseq")) {
			dseq = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "arg")) {
			arg_str = argv[_i]->arg;
			char *_end;
			arg = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "uj")) {
			uj = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return show_ip_prefix_list_magic(self, vty, argc, argv, prefix_list, dseq, arg, arg_str, uj);
}

/* show_ip_prefix_list_prefix => "show ip prefix-list WORD A.B.C.D/M$prefix [longer$dl|first-match$dfm]" */
DEFUN_CMD_FUNC_DECL(show_ip_prefix_list_prefix)
#define funcdecl_show_ip_prefix_list_prefix static int show_ip_prefix_list_prefix_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * prefix_list,\
	const struct prefix_ipv4 * prefix,\
	const char * prefix_str __attribute__ ((unused)),\
	const char * dl,\
	const char * dfm)
funcdecl_show_ip_prefix_list_prefix;
DEFUN_CMD_FUNC_TEXT(show_ip_prefix_list_prefix)
{
#if 4 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *prefix_list = NULL;
	struct prefix_ipv4 prefix = { };
	const char *prefix_str = NULL;
	const char *dl = NULL;
	const char *dfm = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "prefix_list")) {
			prefix_list = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "prefix")) {
			prefix_str = argv[_i]->arg;
			_fail = !str2prefix_ipv4(argv[_i]->arg, &prefix);
		}
		if (!strcmp(argv[_i]->varname, "dl")) {
			dl = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "dfm")) {
			dfm = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!prefix_list) {
		vty_out(vty, "Internal CLI error [%s]\n", "prefix_list");
		return CMD_WARNING;
	}
	if (!prefix_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "prefix_str");
		return CMD_WARNING;
	}

	return show_ip_prefix_list_prefix_magic(self, vty, argc, argv, prefix_list, &prefix, prefix_str, dl, dfm);
}

/* show_ip_prefix_list_summary => "show ip prefix-list summary [WORD$prefix_list] [json$uj]" */
DEFUN_CMD_FUNC_DECL(show_ip_prefix_list_summary)
#define funcdecl_show_ip_prefix_list_summary static int show_ip_prefix_list_summary_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * prefix_list,\
	const char * uj)
funcdecl_show_ip_prefix_list_summary;
DEFUN_CMD_FUNC_TEXT(show_ip_prefix_list_summary)
{
#if 2 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *prefix_list = NULL;
	const char *uj = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "prefix_list")) {
			prefix_list = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "uj")) {
			uj = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return show_ip_prefix_list_summary_magic(self, vty, argc, argv, prefix_list, uj);
}

/* show_ip_prefix_list_detail => "show ip prefix-list detail [WORD$prefix_list] [json$uj]" */
DEFUN_CMD_FUNC_DECL(show_ip_prefix_list_detail)
#define funcdecl_show_ip_prefix_list_detail static int show_ip_prefix_list_detail_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * prefix_list,\
	const char * uj)
funcdecl_show_ip_prefix_list_detail;
DEFUN_CMD_FUNC_TEXT(show_ip_prefix_list_detail)
{
#if 2 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *prefix_list = NULL;
	const char *uj = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "prefix_list")) {
			prefix_list = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "uj")) {
			uj = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return show_ip_prefix_list_detail_magic(self, vty, argc, argv, prefix_list, uj);
}

/* clear_ip_prefix_list => "clear ip prefix-list [WORD [A.B.C.D/M$prefix]]" */
DEFUN_CMD_FUNC_DECL(clear_ip_prefix_list)
#define funcdecl_clear_ip_prefix_list static int clear_ip_prefix_list_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * prefix_list,\
	const struct prefix_ipv4 * prefix,\
	const char * prefix_str __attribute__ ((unused)))
funcdecl_clear_ip_prefix_list;
DEFUN_CMD_FUNC_TEXT(clear_ip_prefix_list)
{
#if 2 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *prefix_list = NULL;
	struct prefix_ipv4 prefix = { };
	const char *prefix_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "prefix_list")) {
			prefix_list = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "prefix")) {
			prefix_str = argv[_i]->arg;
			_fail = !str2prefix_ipv4(argv[_i]->arg, &prefix);
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

	return clear_ip_prefix_list_magic(self, vty, argc, argv, prefix_list, &prefix, prefix_str);
}

/* show_ipv6_prefix_list => "show ipv6 prefix-list [WORD [seq$dseq (1-4294967295)$arg]] [json$uj]" */
DEFUN_CMD_FUNC_DECL(show_ipv6_prefix_list)
#define funcdecl_show_ipv6_prefix_list static int show_ipv6_prefix_list_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * prefix_list,\
	const char * dseq,\
	long arg,\
	const char * arg_str __attribute__ ((unused)),\
	const char * uj)
funcdecl_show_ipv6_prefix_list;
DEFUN_CMD_FUNC_TEXT(show_ipv6_prefix_list)
{
#if 4 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *prefix_list = NULL;
	const char *dseq = NULL;
	long arg = 0;
	const char *arg_str = NULL;
	const char *uj = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "prefix_list")) {
			prefix_list = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "dseq")) {
			dseq = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "arg")) {
			arg_str = argv[_i]->arg;
			char *_end;
			arg = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "uj")) {
			uj = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return show_ipv6_prefix_list_magic(self, vty, argc, argv, prefix_list, dseq, arg, arg_str, uj);
}

/* show_ipv6_prefix_list_prefix => "show ipv6 prefix-list WORD X:X::X:X/M$prefix [longer$dl|first-match$dfm]" */
DEFUN_CMD_FUNC_DECL(show_ipv6_prefix_list_prefix)
#define funcdecl_show_ipv6_prefix_list_prefix static int show_ipv6_prefix_list_prefix_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * prefix_list,\
	const struct prefix_ipv6 * prefix,\
	const char * prefix_str __attribute__ ((unused)),\
	const char * dl,\
	const char * dfm)
funcdecl_show_ipv6_prefix_list_prefix;
DEFUN_CMD_FUNC_TEXT(show_ipv6_prefix_list_prefix)
{
#if 4 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *prefix_list = NULL;
	struct prefix_ipv6 prefix = { };
	const char *prefix_str = NULL;
	const char *dl = NULL;
	const char *dfm = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "prefix_list")) {
			prefix_list = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "prefix")) {
			prefix_str = argv[_i]->arg;
			_fail = !str2prefix_ipv6(argv[_i]->arg, &prefix);
		}
		if (!strcmp(argv[_i]->varname, "dl")) {
			dl = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "dfm")) {
			dfm = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!prefix_list) {
		vty_out(vty, "Internal CLI error [%s]\n", "prefix_list");
		return CMD_WARNING;
	}
	if (!prefix_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "prefix_str");
		return CMD_WARNING;
	}

	return show_ipv6_prefix_list_prefix_magic(self, vty, argc, argv, prefix_list, &prefix, prefix_str, dl, dfm);
}

/* show_ipv6_prefix_list_summary => "show ipv6 prefix-list summary [WORD$prefix-list] [json$uj]" */
DEFUN_CMD_FUNC_DECL(show_ipv6_prefix_list_summary)
#define funcdecl_show_ipv6_prefix_list_summary static int show_ipv6_prefix_list_summary_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * prefix_list,\
	const char * uj)
funcdecl_show_ipv6_prefix_list_summary;
DEFUN_CMD_FUNC_TEXT(show_ipv6_prefix_list_summary)
{
#if 2 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *prefix_list = NULL;
	const char *uj = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "prefix_list")) {
			prefix_list = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "uj")) {
			uj = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return show_ipv6_prefix_list_summary_magic(self, vty, argc, argv, prefix_list, uj);
}

/* show_ipv6_prefix_list_detail => "show ipv6 prefix-list detail [WORD$prefix-list] [json$uj]" */
DEFUN_CMD_FUNC_DECL(show_ipv6_prefix_list_detail)
#define funcdecl_show_ipv6_prefix_list_detail static int show_ipv6_prefix_list_detail_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * prefix_list,\
	const char * uj)
funcdecl_show_ipv6_prefix_list_detail;
DEFUN_CMD_FUNC_TEXT(show_ipv6_prefix_list_detail)
{
#if 2 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *prefix_list = NULL;
	const char *uj = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "prefix_list")) {
			prefix_list = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "uj")) {
			uj = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return show_ipv6_prefix_list_detail_magic(self, vty, argc, argv, prefix_list, uj);
}

/* clear_ipv6_prefix_list => "clear ipv6 prefix-list [WORD [X:X::X:X/M$prefix]]" */
DEFUN_CMD_FUNC_DECL(clear_ipv6_prefix_list)
#define funcdecl_clear_ipv6_prefix_list static int clear_ipv6_prefix_list_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * prefix_list,\
	const struct prefix_ipv6 * prefix,\
	const char * prefix_str __attribute__ ((unused)))
funcdecl_clear_ipv6_prefix_list;
DEFUN_CMD_FUNC_TEXT(clear_ipv6_prefix_list)
{
#if 2 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *prefix_list = NULL;
	struct prefix_ipv6 prefix = { };
	const char *prefix_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "prefix_list")) {
			prefix_list = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "prefix")) {
			prefix_str = argv[_i]->arg;
			_fail = !str2prefix_ipv6(argv[_i]->arg, &prefix);
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

	return clear_ipv6_prefix_list_magic(self, vty, argc, argv, prefix_list, &prefix, prefix_str);
}

/* debug_prefix_list_match => "debug prefix-list WORD$prefix-list match <A.B.C.D/M|X:X::X:X/M> [address-mode$addr_mode]" */
DEFUN_CMD_FUNC_DECL(debug_prefix_list_match)
#define funcdecl_debug_prefix_list_match static int debug_prefix_list_match_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * prefix_list,\
	const struct prefix * match,\
	const char * match_str __attribute__ ((unused)),\
	const char * addr_mode)
funcdecl_debug_prefix_list_match;
DEFUN_CMD_FUNC_TEXT(debug_prefix_list_match)
{
#if 3 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *prefix_list = NULL;
	struct prefix match = { };
	const char *match_str = NULL;
	const char *addr_mode = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "prefix_list")) {
			prefix_list = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "match")) {
			match_str = argv[_i]->arg;
			_fail = !str2prefix(argv[_i]->arg, &match);
		}
		if (!strcmp(argv[_i]->varname, "addr_mode")) {
			addr_mode = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!prefix_list) {
		vty_out(vty, "Internal CLI error [%s]\n", "prefix_list");
		return CMD_WARNING;
	}
	if (!match_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "match_str");
		return CMD_WARNING;
	}

	return debug_prefix_list_match_magic(self, vty, argc, argv, prefix_list, &match, match_str, addr_mode);
}

