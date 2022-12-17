/* access_list_std => "access-list WORD$name [seq (1-4294967295)$seq] <deny|permit>$action <[host] A.B.C.D$host|A.B.C.D$host A.B.C.D$mask>" */
DEFUN_CMD_FUNC_DECL(access_list_std)
#define funcdecl_access_list_std static int access_list_std_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	long seq,\
	const char * seq_str __attribute__ ((unused)),\
	const char * action,\
	struct in_addr host,\
	const char * host_str __attribute__ ((unused)),\
	struct in_addr mask,\
	const char * mask_str __attribute__ ((unused)))
funcdecl_access_list_std;
DEFUN_CMD_FUNC_TEXT(access_list_std)
{
#if 5 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	long seq = 0;
	const char *seq_str = NULL;
	const char *action = NULL;
	struct in_addr host = { INADDR_ANY };
	const char *host_str = NULL;
	struct in_addr mask = { INADDR_ANY };
	const char *mask_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "seq")) {
			seq_str = argv[_i]->arg;
			char *_end;
			seq = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "action")) {
			action = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "host")) {
			host_str = argv[_i]->arg;
			_fail = !inet_aton(argv[_i]->arg, &host);
		}
		if (!strcmp(argv[_i]->varname, "mask")) {
			mask_str = argv[_i]->arg;
			_fail = !inet_aton(argv[_i]->arg, &mask);
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}
	if (!action) {
		vty_out(vty, "Internal CLI error [%s]\n", "action");
		return CMD_WARNING;
	}
	if (!host_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "host_str");
		return CMD_WARNING;
	}

	return access_list_std_magic(self, vty, argc, argv, name, seq, seq_str, action, host, host_str, mask, mask_str);
}

/* no_access_list_std => "no access-list WORD$name [seq (1-4294967295)$seq] <deny|permit>$action <[host] A.B.C.D$host|A.B.C.D$host A.B.C.D$mask>" */
DEFUN_CMD_FUNC_DECL(no_access_list_std)
#define funcdecl_no_access_list_std static int no_access_list_std_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	long seq,\
	const char * seq_str __attribute__ ((unused)),\
	const char * action,\
	struct in_addr host,\
	const char * host_str __attribute__ ((unused)),\
	struct in_addr mask,\
	const char * mask_str __attribute__ ((unused)))
funcdecl_no_access_list_std;
DEFUN_CMD_FUNC_TEXT(no_access_list_std)
{
#if 5 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	long seq = 0;
	const char *seq_str = NULL;
	const char *action = NULL;
	struct in_addr host = { INADDR_ANY };
	const char *host_str = NULL;
	struct in_addr mask = { INADDR_ANY };
	const char *mask_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "seq")) {
			seq_str = argv[_i]->arg;
			char *_end;
			seq = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "action")) {
			action = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "host")) {
			host_str = argv[_i]->arg;
			_fail = !inet_aton(argv[_i]->arg, &host);
		}
		if (!strcmp(argv[_i]->varname, "mask")) {
			mask_str = argv[_i]->arg;
			_fail = !inet_aton(argv[_i]->arg, &mask);
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}
	if (!action) {
		vty_out(vty, "Internal CLI error [%s]\n", "action");
		return CMD_WARNING;
	}
	if (!host_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "host_str");
		return CMD_WARNING;
	}

	return no_access_list_std_magic(self, vty, argc, argv, name, seq, seq_str, action, host, host_str, mask, mask_str);
}

/* access_list_ext => "access-list WORD$name [seq (1-4294967295)$seq] <deny|permit>$action ip <A.B.C.D$src A.B.C.D$src_mask|host A.B.C.D$src|any> <A.B.C.D$dst A.B.C.D$dst_mask|host A.B.C.D$dst|any>" */
DEFUN_CMD_FUNC_DECL(access_list_ext)
#define funcdecl_access_list_ext static int access_list_ext_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	long seq,\
	const char * seq_str __attribute__ ((unused)),\
	const char * action,\
	struct in_addr src,\
	const char * src_str __attribute__ ((unused)),\
	struct in_addr src_mask,\
	const char * src_mask_str __attribute__ ((unused)),\
	struct in_addr dst,\
	const char * dst_str __attribute__ ((unused)),\
	struct in_addr dst_mask,\
	const char * dst_mask_str __attribute__ ((unused)))
