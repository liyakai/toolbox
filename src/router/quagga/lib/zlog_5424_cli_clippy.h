/* log_5424_target => "log extended-syslog EXTLOGNAME" */
DEFUN_CMD_FUNC_DECL(log_5424_target)
#define funcdecl_log_5424_target static int log_5424_target_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * extlogname)
funcdecl_log_5424_target;
DEFUN_CMD_FUNC_TEXT(log_5424_target)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *extlogname = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "extlogname")) {
			extlogname = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!extlogname) {
		vty_out(vty, "Internal CLI error [%s]\n", "extlogname");
		return CMD_WARNING;
	}

	return log_5424_target_magic(self, vty, argc, argv, extlogname);
}

/* no_log_5424_target => "no log extended-syslog EXTLOGNAME" */
DEFUN_CMD_FUNC_DECL(no_log_5424_target)
#define funcdecl_no_log_5424_target static int no_log_5424_target_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * extlogname)
funcdecl_no_log_5424_target;
DEFUN_CMD_FUNC_TEXT(no_log_5424_target)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *extlogname = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "extlogname")) {
			extlogname = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!extlogname) {
		vty_out(vty, "Internal CLI error [%s]\n", "extlogname");
		return CMD_WARNING;
	}

	return no_log_5424_target_magic(self, vty, argc, argv, extlogname);
}

/* log_5424_destination_file => "[no] destination file$type PATH [create$create [{user WORD|group WORD|mode PERMS}]|no-create$nocreate] [format <rfc3164|rfc5424|local-syslogd|journald>$fmt]" */
DEFUN_CMD_FUNC_DECL(log_5424_destination_file)
#define funcdecl_log_5424_destination_file static int log_5424_destination_file_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * no,\
	const char * type,\
	const char * path,\
	const char * create,\
	const char * user,\
	const char * group,\
	const char * perms,\
	const char * nocreate,\
	const char * fmt)
funcdecl_log_5424_destination_file;
DEFUN_CMD_FUNC_TEXT(log_5424_destination_file)
{
#if 9 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *no = NULL;
	const char *type = NULL;
	const char *path = NULL;
	const char *create = NULL;
	const char *user = NULL;
	const char *group = NULL;
	const char *perms = NULL;
	const char *nocreate = NULL;
	const char *fmt = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "no")) {
			no = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "type")) {
			type = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "path")) {
			path = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "create")) {
			create = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "user")) {
			user = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "group")) {
			group = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "perms")) {
			perms = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "nocreate")) {
			nocreate = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "fmt")) {
			fmt = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!type) {
		vty_out(vty, "Internal CLI error [%s]\n", "type");
		return CMD_WARNING;
	}
	if (!path) {
		vty_out(vty, "Internal CLI error [%s]\n", "path");
		return CMD_WARNING;
	}

	return log_5424_destination_file_magic(self, vty, argc, argv, no, type, path, create, user, group, perms, nocreate, fmt);
}

/* log_5424_destination_unix => "[no] destination unix PATH [format <rfc3164|rfc5424|local-syslogd|journald>$fmt]" */
DEFUN_CMD_FUNC_DECL(log_5424_destination_unix)
#define funcdecl_log_5424_destination_unix static int log_5424_destination_unix_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * no,\
	const char * path,\
	const char * fmt)
funcdecl_log_5424_destination_unix;
DEFUN_CMD_FUNC_TEXT(log_5424_destination_unix)
{
#if 3 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *no = NULL;
	const char *path = NULL;
	const char *fmt = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "no")) {
			no = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "path")) {
			path = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "fmt")) {
			fmt = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!path) {
		vty_out(vty, "Internal CLI error [%s]\n", "path");
		return CMD_WARNING;
	}

	return log_5424_destination_unix_magic(self, vty, argc, argv, no, path, fmt);
}

/* log_5424_destination_journald => "[no] destination journald" */
DEFUN_CMD_FUNC_DECL(log_5424_destination_journald)
#define funcdecl_log_5424_destination_journald static int log_5424_destination_journald_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * no)
funcdecl_log_5424_destination_journald;
DEFUN_CMD_FUNC_TEXT(log_5424_destination_journald)
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

	return log_5424_destination_journald_magic(self, vty, argc, argv, no);
}

/* log_5424_destination_syslog => "[no] destination syslog [supports-rfc5424]$supp5424" */
DEFUN_CMD_FUNC_DECL(log_5424_destination_syslog)
#define funcdecl_log_5424_destination_syslog static int log_5424_destination_syslog_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * no,\
	const char * supp5424)
funcdecl_log_5424_destination_syslog;
DEFUN_CMD_FUNC_TEXT(log_5424_destination_syslog)
{
#if 2 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *no = NULL;
	const char *supp5424 = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "no")) {
			no = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "supp5424")) {
			supp5424 = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return log_5424_destination_syslog_magic(self, vty, argc, argv, no, supp5424);
}

