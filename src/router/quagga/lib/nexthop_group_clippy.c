/* nexthop_group_backup => "backup-group WORD$name" */
DEFUN_CMD_FUNC_DECL(nexthop_group_backup)
#define funcdecl_nexthop_group_backup static int nexthop_group_backup_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name)
funcdecl_nexthop_group_backup;
DEFUN_CMD_FUNC_TEXT(nexthop_group_backup)
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

	return nexthop_group_backup_magic(self, vty, argc, argv, name);
}

/* no_nexthop_group_backup => "no backup-group [WORD$name]" */
DEFUN_CMD_FUNC_DECL(no_nexthop_group_backup)
#define funcdecl_no_nexthop_group_backup static int no_nexthop_group_backup_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name)
funcdecl_no_nexthop_group_backup;
DEFUN_CMD_FUNC_TEXT(no_nexthop_group_backup)
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

	return no_nexthop_group_backup_magic(self, vty, argc, argv, name);
}

/* nexthop_group_resilience => "resilient buckets (1-256) idle-timer (1-4294967295) unbalanced-timer (1-4294967295)" */
DEFUN_CMD_FUNC_DECL(nexthop_group_resilience)
#define funcdecl_nexthop_group_resilience static int nexthop_group_resilience_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long buckets,\
	const char * buckets_str __attribute__ ((unused)),\
	long idle_timer,\
	const char * idle_timer_str __attribute__ ((unused)),\
	long unbalanced_timer,\
	const char * unbalanced_timer_str __attribute__ ((unused)))
funcdecl_nexthop_group_resilience;
DEFUN_CMD_FUNC_TEXT(nexthop_group_resilience)
{
#if 3 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long buckets = 0;
	const char *buckets_str = NULL;
	long idle_timer = 0;
	const char *idle_timer_str = NULL;
	long unbalanced_timer = 0;
	const char *unbalanced_timer_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "buckets")) {
			buckets_str = argv[_i]->arg;
			char *_end;
			buckets = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "idle_timer")) {
			idle_timer_str = argv[_i]->arg;
			char *_end;
			idle_timer = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "unbalanced_timer")) {
			unbalanced_timer_str = argv[_i]->arg;
			char *_end;
			unbalanced_timer = strtol(argv[_i]->arg, &_end, 10);
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
	if (!buckets_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "buckets_str");
		return CMD_WARNING;
	}
	if (!idle_timer_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "idle_timer_str");
		return CMD_WARNING;
	}
	if (!unbalanced_timer_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "unbalanced_timer_str");
		return CMD_WARNING;
	}

	return nexthop_group_resilience_magic(self, vty, argc, argv, buckets, buckets_str, idle_timer, idle_timer_str, unbalanced_timer, unbalanced_timer_str);
}

/* no_nexthop_group_resilience => "no resilient [buckets (1-256) idle-timer (1-4294967295) unbalanced-timer (1-4294967295)]" */
DEFUN_CMD_FUNC_DECL(no_nexthop_group_resilience)
#define funcdecl_no_nexthop_group_resilience static int no_nexthop_group_resilience_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long buckets,\
	const char * buckets_str __attribute__ ((unused)),\
	long idle_timer,\
	const char * idle_timer_str __attribute__ ((unused)),\
	long unbalanced_timer,\
	const char * unbalanced_timer_str __attribute__ ((unused)))
funcdecl_no_nexthop_group_resilience;
DEFUN_CMD_FUNC_TEXT(no_nexthop_group_resilience)
{
#if 3 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long buckets = 0;
	const char *buckets_str = NULL;
	long idle_timer = 0;
	const char *idle_timer_str = NULL;
	long unbalanced_timer = 0;
	const char *unbalanced_timer_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "buckets")) {
			buckets_str = argv[_i]->arg;
			char *_end;
			buckets = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "idle_timer")) {
			idle_timer_str = argv[_i]->arg;
			char *_end;
			idle_timer = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "unbalanced_timer")) {
			unbalanced_timer_str = argv[_i]->arg;
			char *_end;
			unbalanced_timer = strtol(argv[_i]->arg, &_end, 10);
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

	return no_nexthop_group_resilience_magic(self, vty, argc, argv, buckets, buckets_str, idle_timer, idle_timer_str, unbalanced_timer, unbalanced_timer_str);
}

/* ecmp_nexthops => "[no] nexthop        <	  <A.B.C.D|X:X::X:X>$addr [INTERFACE$intf [onlink$onlink]]	  |INTERFACE$intf	>	[{ 	   nexthop-vrf NAME$vrf_name 	   |label WORD            |weight (1-255)            |backup-idx WORD 	}]" */
DEFUN_CMD_FUNC_DECL(ecmp_nexthops)
#define funcdecl_ecmp_nexthops static int ecmp_nexthops_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * no,\
	const union sockunion * addr,\
	const char * addr_str __attribute__ ((unused)),\
	const char * intf,\
	const char * onlink,\
	const char * vrf_name,\
	const char * label,\
	long weight,\
	const char * weight_str __attribute__ ((unused)),\
	const char * backup_idx)
funcdecl_ecmp_nexthops;
DEFUN_CMD_FUNC_TEXT(ecmp_nexthops)
{
#if 8 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *no = NULL;
	union sockunion s__addr = { .sa.sa_family = AF_UNSPEC }, *addr = NULL;
	const char *addr_str = NULL;
	const char *intf = NULL;
	const char *onlink = NULL;
	const char *vrf_name = NULL;
	const char *label = NULL;
	long weight = 0;
	const char *weight_str = NULL;
	const char *backup_idx = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "no")) {
			no = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "addr")) {
			addr_str = argv[_i]->arg;
			if (argv[_i]->text[0] == 'X') {
				s__addr.sa.sa_family = AF_INET6;
				_fail = !inet_pton(AF_INET6, argv[_i]->arg, &s__addr.sin6.sin6_addr);
				addr = &s__addr;
			} else {
				s__addr.sa.sa_family = AF_INET;
				_fail = !inet_aton(argv[_i]->arg, &s__addr.sin.sin_addr);
				addr = &s__addr;
			}
		}
		if (!strcmp(argv[_i]->varname, "intf")) {
			intf = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "onlink")) {
			onlink = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "vrf_name")) {
			vrf_name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "label")) {
			label = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "weight")) {
			weight_str = argv[_i]->arg;
			char *_end;
			weight = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "backup_idx")) {
			backup_idx = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return ecmp_nexthops_magic(self, vty, argc, argv, no, addr, addr_str, intf, onlink, vrf_name, label, weight, weight_str, backup_idx);
}