funcdecl_access_list_ext;
DEFUN_CMD_FUNC_TEXT(access_list_ext)
{
#if 7 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	long seq = 0;
	const char *seq_str = NULL;
	const char *action = NULL;
	struct in_addr src = { INADDR_ANY };
	const char *src_str = NULL;
	struct in_addr src_mask = { INADDR_ANY };
	const char *src_mask_str = NULL;
	struct in_addr dst = { INADDR_ANY };
	const char *dst_str = NULL;
	struct in_addr dst_mask = { INADDR_ANY };
	const char *dst_mask_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "seq")) {
			seq_str = argv[_i]->arg;
			char *_end;
			seq = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "action")) {
			action = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "src")) {
			src_str = argv[_i]->arg;
			_fail = !inet_aton(argv[_i]->arg, &src);
		}
		if (!strcmp(argv[_i]->varname, "src_mask")) {
			src_mask_str = argv[_i]->arg;
			_fail = !inet_aton(argv[_i]->arg, &src_mask);
		}
		if (!strcmp(argv[_i]->varname, "dst")) {
			dst_str = argv[_i]->arg;
			_fail = !inet_aton(argv[_i]->arg, &dst);
		}
		if (!strcmp(argv[_i]->varname, "dst_mask")) {
			dst_mask_str = argv[_i]->arg;
			_fail = !inet_aton(argv[_i]->arg, &dst_mask);
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}
	if (!action) {
		vty_out(vty, "Internal CLI error [%s]\n", "action");
		return CMD_WARNING;
	}

	return access_list_ext_magic(self, vty, argc, argv, name, seq, seq_str, action, src, src_str, src_mask, src_mask_str, dst, dst_str, dst_mask, dst_mask_str);
}

/* no_access_list_ext => "no access-list WORD$name [seq (1-4294967295)$seq] <deny|permit>$action ip <A.B.C.D$src A.B.C.D$src_mask|host A.B.C.D$src|any> <A.B.C.D$dst A.B.C.D$dst_mask|host A.B.C.D$dst|any>" */
DEFUN_CMD_FUNC_DECL(no_access_list_ext)
#define funcdecl_no_access_list_ext static int no_access_list_ext_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	long seq,\
	const char * seq_str __attribute__ ((unused)),\
	const char * action,\
	struct in_addr src,\
	const char * src_str __attribute__ ((unused)),\
	struct in_addr src_mask,\
	const char * src_mask_str __attribute__ ((unused)),\
	struct in_addr dst,\
	const char * dst_str __attribute__ ((unused)),\
	struct in_addr dst_mask,\
	const char * dst_mask_str __attribute__ ((unused)))
funcdecl_no_access_list_ext;
DEFUN_CMD_FUNC_TEXT(no_access_list_ext)
{
#if 7 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	long seq = 0;
	const char *seq_str = NULL;
	const char *action = NULL;
	struct in_addr src = { INADDR_ANY };
	const char *src_str = NULL;
	struct in_addr src_mask = { INADDR_ANY };
	const char *src_mask_str = NULL;
	struct in_addr dst = { INADDR_ANY };
	const char *dst_str = NULL;
	struct in_addr dst_mask = { INADDR_ANY };
	const char *dst_mask_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "seq")) {
			seq_str = argv[_i]->arg;
			char *_end;
			seq = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "action")) {
			action = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "src")) {
			src_str = argv[_i]->arg;
			_fail = !inet_aton(argv[_i]->arg, &src);
		}
		if (!strcmp(argv[_i]->varname, "src_mask")) {
			src_mask_str = argv[_i]->arg;
			_fail = !inet_aton(argv[_i]->arg, &src_mask);
		}
		if (!strcmp(argv[_i]->varname, "dst")) {
			dst_str = argv[_i]->arg;
			_fail = !inet_aton(argv[_i]->arg, &dst);
		}
		if (!strcmp(argv[_i]->varname, "dst_mask")) {
			dst_mask_str = argv[_i]->arg;
			_fail = !inet_aton(argv[_i]->arg, &dst_mask);
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}
	if (!action) {
		vty_out(vty, "Internal CLI error [%s]\n", "action");
		return CMD_WARNING;
	}

	return no_access_list_ext_magic(self, vty, argc, argv, name, seq, seq_str, action, src, src_str, src_mask, src_mask_str, dst, dst_str, dst_mask, dst_mask_str);
}