/* log_5424_destination_fd => "[no] destination <fd <(0-63)$fd|envvar WORD>|stdout$fd1|stderr$fd2>[format <rfc3164|rfc5424|local-syslogd|journald>$fmt]" */
DEFUN_CMD_FUNC_DECL(log_5424_destination_fd)
#define funcdecl_log_5424_destination_fd static int log_5424_destination_fd_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * no,\
	long fd,\
	const char * fd_str __attribute__ ((unused)),\
	const char * envvar,\
	const char * fd1,\
	const char * fd2,\
	const char * fmt)
funcdecl_log_5424_destination_fd;
DEFUN_CMD_FUNC_TEXT(log_5424_destination_fd)
{
#if 6 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *no = NULL;
	long fd = 0;
	const char *fd_str = NULL;
	const char *envvar = NULL;
	const char *fd1 = NULL;
	const char *fd2 = NULL;
	const char *fmt = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "no")) {
			no = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "fd")) {
			fd_str = argv[_i]->arg;
			char *_end;
			fd = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "envvar")) {
			envvar = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "fd1")) {
			fd1 = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "fd2")) {
			fd2 = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "fmt")) {
			fmt = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return log_5424_destination_fd_magic(self, vty, argc, argv, no, fd, fd_str, envvar, fd1, fd2, fmt);
}

/* log_5424_destination_none => "[no] destination [none]" */
DEFUN_CMD_FUNC_DECL(log_5424_destination_none)
#define funcdecl_log_5424_destination_none static int log_5424_destination_none_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * no)
funcdecl_log_5424_destination_none;
DEFUN_CMD_FUNC_TEXT(log_5424_destination_none)
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

	return log_5424_destination_none_magic(self, vty, argc, argv, no);
}

/* log_5424_prio => "[no] priority <emergencies|alerts|critical|errors|warnings|notifications|informational|debugging>$levelarg" */
DEFUN_CMD_FUNC_DECL(log_5424_prio)
#define funcdecl_log_5424_prio static int log_5424_prio_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * no,\
	const char * levelarg)
funcdecl_log_5424_prio;
DEFUN_CMD_FUNC_TEXT(log_5424_prio)
{
#if 2 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *no = NULL;
	const char *levelarg = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "no")) {
			no = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "levelarg")) {
			levelarg = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!levelarg) {
		vty_out(vty, "Internal CLI error [%s]\n", "levelarg");
		return CMD_WARNING;
	}

	return log_5424_prio_magic(self, vty, argc, argv, no, levelarg);
}

/* log_5424_facility => "[no] facility <kern|user|mail|daemon|auth|syslog|lpr|news|uucp|cron|local0|local1|local2|local3|local4|local5|local6|local7>$facilityarg" */
DEFUN_CMD_FUNC_DECL(log_5424_facility)
#define funcdecl_log_5424_facility static int log_5424_facility_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * no,\
	const char * facilityarg)
funcdecl_log_5424_facility;
DEFUN_CMD_FUNC_TEXT(log_5424_facility)
{
#if 2 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *no = NULL;
	const char *facilityarg = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "no")) {
			no = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "facilityarg")) {
			facilityarg = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!facilityarg) {
		vty_out(vty, "Internal CLI error [%s]\n", "facilityarg");
		return CMD_WARNING;
	}

	return log_5424_facility_magic(self, vty, argc, argv, no, facilityarg);
}

/* log_5424_meta => "[no] structured-data <code-location|version|unique-id|error-category|format-args>$option" */
DEFUN_CMD_FUNC_DECL(log_5424_meta)
#define funcdecl_log_5424_meta static int log_5424_meta_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * no,\
	const char * option)
funcdecl_log_5424_meta;
DEFUN_CMD_FUNC_TEXT(log_5424_meta)
{
#if 2 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *no = NULL;
	const char *option = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "no")) {
			no = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "option")) {
			option = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!option) {
		vty_out(vty, "Internal CLI error [%s]\n", "option");
		return CMD_WARNING;
	}

	return log_5424_meta_magic(self, vty, argc, argv, no, option);
}

/* log_5424_ts_prec => "[no] timestamp precision (0-9)" */
DEFUN_CMD_FUNC_DECL(log_5424_ts_prec)
#define funcdecl_log_5424_ts_prec static int log_5424_ts_prec_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * no,\
	long precision,\
	const char * precision_str __attribute__ ((unused)))
funcdecl_log_5424_ts_prec;
DEFUN_CMD_FUNC_TEXT(log_5424_ts_prec)
{
#if 2 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *no = NULL;
	long precision = 0;
	const char *precision_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "no")) {
			no = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "precision")) {
			precision_str = argv[_i]->arg;
			char *_end;
			precision = strtol(argv[_i]->arg, &_end, 10);
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
	if (!precision_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "precision_str");
		return CMD_WARNING;
	}

	return log_5424_ts_prec_magic(self, vty, argc, argv, no, precision, precision_str);
}

/* log_5424_ts_local => "[no] timestamp local-time" */
DEFUN_CMD_FUNC_DECL(log_5424_ts_local)
#define funcdecl_log_5424_ts_local static int log_5424_ts_local_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * no)
funcdecl_log_5424_ts_local;
DEFUN_CMD_FUNC_TEXT(log_5424_ts_local)
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

	return log_5424_ts_local_magic(self, vty, argc, argv, no);
}

