/* config_commit => "commit [{force$force|confirmed (1-60)}]" */
DEFUN_CMD_FUNC_DECL(config_commit)
#define funcdecl_config_commit static int config_commit_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * force,\
	long confirmed,\
	const char * confirmed_str __attribute__ ((unused)))
funcdecl_config_commit;
DEFUN_CMD_FUNC_TEXT(config_commit)
{
#if 2 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *force = NULL;
	long confirmed = 0;
	const char *confirmed_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "force")) {
			force = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "confirmed")) {
			confirmed_str = argv[_i]->arg;
			char *_end;
			confirmed = strtol(argv[_i]->arg, &_end, 10);
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

	return config_commit_magic(self, vty, argc, argv, force, confirmed, confirmed_str);
}

/* config_commit_comment => "commit [{force$force|confirmed (1-60)}] comment LINE..." */
DEFUN_CMD_FUNC_DECL(config_commit_comment)
#define funcdecl_config_commit_comment static int config_commit_comment_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * force,\
	long confirmed,\
	const char * confirmed_str __attribute__ ((unused)),\
	const char * line)
funcdecl_config_commit_comment;
DEFUN_CMD_FUNC_TEXT(config_commit_comment)
{
#if 3 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *force = NULL;
	long confirmed = 0;
	const char *confirmed_str = NULL;
	const char *line = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "force")) {
			force = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "confirmed")) {
			confirmed_str = argv[_i]->arg;
			char *_end;
			confirmed = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "line")) {
			line = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!line) {
		vty_out(vty, "Internal CLI error [%s]\n", "line");
		return CMD_WARNING;
	}

	return config_commit_comment_magic(self, vty, argc, argv, force, confirmed, confirmed_str, line);
}

/* config_commit_check => "commit check" */
DEFUN_CMD_FUNC_DECL(config_commit_check)
#define funcdecl_config_commit_check static int config_commit_check_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)))
funcdecl_config_commit_check;
DEFUN_CMD_FUNC_TEXT(config_commit_check)
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

	return config_commit_check_magic(self, vty, argc, argv);
}

/* config_update => "update" */
DEFUN_CMD_FUNC_DECL(config_update)
#define funcdecl_config_update static int config_update_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)))
funcdecl_config_update;
DEFUN_CMD_FUNC_TEXT(config_update)
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

	return config_update_magic(self, vty, argc, argv);
}

/* config_discard => "discard" */
DEFUN_CMD_FUNC_DECL(config_discard)
#define funcdecl_config_discard static int config_discard_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)))
funcdecl_config_discard;
DEFUN_CMD_FUNC_TEXT(config_discard)
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

	return config_discard_magic(self, vty, argc, argv);
}

/* config_load => "configuration load          <	    file [<json$json|xml$xml> [translate WORD$translator_family]] FILENAME$filename	    |transaction (1-4294967295)$tid	  >	  [replace$replace]" */
DEFUN_CMD_FUNC_DECL(config_load)
#define funcdecl_config_load static int config_load_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * json,\
	const char * xml,\
	const char * translator_family,\
	const char * filename,\
	long tid,\
	const char * tid_str __attribute__ ((unused)),\
	const char * replace)
funcdecl_config_load;
DEFUN_CMD_FUNC_TEXT(config_load)
{
#if 6 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *json = NULL;
	const char *xml = NULL;
	const char *translator_family = NULL;
	const char *filename = NULL;
	long tid = 0;
	const char *tid_str = NULL;
	const char *replace = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "json")) {
			json = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "xml")) {
			xml = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "translator_family")) {
			translator_family = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "filename")) {
			filename = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "tid")) {
			tid_str = argv[_i]->arg;
			char *_end;
			tid = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "replace")) {
			replace = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return config_load_magic(self, vty, argc, argv, json, xml, translator_family, filename, tid, tid_str, replace);
}

/* show_config_running => "show configuration running          [<json$json|xml$xml> [translate WORD$translator_family]]	  [with-defaults$with_defaults]" */
DEFUN_CMD_FUNC_DECL(show_config_running)
#define funcdecl_show_config_running static int show_config_running_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * json,\
	const char * xml,\
	const char * translator_family,\
	const char * with_defaults)
funcdecl_show_config_running;
DEFUN_CMD_FUNC_TEXT(show_config_running)
{
#if 4 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *json = NULL;
	const char *xml = NULL;
	const char *translator_family = NULL;
	const char *with_defaults = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "json")) {
			json = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "xml")) {
			xml = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "translator_family")) {
			translator_family = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "with_defaults")) {
			with_defaults = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return show_config_running_magic(self, vty, argc, argv, json, xml, translator_family, with_defaults);
}