/* access_list => "access-list WORD$name [seq (1-4294967295)$seq] <deny|permit>$action <A.B.C.D/M$prefix [exact-match$exact]|any>" */
DEFUN_CMD_FUNC_DECL(access_list)
#define funcdecl_access_list static int access_list_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	long seq,\
	const char * seq_str __attribute__ ((unused)),\
	const char * action,\
	const struct prefix_ipv4 * prefix,\
	const char * prefix_str __attribute__ ((unused)),\
	const char * exact)
funcdecl_access_list;
DEFUN_CMD_FUNC_TEXT(access_list)
{
#if 5 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	long seq = 0;
	const char *seq_str = NULL;
	const char *action = NULL;
	struct prefix_ipv4 prefix = { };
	const char *prefix_str = NULL;
	const char *exact = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "seq")) {
			seq_str = argv[_i]->arg;
			char *_end;
			seq = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "action")) {
			action = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "prefix")) {
			prefix_str = argv[_i]->arg;
			_fail = !str2prefix_ipv4(argv[_i]->arg, &prefix);
		}
		if (!strcmp(argv[_i]->varname, "exact")) {
			exact = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}
	if (!action) {
		vty_out(vty, "Internal CLI error [%s]\n", "action");
		return CMD_WARNING;
	}

	return access_list_magic(self, vty, argc, argv, name, seq, seq_str, action, &prefix, prefix_str, exact);
}

/* no_access_list => "no access-list WORD$name [seq (1-4294967295)$seq] <deny|permit>$action <A.B.C.D/M$prefix [exact-match$exact]|any>" */
DEFUN_CMD_FUNC_DECL(no_access_list)
#define funcdecl_no_access_list static int no_access_list_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	long seq,\
	const char * seq_str __attribute__ ((unused)),\
	const char * action,\
	const struct prefix_ipv4 * prefix,\
	const char * prefix_str __attribute__ ((unused)),\
	const char * exact)
funcdecl_no_access_list;
DEFUN_CMD_FUNC_TEXT(no_access_list)
{
#if 5 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	long seq = 0;
	const char *seq_str = NULL;
	const char *action = NULL;
	struct prefix_ipv4 prefix = { };
	const char *prefix_str = NULL;
	const char *exact = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "seq")) {
			seq_str = argv[_i]->arg;
			char *_end;
			seq = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "action")) {
			action = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "prefix")) {
			prefix_str = argv[_i]->arg;
			_fail = !str2prefix_ipv4(argv[_i]->arg, &prefix);
		}
		if (!strcmp(argv[_i]->varname, "exact")) {
			exact = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}
	if (!action) {
		vty_out(vty, "Internal CLI error [%s]\n", "action");
		return CMD_WARNING;
	}

	return no_access_list_magic(self, vty, argc, argv, name, seq, seq_str, action, &prefix, prefix_str, exact);
}

/* no_access_list_all => "no access-list WORD$name" */
DEFUN_CMD_FUNC_DECL(no_access_list_all)
#define funcdecl_no_access_list_all static int no_access_list_all_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name)
funcdecl_no_access_list_all;
DEFUN_CMD_FUNC_TEXT(no_access_list_all)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}

	return no_access_list_all_magic(self, vty, argc, argv, name);
}

/* access_list_remark => "access-list WORD$name remark LINE..." */
DEFUN_CMD_FUNC_DECL(access_list_remark)
#define funcdecl_access_list_remark static int access_list_remark_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	const char * line)
funcdecl_access_list_remark;
DEFUN_CMD_FUNC_TEXT(access_list_remark)
{
#if 2 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	const char *line = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "line")) {
			line = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}
	if (!line) {
		vty_out(vty, "Internal CLI error [%s]\n", "line");
		return CMD_WARNING;
	}

	return access_list_remark_magic(self, vty, argc, argv, name, line);
}

/* no_access_list_remark => "no access-list WORD$name remark" */
DEFUN_CMD_FUNC_DECL(no_access_list_remark)
#define funcdecl_no_access_list_remark static int no_access_list_remark_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name)
funcdecl_no_access_list_remark;
DEFUN_CMD_FUNC_TEXT(no_access_list_remark)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}

	return no_access_list_remark_magic(self, vty, argc, argv, name);
}

/* ipv6_access_list => "ipv6 access-list WORD$name [seq (1-4294967295)$seq] <deny|permit>$action <X:X::X:X/M$prefix [exact-match$exact]|any>" */
DEFUN_CMD_FUNC_DECL(ipv6_access_list)
#define funcdecl_ipv6_access_list static int ipv6_access_list_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	long seq,\
	const char * seq_str __attribute__ ((unused)),\
	const char * action,\
	const struct prefix_ipv6 * prefix,\
	const char * prefix_str __attribute__ ((unused)),\
	const char * exact)
