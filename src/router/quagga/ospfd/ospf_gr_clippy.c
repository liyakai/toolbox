/* graceful_restart_prepare => "graceful-restart prepare ip ospf" */
DEFUN_CMD_FUNC_DECL(graceful_restart_prepare)
#define funcdecl_graceful_restart_prepare static int graceful_restart_prepare_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)))
funcdecl_graceful_restart_prepare;
DEFUN_CMD_FUNC_TEXT(graceful_restart_prepare)
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

	return graceful_restart_prepare_magic(self, vty, argc, argv);
}

/* graceful_restart => "graceful-restart [grace-period (1-1800)$grace_period]" */
DEFUN_CMD_FUNC_DECL(graceful_restart)
#define funcdecl_graceful_restart static int graceful_restart_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long grace_period,\
	const char * grace_period_str __attribute__ ((unused)))
funcdecl_graceful_restart;
DEFUN_CMD_FUNC_TEXT(graceful_restart)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long grace_period = 0;
	const char *grace_period_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "grace_period")) {
			grace_period_str = argv[_i]->arg;
			char *_end;
			grace_period = strtol(argv[_i]->arg, &_end, 10);
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

	return graceful_restart_magic(self, vty, argc, argv, grace_period, grace_period_str);
}

/* no_graceful_restart => "no graceful-restart [grace-period (1-1800)]" */
DEFUN_CMD_FUNC_DECL(no_graceful_restart)
#define funcdecl_no_graceful_restart static int no_graceful_restart_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long grace_period,\
	const char * grace_period_str __attribute__ ((unused)))
funcdecl_no_graceful_restart;
DEFUN_CMD_FUNC_TEXT(no_graceful_restart)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long grace_period = 0;
	const char *grace_period_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "grace_period")) {
			grace_period_str = argv[_i]->arg;
			char *_end;
			grace_period = strtol(argv[_i]->arg, &_end, 10);
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

	return no_graceful_restart_magic(self, vty, argc, argv, grace_period, grace_period_str);
}