/* show_config_candidate => "show configuration candidate          [<json$json|xml$xml> [translate WORD$translator_family]]          [<	    with-defaults$with_defaults	    |changes$changes	   >]" */
DEFUN_CMD_FUNC_DECL(show_config_candidate)
#define funcdecl_show_config_candidate static int show_config_candidate_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * json,\
	const char * xml,\
	const char * translator_family,\
	const char * with_defaults,\
	const char * changes)
funcdecl_show_config_candidate;
DEFUN_CMD_FUNC_TEXT(show_config_candidate)
{
#if 5 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *json = NULL;
	const char *xml = NULL;
	const char *translator_family = NULL;
	const char *with_defaults = NULL;
	const char *changes = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "json")) {
			json = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "xml")) {
			xml = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "translator_family")) {
			translator_family = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "with_defaults")) {
			with_defaults = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "changes")) {
			changes = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return show_config_candidate_magic(self, vty, argc, argv, json, xml, translator_family, with_defaults, changes);
}

/* show_config_candidate_section => "show" */
DEFUN_CMD_FUNC_DECL(show_config_candidate_section)
#define funcdecl_show_config_candidate_section static int show_config_candidate_section_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)))
funcdecl_show_config_candidate_section;
DEFUN_CMD_FUNC_TEXT(show_config_candidate_section)
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

	return show_config_candidate_section_magic(self, vty, argc, argv);
}

/* show_config_compare => "show configuration compare          <	    candidate$c1_candidate	    |running$c1_running	    |transaction (1-4294967295)$c1_tid	  >          <	    candidate$c2_candidate	    |running$c2_running	    |transaction (1-4294967295)$c2_tid	  >	  [<json$json|xml$xml> [translate WORD$translator_family]]" */
DEFUN_CMD_FUNC_DECL(show_config_compare)
#define funcdecl_show_config_compare static int show_config_compare_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * c1_candidate,\
	const char * c1_running,\
	long c1_tid,\
	const char * c1_tid_str __attribute__ ((unused)),\
	const char * c2_candidate,\
	const char * c2_running,\
	long c2_tid,\
	const char * c2_tid_str __attribute__ ((unused)),\
	const char * json,\
	const char * xml,\
	const char * translator_family)
funcdecl_show_config_compare;
DEFUN_CMD_FUNC_TEXT(show_config_compare)
{
#if 9 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *c1_candidate = NULL;
	const char *c1_running = NULL;
	long c1_tid = 0;
	const char *c1_tid_str = NULL;
	const char *c2_candidate = NULL;
	const char *c2_running = NULL;
	long c2_tid = 0;
	const char *c2_tid_str = NULL;
	const char *json = NULL;
	const char *xml = NULL;
	const char *translator_family = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "c1_candidate")) {
			c1_candidate = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "c1_running")) {
			c1_running = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "c1_tid")) {
			c1_tid_str = argv[_i]->arg;
			char *_end;
			c1_tid = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "c2_candidate")) {
			c2_candidate = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "c2_running")) {
			c2_running = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "c2_tid")) {
			c2_tid_str = argv[_i]->arg;
			char *_end;
			c2_tid = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "json")) {
			json = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "xml")) {
			xml = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "translator_family")) {
			translator_family = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return show_config_compare_magic(self, vty, argc, argv, c1_candidate, c1_running, c1_tid, c1_tid_str, c2_candidate, c2_running, c2_tid, c2_tid_str, json, xml, translator_family);
}

/* clear_config_transactions => "clear configuration transactions oldest (1-100)$n" */
DEFUN_CMD_FUNC_DECL(clear_config_transactions)
#define funcdecl_clear_config_transactions static int clear_config_transactions_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long n,\
	const char * n_str __attribute__ ((unused)))
funcdecl_clear_config_transactions;
DEFUN_CMD_FUNC_TEXT(clear_config_transactions)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long n = 0;
	const char *n_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "n")) {
			n_str = argv[_i]->arg;
			char *_end;
			n = strtol(argv[_i]->arg, &_end, 10);
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
	if (!n_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "n_str");
		return CMD_WARNING;
	}

	return clear_config_transactions_magic(self, vty, argc, argv, n, n_str);
}