funcdecl_ipv6_access_list;
DEFUN_CMD_FUNC_TEXT(ipv6_access_list)
{
#if 5 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	long seq = 0;
	const char *seq_str = NULL;
	const char *action = NULL;
	struct prefix_ipv6 prefix = { };
	const char *prefix_str = NULL;
	const char *exact = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "seq")) {
			seq_str = argv[_i]->arg;
			char *_end;
			seq = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "action")) {
			action = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "prefix")) {
			prefix_str = argv[_i]->arg;
			_fail = !str2prefix_ipv6(argv[_i]->arg, &prefix);
		}
		if (!strcmp(argv[_i]->varname, "exact")) {
			exact = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}
	if (!action) {
		vty_out(vty, "Internal CLI error [%s]\n", "action");
		return CMD_WARNING;
	}

	return ipv6_access_list_magic(self, vty, argc, argv, name, seq, seq_str, action, &prefix, prefix_str, exact);
}

/* no_ipv6_access_list => "no ipv6 access-list WORD$name [seq (1-4294967295)$seq] <deny|permit>$action <X:X::X:X/M$prefix [exact-match$exact]|any>" */
DEFUN_CMD_FUNC_DECL(no_ipv6_access_list)
#define funcdecl_no_ipv6_access_list static int no_ipv6_access_list_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	long seq,\
	const char * seq_str __attribute__ ((unused)),\
	const char * action,\
	const struct prefix_ipv6 * prefix,\
	const char * prefix_str __attribute__ ((unused)),\
	const char * exact)
funcdecl_no_ipv6_access_list;
DEFUN_CMD_FUNC_TEXT(no_ipv6_access_list)
{
#if 5 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	long seq = 0;
	const char *seq_str = NULL;
	const char *action = NULL;
	struct prefix_ipv6 prefix = { };
	const char *prefix_str = NULL;
	const char *exact = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "seq")) {
			seq_str = argv[_i]->arg;
			char *_end;
			seq = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "action")) {
			action = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "prefix")) {
			prefix_str = argv[_i]->arg;
			_fail = !str2prefix_ipv6(argv[_i]->arg, &prefix);
		}
		if (!strcmp(argv[_i]->varname, "exact")) {
			exact = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}
	if (!action) {
		vty_out(vty, "Internal CLI error [%s]\n", "action");
		return CMD_WARNING;
	}

	return no_ipv6_access_list_magic(self, vty, argc, argv, name, seq, seq_str, action, &prefix, prefix_str, exact);
}

/* no_ipv6_access_list_all => "no ipv6 access-list WORD$name" */
DEFUN_CMD_FUNC_DECL(no_ipv6_access_list_all)
#define funcdecl_no_ipv6_access_list_all static int no_ipv6_access_list_all_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name)
funcdecl_no_ipv6_access_list_all;
DEFUN_CMD_FUNC_TEXT(no_ipv6_access_list_all)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}

	return no_ipv6_access_list_all_magic(self, vty, argc, argv, name);
}

/* ipv6_access_list_remark => "ipv6 access-list WORD$name remark LINE..." */
DEFUN_CMD_FUNC_DECL(ipv6_access_list_remark)
#define funcdecl_ipv6_access_list_remark static int ipv6_access_list_remark_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	const char * line)
funcdecl_ipv6_access_list_remark;
DEFUN_CMD_FUNC_TEXT(ipv6_access_list_remark)
{
#if 2 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	const char *line = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "line")) {
			line = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}
	if (!line) {
		vty_out(vty, "Internal CLI error [%s]\n", "line");
		return CMD_WARNING;
	}

	return ipv6_access_list_remark_magic(self, vty, argc, argv, name, line);
}

/* no_ipv6_access_list_remark => "no ipv6 access-list WORD$name remark" */
DEFUN_CMD_FUNC_DECL(no_ipv6_access_list_remark)
#define funcdecl_no_ipv6_access_list_remark static int no_ipv6_access_list_remark_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name)
funcdecl_no_ipv6_access_list_remark;
DEFUN_CMD_FUNC_TEXT(no_ipv6_access_list_remark)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}

	return no_ipv6_access_list_remark_magic(self, vty, argc, argv, name);
}

