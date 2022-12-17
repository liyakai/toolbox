/* service_cputime_stats => "[no] service cputime-stats" */
DEFUN_CMD_FUNC_DECL(service_cputime_stats)
#define funcdecl_service_cputime_stats static int service_cputime_stats_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * no)
funcdecl_service_cputime_stats;
DEFUN_CMD_FUNC_TEXT(service_cputime_stats)
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

	return service_cputime_stats_magic(self, vty, argc, argv, no);
}

/* service_cputime_warning => "[no] service cputime-warning (1-4294967295)" */
DEFUN_CMD_FUNC_DECL(service_cputime_warning)
#define funcdecl_service_cputime_warning static int service_cputime_warning_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * no,\
	long cputime_warning,\
	const char * cputime_warning_str __attribute__ ((unused)))
funcdecl_service_cputime_warning;
DEFUN_CMD_FUNC_TEXT(service_cputime_warning)
{
#if 2 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *no = NULL;
	long cputime_warning = 0;
	const char *cputime_warning_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "no")) {
			no = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "cputime_warning")) {
			cputime_warning_str = argv[_i]->arg;
			char *_end;
			cputime_warning = strtol(argv[_i]->arg, &_end, 10);
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
	if (!cputime_warning_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "cputime_warning_str");
		return CMD_WARNING;
	}

	return service_cputime_warning_magic(self, vty, argc, argv, no, cputime_warning, cputime_warning_str);
}

/* service_walltime_warning => "[no] service walltime-warning (1-4294967295)" */
DEFUN_CMD_FUNC_DECL(service_walltime_warning)
#define funcdecl_service_walltime_warning static int service_walltime_warning_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * no,\
	long walltime_warning,\
	const char * walltime_warning_str __attribute__ ((unused)))
funcdecl_service_walltime_warning;
DEFUN_CMD_FUNC_TEXT(service_walltime_warning)
{
#if 2 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *no = NULL;
	long walltime_warning = 0;
	const char *walltime_warning_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "no")) {
			no = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "walltime_warning")) {
			walltime_warning_str = argv[_i]->arg;
			char *_end;
			walltime_warning = strtol(argv[_i]->arg, &_end, 10);
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
	if (!walltime_warning_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "walltime_warning_str");
		return CMD_WARNING;
	}

	return service_walltime_warning_magic(self, vty, argc, argv, no, walltime_warning, walltime_warning_str);
}

/* show_thread_timers => "show thread timers" */
DEFUN_CMD_FUNC_DECL(show_thread_timers)
#define funcdecl_show_thread_timers static int show_thread_timers_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)))
funcdecl_show_thread_timers;
DEFUN_CMD_FUNC_TEXT(show_thread_timers)
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

	return show_thread_timers_magic(self, vty, argc, argv);
}