/* config_database_max_transactions => "configuration database max-transactions (1-100)$max" */
DEFUN_CMD_FUNC_DECL(config_database_max_transactions)
#define funcdecl_config_database_max_transactions static int config_database_max_transactions_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long max,\
	const char * max_str __attribute__ ((unused)))
funcdecl_config_database_max_transactions;
DEFUN_CMD_FUNC_TEXT(config_database_max_transactions)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long max = 0;
	const char *max_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "max")) {
			max_str = argv[_i]->arg;
			char *_end;
			max = strtol(argv[_i]->arg, &_end, 10);
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
	if (!max_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "max_str");
		return CMD_WARNING;
	}

	return config_database_max_transactions_magic(self, vty, argc, argv, max, max_str);
}

/* yang_module_translator_load => "yang module-translator load FILENAME$filename" */
DEFUN_CMD_FUNC_DECL(yang_module_translator_load)
#define funcdecl_yang_module_translator_load static int yang_module_translator_load_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * filename)
funcdecl_yang_module_translator_load;
DEFUN_CMD_FUNC_TEXT(yang_module_translator_load)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *filename = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "filename")) {
			filename = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!filename) {
		vty_out(vty, "Internal CLI error [%s]\n", "filename");
		return CMD_WARNING;
	}

	return yang_module_translator_load_magic(self, vty, argc, argv, filename);
}

/* yang_module_translator_unload_family => "yang module-translator unload WORD$translator_family" */
DEFUN_CMD_FUNC_DECL(yang_module_translator_unload_family)
#define funcdecl_yang_module_translator_unload_family static int yang_module_translator_unload_family_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * translator_family)
funcdecl_yang_module_translator_unload_family;
DEFUN_CMD_FUNC_TEXT(yang_module_translator_unload_family)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *translator_family = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "translator_family")) {
			translator_family = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!translator_family) {
		vty_out(vty, "Internal CLI error [%s]\n", "translator_family");
		return CMD_WARNING;
	}

	return yang_module_translator_unload_family_magic(self, vty, argc, argv, translator_family);
}

/* show_config_transaction => "show configuration transaction          [	    (1-4294967295)$transaction_id	    [<json$json|xml$xml> [translate WORD$translator_family]]            [<	      with-defaults$with_defaults	      |changes$changes	     >]	  ]" */
DEFUN_CMD_FUNC_DECL(show_config_transaction)
#define funcdecl_show_config_transaction static int show_config_transaction_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long transaction_id,\
	const char * transaction_id_str __attribute__ ((unused)),\
	const char * json,\
	const char * xml,\
	const char * translator_family,\
	const char * with_defaults,\
	const char * changes)
funcdecl_show_config_transaction;
DEFUN_CMD_FUNC_TEXT(show_config_transaction)
{
#if 6 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long transaction_id = 0;
	const char *transaction_id_str = NULL;
	const char *json = NULL;
	const char *xml = NULL;
	const char *translator_family = NULL;
	const char *with_defaults = NULL;
	const char *changes = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "transaction_id")) {
			transaction_id_str = argv[_i]->arg;
			char *_end;
			transaction_id = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "json")) {
			json = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "xml")) {
			xml = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "translator_family")) {
			translator_family = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "with_defaults")) {
			with_defaults = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "changes")) {
			changes = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return show_config_transaction_magic(self, vty, argc, argv, transaction_id, transaction_id_str, json, xml, translator_family, with_defaults, changes);
}

/* show_yang_operational_data => "show yang operational-data XPATH$xpath         [{	   format <json$json|xml$xml>	   |translate WORD$translator_family	   |with-config$with_config	 }]" */
DEFUN_CMD_FUNC_DECL(show_yang_operational_data)
#define funcdecl_show_yang_operational_data static int show_yang_operational_data_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * xpath,\
	const char * json,\
	const char * xml,\
	const char * translator_family,\
	const char * with_config)
funcdecl_show_yang_operational_data;
DEFUN_CMD_FUNC_TEXT(show_yang_operational_data)
{
#if 5 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *xpath = NULL;
	const char *json = NULL;
	const char *xml = NULL;
	const char *translator_family = NULL;
	const char *with_config = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "xpath")) {
			xpath = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "json")) {
			json = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "xml")) {
			xml = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "translator_family")) {
			translator_family = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "with_config")) {
			with_config = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!xpath) {
		vty_out(vty, "Internal CLI error [%s]\n", "xpath");
		return CMD_WARNING;
	}

	return show_yang_operational_data_magic(self, vty, argc, argv, xpath, json, xml, translator_family, with_config);
}