/* mac_access_list => "mac access-list ACCESSLIST_MAC_NAME$name [seq (1-4294967295)$seq] <deny|permit>$action <X:X:X:X:X:X$mac|any>" */
DEFUN_CMD_FUNC_DECL(mac_access_list)
#define funcdecl_mac_access_list static int mac_access_list_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	long seq,\
	const char * seq_str __attribute__ ((unused)),\
	const char * action,\
	struct prefix_eth * mac,\
	const char * mac_str __attribute__ ((unused)))
funcdecl_mac_access_list;
DEFUN_CMD_FUNC_TEXT(mac_access_list)
{
#if 4 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	long seq = 0;
	const char *seq_str = NULL;
	const char *action = NULL;
	struct prefix_eth mac = { };
	const char *mac_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "seq")) {
			seq_str = argv[_i]->arg;
			char *_end;
			seq = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "action")) {
			action = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "mac")) {
			mac_str = argv[_i]->arg;
			_fail = !str2prefix_eth(argv[_i]->arg, &mac);
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}
	if (!action) {
		vty_out(vty, "Internal CLI error [%s]\n", "action");
		return CMD_WARNING;
	}

	return mac_access_list_magic(self, vty, argc, argv, name, seq, seq_str, action, &mac, mac_str);
}

/* no_mac_access_list => "no mac access-list ACCESSLIST_MAC_NAME$name [seq (1-4294967295)$seq] <deny|permit>$action <X:X:X:X:X:X$mac|any>" */
DEFUN_CMD_FUNC_DECL(no_mac_access_list)
#define funcdecl_no_mac_access_list static int no_mac_access_list_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	long seq,\
	const char * seq_str __attribute__ ((unused)),\
	const char * action,\
	struct prefix_eth * mac,\
	const char * mac_str __attribute__ ((unused)))
funcdecl_no_mac_access_list;
DEFUN_CMD_FUNC_TEXT(no_mac_access_list)
{
#if 4 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	long seq = 0;
	const char *seq_str = NULL;
	const char *action = NULL;
	struct prefix_eth mac = { };
	const char *mac_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "seq")) {
			seq_str = argv[_i]->arg;
			char *_end;
			seq = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "action")) {
			action = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "mac")) {
			mac_str = argv[_i]->arg;
			_fail = !str2prefix_eth(argv[_i]->arg, &mac);
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}
	if (!action) {
		vty_out(vty, "Internal CLI error [%s]\n", "action");
		return CMD_WARNING;
	}

	return no_mac_access_list_magic(self, vty, argc, argv, name, seq, seq_str, action, &mac, mac_str);
}

/* no_mac_access_list_all => "no mac access-list ACCESSLIST_MAC_NAME$name" */
DEFUN_CMD_FUNC_DECL(no_mac_access_list_all)
#define funcdecl_no_mac_access_list_all static int no_mac_access_list_all_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name)
funcdecl_no_mac_access_list_all;
DEFUN_CMD_FUNC_TEXT(no_mac_access_list_all)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}

	return no_mac_access_list_all_magic(self, vty, argc, argv, name);
}

/* mac_access_list_remark => "mac access-list ACCESSLIST_MAC_NAME$name remark LINE..." */
DEFUN_CMD_FUNC_DECL(mac_access_list_remark)
#define funcdecl_mac_access_list_remark static int mac_access_list_remark_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	const char * line)
funcdecl_mac_access_list_remark;
DEFUN_CMD_FUNC_TEXT(mac_access_list_remark)
{
#if 2 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	const char *line = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "line")) {
			line = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}
	if (!line) {
		vty_out(vty, "Internal CLI error [%s]\n", "line");
		return CMD_WARNING;
	}

	return mac_access_list_remark_magic(self, vty, argc, argv, name, line);
}

/* no_mac_access_list_remark => "no mac access-list ACCESSLIST_MAC_NAME$name remark" */
DEFUN_CMD_FUNC_DECL(no_mac_access_list_remark)
#define funcdecl_no_mac_access_list_remark static int no_mac_access_list_remark_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name)
funcdecl_no_mac_access_list_remark;
DEFUN_CMD_FUNC_TEXT(no_mac_access_list_remark)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}

	return no_mac_access_list_remark_magic(self, vty, argc, argv, name);
}

/* ip_prefix_list => "ip prefix-list WORD$name [seq (1-4294967295)$seq] <deny|permit>$action <any|A.B.C.D/M$prefix [{ge (0-32)$ge|le (0-32)$le}]>" */
DEFUN_CMD_FUNC_DECL(ip_prefix_list)
#define funcdecl_ip_prefix_list static int ip_prefix_list_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	long seq,\
	const char * seq_str __attribute__ ((unused)),\
	const char * action,\
	const struct prefix_ipv4 * prefix,\
	const char * prefix_str __attribute__ ((unused)),\
	long ge,\
	const char * ge_str __attribute__ ((unused)),\
	long le,\
	const char * le_str __attribute__ ((unused)))
funcdecl_ip_prefix_list;
DEFUN_CMD_FUNC_TEXT(ip_prefix_list)
{
#if 6 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	long seq = 0;
	const char *seq_str = NULL;
	const char *action = NULL;
	struct prefix_ipv4 prefix = { };
	const char *prefix_str = NULL;
	long ge = 0;
	const char *ge_str = NULL;
	long le = 0;
	const char *le_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "seq")) {
			seq_str = argv[_i]->arg;
			char *_end;
			seq = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "action")) {
			action = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "prefix")) {
			prefix_str = argv[_i]->arg;
			_fail = !str2prefix_ipv4(argv[_i]->arg, &prefix);
		}
		if (!strcmp(argv[_i]->varname, "ge")) {
			ge_str = argv[_i]->arg;
			char *_end;
			ge = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "le")) {
			le_str = argv[_i]->arg;
			char *_end;
			le = strtol(argv[_i]->arg, &_end, 10);
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}
	if (!action) {
		vty_out(vty, "Internal CLI error [%s]\n", "action");
		return CMD_WARNING;
	}

	return ip_prefix_list_magic(self, vty, argc, argv, name, seq, seq_str, action, &prefix, prefix_str, ge, ge_str, le, le_str);
}

/* no_ip_prefix_list => "no ip prefix-list WORD$name [seq (1-4294967295)$seq] <deny|permit>$action <any|A.B.C.D/M$prefix [{ge (0-32)|le (0-32)}]>" */
DEFUN_CMD_FUNC_DECL(no_ip_prefix_list)
#define funcdecl_no_ip_prefix_list static int no_ip_prefix_list_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	long seq,\
	const char * seq_str __attribute__ ((unused)),\
	const char * action,\
	const struct prefix_ipv4 * prefix,\
	const char * prefix_str __attribute__ ((unused)),\
	long ge,\
	const char * ge_str __attribute__ ((unused)),\
	long le,\
	const char * le_str __attribute__ ((unused)))
funcdecl_no_ip_prefix_list;
DEFUN_CMD_FUNC_TEXT(no_ip_prefix_list)
{
#if 6 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	long seq = 0;
	const char *seq_str = NULL;
	const char *action = NULL;
	struct prefix_ipv4 prefix = { };
	const char *prefix_str = NULL;
	long ge = 0;
	const char *ge_str = NULL;
	long le = 0;
	const char *le_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "seq")) {
			seq_str = argv[_i]->arg;
			char *_end;
			seq = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "action")) {
			action = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "prefix")) {
			prefix_str = argv[_i]->arg;
			_fail = !str2prefix_ipv4(argv[_i]->arg, &prefix);
		}
		if (!strcmp(argv[_i]->varname, "ge")) {
			ge_str = argv[_i]->arg;
			char *_end;
			ge = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "le")) {
			le_str = argv[_i]->arg;
			char *_end;
			le = strtol(argv[_i]->arg, &_end, 10);
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}
	if (!action) {
		vty_out(vty, "Internal CLI error [%s]\n", "action");
		return CMD_WARNING;
	}

	return no_ip_prefix_list_magic(self, vty, argc, argv, name, seq, seq_str, action, &prefix, prefix_str, ge, ge_str, le, le_str);
}

/* no_ip_prefix_list_seq => "no ip prefix-list WORD$name seq (1-4294967295)$seq" */
DEFUN_CMD_FUNC_DECL(no_ip_prefix_list_seq)
#define funcdecl_no_ip_prefix_list_seq static int no_ip_prefix_list_seq_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	long seq,\
	const char * seq_str __attribute__ ((unused)))
funcdecl_no_ip_prefix_list_seq;
DEFUN_CMD_FUNC_TEXT(no_ip_prefix_list_seq)
{
#if 2 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	long seq = 0;
	const char *seq_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "seq")) {
			seq_str = argv[_i]->arg;
			char *_end;
			seq = strtol(argv[_i]->arg, &_end, 10);
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}
	if (!seq_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "seq_str");
		return CMD_WARNING;
	}

	return no_ip_prefix_list_seq_magic(self, vty, argc, argv, name, seq, seq_str);
}