/* show_yang_module => "show yang module [module-translator WORD$translator_family]" */
DEFUN_CMD_FUNC_DECL(show_yang_module)
#define funcdecl_show_yang_module static int show_yang_module_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * translator_family)
funcdecl_show_yang_module;
DEFUN_CMD_FUNC_TEXT(show_yang_module)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *translator_family = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "translator_family")) {
			translator_family = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return show_yang_module_magic(self, vty, argc, argv, translator_family);
}

/* show_yang_module_detail => "show yang module          [module-translator WORD$translator_family]          WORD$module_name <compiled$compiled|summary|tree$tree|yang$yang|yin$yin>" */
DEFUN_CMD_FUNC_DECL(show_yang_module_detail)
#define funcdecl_show_yang_module_detail static int show_yang_module_detail_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * translator_family,\
	const char * module_name,\
	const char * compiled,\
	const char * tree,\
	const char * yang,\
	const char * yin)
funcdecl_show_yang_module_detail;
DEFUN_CMD_FUNC_TEXT(show_yang_module_detail)
{
#if 6 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *translator_family = NULL;
	const char *module_name = NULL;
	const char *compiled = NULL;
	const char *tree = NULL;
	const char *yang = NULL;
	const char *yin = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "translator_family")) {
			translator_family = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "module_name")) {
			module_name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "compiled")) {
			compiled = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "tree")) {
			tree = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "yang")) {
			yang = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "yin")) {
			yin = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!module_name) {
		vty_out(vty, "Internal CLI error [%s]\n", "module_name");
		return CMD_WARNING;
	}

	return show_yang_module_detail_magic(self, vty, argc, argv, translator_family, module_name, compiled, tree, yang, yin);
}

/* show_yang_module_translator => "show yang module-translator" */
DEFUN_CMD_FUNC_DECL(show_yang_module_translator)
#define funcdecl_show_yang_module_translator static int show_yang_module_translator_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)))
funcdecl_show_yang_module_translator;
DEFUN_CMD_FUNC_TEXT(show_yang_module_translator)
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

	return show_yang_module_translator_magic(self, vty, argc, argv);
}

/* rollback_config => "rollback configuration (1-4294967295)$transaction_id" */
DEFUN_CMD_FUNC_DECL(rollback_config)
#define funcdecl_rollback_config static int rollback_config_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long transaction_id,\
	const char * transaction_id_str __attribute__ ((unused)))
funcdecl_rollback_config;
DEFUN_CMD_FUNC_TEXT(rollback_config)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long transaction_id = 0;
	const char *transaction_id_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "transaction_id")) {
			transaction_id_str = argv[_i]->arg;
			char *_end;
			transaction_id = strtol(argv[_i]->arg, &_end, 10);
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
	if (!transaction_id_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "transaction_id_str");
		return CMD_WARNING;
	}

	return rollback_config_magic(self, vty, argc, argv, transaction_id, transaction_id_str);
}

/* debug_nb => "[no] debug northbound          [<	    callbacks$cbs [{configuration$cbs_cfg|state$cbs_state|rpc$cbs_rpc}]	    |notifications$notifications	    |events$events	    |libyang$libyang          >]" */
DEFUN_CMD_FUNC_DECL(debug_nb)
#define funcdecl_debug_nb static int debug_nb_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * no,\
	const char * cbs,\
	const char * cbs_cfg,\
	const char * cbs_state,\
	const char * cbs_rpc,\
	const char * notifications,\
	const char * events,\
	const char * libyang)
funcdecl_debug_nb;
DEFUN_CMD_FUNC_TEXT(debug_nb)
{
#if 8 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *no = NULL;
	const char *cbs = NULL;
	const char *cbs_cfg = NULL;
	const char *cbs_state = NULL;
	const char *cbs_rpc = NULL;
	const char *notifications = NULL;
	const char *events = NULL;
	const char *libyang = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "no")) {
			no = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "cbs")) {
			cbs = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "cbs_cfg")) {
			cbs_cfg = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "cbs_state")) {
			cbs_state = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "cbs_rpc")) {
			cbs_rpc = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "notifications")) {
			notifications = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "events")) {
			events = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "libyang")) {
			libyang = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return debug_nb_magic(self, vty, argc, argv, no, cbs, cbs_cfg, cbs_state, cbs_rpc, notifications, events, libyang);
}