/* no_ip_prefix_list_all => "no ip prefix-list WORD$name" */
DEFUN_CMD_FUNC_DECL(no_ip_prefix_list_all)
#define funcdecl_no_ip_prefix_list_all static int no_ip_prefix_list_all_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name)
funcdecl_no_ip_prefix_list_all;
DEFUN_CMD_FUNC_TEXT(no_ip_prefix_list_all)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}

	return no_ip_prefix_list_all_magic(self, vty, argc, argv, name);
}

/* ip_prefix_list_remark => "ip prefix-list WORD$name description LINE..." */
DEFUN_CMD_FUNC_DECL(ip_prefix_list_remark)
#define funcdecl_ip_prefix_list_remark static int ip_prefix_list_remark_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	const char * line)
funcdecl_ip_prefix_list_remark;
DEFUN_CMD_FUNC_TEXT(ip_prefix_list_remark)
{
#if 2 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	const char *line = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "line")) {
			line = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}
	if (!line) {
		vty_out(vty, "Internal CLI error [%s]\n", "line");
		return CMD_WARNING;
	}

	return ip_prefix_list_remark_magic(self, vty, argc, argv, name, line);
}

/* no_ip_prefix_list_remark => "no ip prefix-list WORD$name description" */
DEFUN_CMD_FUNC_DECL(no_ip_prefix_list_remark)
#define funcdecl_no_ip_prefix_list_remark static int no_ip_prefix_list_remark_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name)
funcdecl_no_ip_prefix_list_remark;
DEFUN_CMD_FUNC_TEXT(no_ip_prefix_list_remark)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}

	return no_ip_prefix_list_remark_magic(self, vty, argc, argv, name);
}

/* ipv6_prefix_list => "ipv6 prefix-list WORD$name [seq (1-4294967295)] <deny|permit>$action <any|X:X::X:X/M$prefix [{ge (0-128)$ge|le (0-128)$le}]>" */
DEFUN_CMD_FUNC_DECL(ipv6_prefix_list)
#define funcdecl_ipv6_prefix_list static int ipv6_prefix_list_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	long seq,\
	const char * seq_str __attribute__ ((unused)),\
	const char * action,\
	const struct prefix_ipv6 * prefix,\
	const char * prefix_str __attribute__ ((unused)),\
	long ge,\
	const char * ge_str __attribute__ ((unused)),\
	long le,\
	const char * le_str __attribute__ ((unused)))
funcdecl_ipv6_prefix_list;
DEFUN_CMD_FUNC_TEXT(ipv6_prefix_list)
{
#if 6 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	long seq = 0;
	const char *seq_str = NULL;
	const char *action = NULL;
	struct prefix_ipv6 prefix = { };
	const char *prefix_str = NULL;
	long ge = 0;
	const char *ge_str = NULL;
	long le = 0;
	const char *le_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "seq")) {
			seq_str = argv[_i]->arg;
			char *_end;
			seq = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "action")) {
			action = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "prefix")) {
			prefix_str = argv[_i]->arg;
			_fail = !str2prefix_ipv6(argv[_i]->arg, &prefix);
		}
		if (!strcmp(argv[_i]->varname, "ge")) {
			ge_str = argv[_i]->arg;
			char *_end;
			ge = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "le")) {
			le_str = argv[_i]->arg;
			char *_end;
			le = strtol(argv[_i]->arg, &_end, 10);
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}
	if (!action) {
		vty_out(vty, "Internal CLI error [%s]\n", "action");
		return CMD_WARNING;
	}

	return ipv6_prefix_list_magic(self, vty, argc, argv, name, seq, seq_str, action, &prefix, prefix_str, ge, ge_str, le, le_str);
}

/* no_ipv6_prefix_list => "no ipv6 prefix-list WORD$name [seq (1-4294967295)$seq] <deny|permit>$action <any|X:X::X:X/M$prefix [{ge (0-128)$ge|le (0-128)$le}]>" */
DEFUN_CMD_FUNC_DECL(no_ipv6_prefix_list)
#define funcdecl_no_ipv6_prefix_list static int no_ipv6_prefix_list_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	long seq,\
	const char * seq_str __attribute__ ((unused)),\
	const char * action,\
	const struct prefix_ipv6 * prefix,\
	const char * prefix_str __attribute__ ((unused)),\
	long ge,\
	const char * ge_str __attribute__ ((unused)),\
	long le,\
	const char * le_str __attribute__ ((unused)))
funcdecl_no_ipv6_prefix_list;
DEFUN_CMD_FUNC_TEXT(no_ipv6_prefix_list)
{
#if 6 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	long seq = 0;
	const char *seq_str = NULL;
	const char *action = NULL;
	struct prefix_ipv6 prefix = { };
	const char *prefix_str = NULL;
	long ge = 0;
	const char *ge_str = NULL;
	long le = 0;
	const char *le_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "seq")) {
			seq_str = argv[_i]->arg;
			char *_end;
			seq = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "action")) {
			action = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "prefix")) {
			prefix_str = argv[_i]->arg;
			_fail = !str2prefix_ipv6(argv[_i]->arg, &prefix);
		}
		if (!strcmp(argv[_i]->varname, "ge")) {
			ge_str = argv[_i]->arg;
			char *_end;
			ge = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "le")) {
			le_str = argv[_i]->arg;
			char *_end;
			le = strtol(argv[_i]->arg, &_end, 10);
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}
	if (!action) {
		vty_out(vty, "Internal CLI error [%s]\n", "action");
		return CMD_WARNING;
	}

	return no_ipv6_prefix_list_magic(self, vty, argc, argv, name, seq, seq_str, action, &prefix, prefix_str, ge, ge_str, le, le_str);
}

/* no_ipv6_prefix_list_seq => "no ipv6 prefix-list WORD$name seq (1-4294967295)$seq" */
DEFUN_CMD_FUNC_DECL(no_ipv6_prefix_list_seq)
#define funcdecl_no_ipv6_prefix_list_seq static int no_ipv6_prefix_list_seq_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	long seq,\
	const char * seq_str __attribute__ ((unused)))
funcdecl_no_ipv6_prefix_list_seq;
DEFUN_CMD_FUNC_TEXT(no_ipv6_prefix_list_seq)
{
#if 2 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	long seq = 0;
	const char *seq_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "seq")) {
			seq_str = argv[_i]->arg;
			char *_end;
			seq = strtol(argv[_i]->arg, &_end, 10);
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}
	if (!seq_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "seq_str");
		return CMD_WARNING;
	}

	return no_ipv6_prefix_list_seq_magic(self, vty, argc, argv, name, seq, seq_str);
}

/* no_ipv6_prefix_list_all => "no ipv6 prefix-list WORD$name" */
DEFUN_CMD_FUNC_DECL(no_ipv6_prefix_list_all)
#define funcdecl_no_ipv6_prefix_list_all static int no_ipv6_prefix_list_all_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name)
funcdecl_no_ipv6_prefix_list_all;
DEFUN_CMD_FUNC_TEXT(no_ipv6_prefix_list_all)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}

	return no_ipv6_prefix_list_all_magic(self, vty, argc, argv, name);
}

/* ipv6_prefix_list_remark => "ipv6 prefix-list WORD$name description LINE..." */
DEFUN_CMD_FUNC_DECL(ipv6_prefix_list_remark)
#define funcdecl_ipv6_prefix_list_remark static int ipv6_prefix_list_remark_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	const char * line)
funcdecl_ipv6_prefix_list_remark;
DEFUN_CMD_FUNC_TEXT(ipv6_prefix_list_remark)
{
#if 2 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	const char *line = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "line")) {
			line = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}
	if (!line) {
		vty_out(vty, "Internal CLI error [%s]\n", "line");
		return CMD_WARNING;
	}

	return ipv6_prefix_list_remark_magic(self, vty, argc, argv, name, line);
}

/* no_ipv6_prefix_list_remark => "no ipv6 prefix-list WORD$name description" */
DEFUN_CMD_FUNC_DECL(no_ipv6_prefix_list_remark)
#define funcdecl_no_ipv6_prefix_list_remark static int no_ipv6_prefix_list_remark_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name)
funcdecl_no_ipv6_prefix_list_remark;
DEFUN_CMD_FUNC_TEXT(no_ipv6_prefix_list_remark)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!name) {
		vty_out(vty, "Internal CLI error [%s]\n", "name");
		return CMD_WARNING;
	}

	return no_ipv6_prefix_list_remark_magic(self, vty, argc, argv, name);
}

