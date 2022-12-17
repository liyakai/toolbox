/* route_map => "route-map RMAP_NAME$name <deny|permit>$action (1-65535)$sequence" */
DEFUN_CMD_FUNC_DECL(route_map)
#define funcdecl_route_map static int route_map_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	const char * action,\
	long sequence,\
	const char * sequence_str __attribute__ ((unused)))
funcdecl_route_map;
DEFUN_CMD_FUNC_TEXT(route_map)
{
#if 3 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	const char *action = NULL;
	long sequence = 0;
	const char *sequence_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "action")) {
			action = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "sequence")) {
			sequence_str = argv[_i]->arg;
			char *_end;
			sequence = strtol(argv[_i]->arg, &_end, 10);
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
	if (!sequence_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "sequence_str");
		return CMD_WARNING;
	}

	return route_map_magic(self, vty, argc, argv, name, action, sequence, sequence_str);
}

/* no_route_map_all => "no route-map RMAP_NAME$name" */
DEFUN_CMD_FUNC_DECL(no_route_map_all)
#define funcdecl_no_route_map_all static int no_route_map_all_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name)
funcdecl_no_route_map_all;
DEFUN_CMD_FUNC_TEXT(no_route_map_all)
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

	return no_route_map_all_magic(self, vty, argc, argv, name);
}

/* no_route_map => "no route-map RMAP_NAME$name <deny|permit>$action (1-65535)$sequence" */
DEFUN_CMD_FUNC_DECL(no_route_map)
#define funcdecl_no_route_map static int no_route_map_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name,\
	const char * action,\
	long sequence,\
	const char * sequence_str __attribute__ ((unused)))
funcdecl_no_route_map;
DEFUN_CMD_FUNC_TEXT(no_route_map)
{
#if 3 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *name = NULL;
	const char *action = NULL;
	long sequence = 0;
	const char *sequence_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "name")) {
			name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "action")) {
			action = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "sequence")) {
			sequence_str = argv[_i]->arg;
			char *_end;
			sequence = strtol(argv[_i]->arg, &_end, 10);
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
	if (!sequence_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "sequence_str");
		return CMD_WARNING;
	}

	return no_route_map_magic(self, vty, argc, argv, name, action, sequence, sequence_str);
}

/* match_interface => "match interface IFNAME" */
DEFUN_CMD_FUNC_DECL(match_interface)
#define funcdecl_match_interface static int match_interface_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * ifname)
funcdecl_match_interface;
DEFUN_CMD_FUNC_TEXT(match_interface)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *ifname = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "ifname")) {
			ifname = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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
	if (!ifname) {
		vty_out(vty, "Internal CLI error [%s]\n", "ifname");
		return CMD_WARNING;
	}

	return match_interface_magic(self, vty, argc, argv, ifname);
}

/* no_match_interface => "no match interface [IFNAME]" */
DEFUN_CMD_FUNC_DECL(no_match_interface)
#define funcdecl_no_match_interface static int no_match_interface_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * ifname)
funcdecl_no_match_interface;
DEFUN_CMD_FUNC_TEXT(no_match_interface)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *ifname = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "ifname")) {
			ifname = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return no_match_interface_magic(self, vty, argc, argv, ifname);
}

/* match_ip_address => "match ip address ACCESSLIST4_NAME$name" */
DEFUN_CMD_FUNC_DECL(match_ip_address)
#define funcdecl_match_ip_address static int match_ip_address_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name)
funcdecl_match_ip_address;
DEFUN_CMD_FUNC_TEXT(match_ip_address)
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

	return match_ip_address_magic(self, vty, argc, argv, name);
}

/* no_match_ip_address => "no match ip address [ACCESSLIST4_NAME]" */
DEFUN_CMD_FUNC_DECL(no_match_ip_address)
#define funcdecl_no_match_ip_address static int no_match_ip_address_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * accesslist4_name)
funcdecl_no_match_ip_address;
DEFUN_CMD_FUNC_TEXT(no_match_ip_address)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *accesslist4_name = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "accesslist4_name")) {
			accesslist4_name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return no_match_ip_address_magic(self, vty, argc, argv, accesslist4_name);
}

/* match_ip_address_prefix_list => "match ip address prefix-list PREFIXLIST_NAME$name" */
DEFUN_CMD_FUNC_DECL(match_ip_address_prefix_list)
#define funcdecl_match_ip_address_prefix_list static int match_ip_address_prefix_list_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name)
funcdecl_match_ip_address_prefix_list;
DEFUN_CMD_FUNC_TEXT(match_ip_address_prefix_list)
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

	return match_ip_address_prefix_list_magic(self, vty, argc, argv, name);
}

/* no_match_ip_address_prefix_list => "no match ip address prefix-list [PREFIXLIST_NAME]" */
DEFUN_CMD_FUNC_DECL(no_match_ip_address_prefix_list)
#define funcdecl_no_match_ip_address_prefix_list static int no_match_ip_address_prefix_list_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * prefixlist_name)
funcdecl_no_match_ip_address_prefix_list;
DEFUN_CMD_FUNC_TEXT(no_match_ip_address_prefix_list)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *prefixlist_name = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "prefixlist_name")) {
			prefixlist_name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return no_match_ip_address_prefix_list_magic(self, vty, argc, argv, prefixlist_name);
}

/* match_ip_next_hop => "match ip next-hop ACCESSLIST4_NAME$name" */
DEFUN_CMD_FUNC_DECL(match_ip_next_hop)
#define funcdecl_match_ip_next_hop static int match_ip_next_hop_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name)
funcdecl_match_ip_next_hop;
DEFUN_CMD_FUNC_TEXT(match_ip_next_hop)
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

	return match_ip_next_hop_magic(self, vty, argc, argv, name);
}

/* no_match_ip_next_hop => "no match ip next-hop [ACCESSLIST4_NAME]" */
DEFUN_CMD_FUNC_DECL(no_match_ip_next_hop)
#define funcdecl_no_match_ip_next_hop static int no_match_ip_next_hop_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * accesslist4_name)
funcdecl_no_match_ip_next_hop;
DEFUN_CMD_FUNC_TEXT(no_match_ip_next_hop)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *accesslist4_name = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "accesslist4_name")) {
			accesslist4_name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return no_match_ip_next_hop_magic(self, vty, argc, argv, accesslist4_name);
}

/* match_ip_next_hop_prefix_list => "match ip next-hop prefix-list PREFIXLIST_NAME$name" */
DEFUN_CMD_FUNC_DECL(match_ip_next_hop_prefix_list)
#define funcdecl_match_ip_next_hop_prefix_list static int match_ip_next_hop_prefix_list_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name)
funcdecl_match_ip_next_hop_prefix_list;
DEFUN_CMD_FUNC_TEXT(match_ip_next_hop_prefix_list)
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

	return match_ip_next_hop_prefix_list_magic(self, vty, argc, argv, name);
}

/* no_match_ip_next_hop_prefix_list => "no match ip next-hop prefix-list [PREFIXLIST_NAME]" */
DEFUN_CMD_FUNC_DECL(no_match_ip_next_hop_prefix_list)
#define funcdecl_no_match_ip_next_hop_prefix_list static int no_match_ip_next_hop_prefix_list_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * prefixlist_name)
funcdecl_no_match_ip_next_hop_prefix_list;
DEFUN_CMD_FUNC_TEXT(no_match_ip_next_hop_prefix_list)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *prefixlist_name = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "prefixlist_name")) {
			prefixlist_name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return no_match_ip_next_hop_prefix_list_magic(self, vty, argc, argv, prefixlist_name);
}

/* match_ip_next_hop_type => "match ip next-hop type <blackhole>$type" */
DEFUN_CMD_FUNC_DECL(match_ip_next_hop_type)
#define funcdecl_match_ip_next_hop_type static int match_ip_next_hop_type_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * type)
funcdecl_match_ip_next_hop_type;
DEFUN_CMD_FUNC_TEXT(match_ip_next_hop_type)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *type = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "type")) {
			type = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return match_ip_next_hop_type_magic(self, vty, argc, argv, type);
}

/* no_match_ip_next_hop_type => "no match ip next-hop type [<blackhole>]" */
DEFUN_CMD_FUNC_DECL(no_match_ip_next_hop_type)
#define funcdecl_no_match_ip_next_hop_type static int no_match_ip_next_hop_type_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)))
funcdecl_no_match_ip_next_hop_type;
DEFUN_CMD_FUNC_TEXT(no_match_ip_next_hop_type)
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

	return no_match_ip_next_hop_type_magic(self, vty, argc, argv);
}

/* match_ipv6_address => "match ipv6 address ACCESSLIST6_NAME$name" */
DEFUN_CMD_FUNC_DECL(match_ipv6_address)
#define funcdecl_match_ipv6_address static int match_ipv6_address_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name)
funcdecl_match_ipv6_address;
DEFUN_CMD_FUNC_TEXT(match_ipv6_address)
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

	return match_ipv6_address_magic(self, vty, argc, argv, name);
}

/* no_match_ipv6_address => "no match ipv6 address [ACCESSLIST6_NAME]" */
DEFUN_CMD_FUNC_DECL(no_match_ipv6_address)
#define funcdecl_no_match_ipv6_address static int no_match_ipv6_address_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * accesslist6_name)
funcdecl_no_match_ipv6_address;
DEFUN_CMD_FUNC_TEXT(no_match_ipv6_address)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *accesslist6_name = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "accesslist6_name")) {
			accesslist6_name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return no_match_ipv6_address_magic(self, vty, argc, argv, accesslist6_name);
}

/* match_ipv6_address_prefix_list => "match ipv6 address prefix-list PREFIXLIST_NAME$name" */
DEFUN_CMD_FUNC_DECL(match_ipv6_address_prefix_list)
#define funcdecl_match_ipv6_address_prefix_list static int match_ipv6_address_prefix_list_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name)
funcdecl_match_ipv6_address_prefix_list;
DEFUN_CMD_FUNC_TEXT(match_ipv6_address_prefix_list)
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

	return match_ipv6_address_prefix_list_magic(self, vty, argc, argv, name);
}

/* no_match_ipv6_address_prefix_list => "no match ipv6 address prefix-list [PREFIXLIST_NAME]" */
DEFUN_CMD_FUNC_DECL(no_match_ipv6_address_prefix_list)
#define funcdecl_no_match_ipv6_address_prefix_list static int no_match_ipv6_address_prefix_list_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * prefixlist_name)
funcdecl_no_match_ipv6_address_prefix_list;
DEFUN_CMD_FUNC_TEXT(no_match_ipv6_address_prefix_list)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *prefixlist_name = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "prefixlist_name")) {
			prefixlist_name = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return no_match_ipv6_address_prefix_list_magic(self, vty, argc, argv, prefixlist_name);
}

/* match_ipv6_next_hop_type => "match ipv6 next-hop type <blackhole>$type" */
DEFUN_CMD_FUNC_DECL(match_ipv6_next_hop_type)
#define funcdecl_match_ipv6_next_hop_type static int match_ipv6_next_hop_type_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * type)
funcdecl_match_ipv6_next_hop_type;
DEFUN_CMD_FUNC_TEXT(match_ipv6_next_hop_type)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *type = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "type")) {
			type = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return match_ipv6_next_hop_type_magic(self, vty, argc, argv, type);
}

/* no_match_ipv6_next_hop_type => "no match ipv6 next-hop type [<blackhole>]" */
DEFUN_CMD_FUNC_DECL(no_match_ipv6_next_hop_type)
#define funcdecl_no_match_ipv6_next_hop_type static int no_match_ipv6_next_hop_type_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)))
funcdecl_no_match_ipv6_next_hop_type;
DEFUN_CMD_FUNC_TEXT(no_match_ipv6_next_hop_type)
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

	return no_match_ipv6_next_hop_type_magic(self, vty, argc, argv);
}

/* match_metric => "match metric (0-4294967295)$metric" */
DEFUN_CMD_FUNC_DECL(match_metric)
#define funcdecl_match_metric static int match_metric_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long metric,\
	const char * metric_str __attribute__ ((unused)))
funcdecl_match_metric;
DEFUN_CMD_FUNC_TEXT(match_metric)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long metric = 0;
	const char *metric_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "metric")) {
			metric_str = argv[_i]->arg;
			char *_end;
			metric = strtol(argv[_i]->arg, &_end, 10);
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
	if (!metric_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "metric_str");
		return CMD_WARNING;
	}

	return match_metric_magic(self, vty, argc, argv, metric, metric_str);
}

/* no_match_metric => "no match metric [(0-4294967295)]" */
DEFUN_CMD_FUNC_DECL(no_match_metric)
#define funcdecl_no_match_metric static int no_match_metric_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long metric,\
	const char * metric_str __attribute__ ((unused)))
funcdecl_no_match_metric;
DEFUN_CMD_FUNC_TEXT(no_match_metric)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long metric = 0;
	const char *metric_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "metric")) {
			metric_str = argv[_i]->arg;
			char *_end;
			metric = strtol(argv[_i]->arg, &_end, 10);
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

	return no_match_metric_magic(self, vty, argc, argv, metric, metric_str);
}

/* match_tag => "match tag (1-4294967295)$tag" */
DEFUN_CMD_FUNC_DECL(match_tag)
#define funcdecl_match_tag static int match_tag_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long tag,\
	const char * tag_str __attribute__ ((unused)))
funcdecl_match_tag;
DEFUN_CMD_FUNC_TEXT(match_tag)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long tag = 0;
	const char *tag_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "tag")) {
			tag_str = argv[_i]->arg;
			char *_end;
			tag = strtol(argv[_i]->arg, &_end, 10);
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
	if (!tag_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "tag_str");
		return CMD_WARNING;
	}

	return match_tag_magic(self, vty, argc, argv, tag, tag_str);
}

/* no_match_tag => "no match tag [(1-4294967295)]" */
DEFUN_CMD_FUNC_DECL(no_match_tag)
#define funcdecl_no_match_tag static int no_match_tag_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long tag,\
	const char * tag_str __attribute__ ((unused)))
funcdecl_no_match_tag;
DEFUN_CMD_FUNC_TEXT(no_match_tag)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long tag = 0;
	const char *tag_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "tag")) {
			tag_str = argv[_i]->arg;
			char *_end;
			tag = strtol(argv[_i]->arg, &_end, 10);
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

	return no_match_tag_magic(self, vty, argc, argv, tag, tag_str);
}

/* set_ip_nexthop => "set ip next-hop A.B.C.D$addr" */
DEFUN_CMD_FUNC_DECL(set_ip_nexthop)
#define funcdecl_set_ip_nexthop static int set_ip_nexthop_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	struct in_addr addr,\
	const char * addr_str __attribute__ ((unused)))
funcdecl_set_ip_nexthop;
DEFUN_CMD_FUNC_TEXT(set_ip_nexthop)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	struct in_addr addr = { INADDR_ANY };
	const char *addr_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "addr")) {
			addr_str = argv[_i]->arg;
			_fail = !inet_aton(argv[_i]->arg, &addr);
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
	if (!addr_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "addr_str");
		return CMD_WARNING;
	}

	return set_ip_nexthop_magic(self, vty, argc, argv, addr, addr_str);
}

/* no_set_ip_nexthop => "no set ip next-hop [A.B.C.D]" */
DEFUN_CMD_FUNC_DECL(no_set_ip_nexthop)
#define funcdecl_no_set_ip_nexthop static int no_set_ip_nexthop_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	struct in_addr next_hop,\
	const char * next_hop_str __attribute__ ((unused)))
funcdecl_no_set_ip_nexthop;
DEFUN_CMD_FUNC_TEXT(no_set_ip_nexthop)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	struct in_addr next_hop = { INADDR_ANY };
	const char *next_hop_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "next_hop")) {
			next_hop_str = argv[_i]->arg;
			_fail = !inet_aton(argv[_i]->arg, &next_hop);
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

	return no_set_ip_nexthop_magic(self, vty, argc, argv, next_hop, next_hop_str);
}

/* set_ipv6_nexthop_local => "set ipv6 next-hop local X:X::X:X$addr" */
DEFUN_CMD_FUNC_DECL(set_ipv6_nexthop_local)
#define funcdecl_set_ipv6_nexthop_local static int set_ipv6_nexthop_local_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	struct in6_addr addr,\
	const char * addr_str __attribute__ ((unused)))
funcdecl_set_ipv6_nexthop_local;
DEFUN_CMD_FUNC_TEXT(set_ipv6_nexthop_local)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	struct in6_addr addr = {};
	const char *addr_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "addr")) {
			addr_str = argv[_i]->arg;
			_fail = !inet_pton(AF_INET6, argv[_i]->arg, &addr);
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
	if (!addr_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "addr_str");
		return CMD_WARNING;
	}

	return set_ipv6_nexthop_local_magic(self, vty, argc, argv, addr, addr_str);
}

/* no_set_ipv6_nexthop_local => "no set ipv6 next-hop local [X:X::X:X]" */
DEFUN_CMD_FUNC_DECL(no_set_ipv6_nexthop_local)
#define funcdecl_no_set_ipv6_nexthop_local static int no_set_ipv6_nexthop_local_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	struct in6_addr local,\
	const char * local_str __attribute__ ((unused)))
funcdecl_no_set_ipv6_nexthop_local;
DEFUN_CMD_FUNC_TEXT(no_set_ipv6_nexthop_local)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	struct in6_addr local = {};
	const char *local_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "local")) {
			local_str = argv[_i]->arg;
			_fail = !inet_pton(AF_INET6, argv[_i]->arg, &local);
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

	return no_set_ipv6_nexthop_local_magic(self, vty, argc, argv, local, local_str);
}

/* set_metric => "set metric <(-4294967295-4294967295)$metric|rtt$rtt|+rtt$artt|-rtt$srtt>" */
DEFUN_CMD_FUNC_DECL(set_metric)
#define funcdecl_set_metric static int set_metric_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long metric,\
	const char * metric_str __attribute__ ((unused)),\
	const char * rtt,\
	const char * artt,\
	const char * srtt)
funcdecl_set_metric;
DEFUN_CMD_FUNC_TEXT(set_metric)
{
#if 4 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long metric = 0;
	const char *metric_str = NULL;
	const char *rtt = NULL;
	const char *artt = NULL;
	const char *srtt = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "metric")) {
			metric_str = argv[_i]->arg;
			char *_end;
			metric = strtol(argv[_i]->arg, &_end, 10);
			_fail = (_end == argv[_i]->arg) || (*_end != '\0');
		}
		if (!strcmp(argv[_i]->varname, "rtt")) {
			rtt = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "artt")) {
			artt = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
		if (!strcmp(argv[_i]->varname, "srtt")) {
			srtt = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return set_metric_magic(self, vty, argc, argv, metric, metric_str, rtt, artt, srtt);
}

/* no_set_metric => "no set metric [OPTVAL]" */
DEFUN_CMD_FUNC_DECL(no_set_metric)
#define funcdecl_no_set_metric static int no_set_metric_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * optval)
funcdecl_no_set_metric;
DEFUN_CMD_FUNC_TEXT(no_set_metric)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *optval = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "optval")) {
			optval = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return no_set_metric_magic(self, vty, argc, argv, optval);
}

/* set_tag => "set tag (1-4294967295)$tag" */
DEFUN_CMD_FUNC_DECL(set_tag)
#define funcdecl_set_tag static int set_tag_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long tag,\
	const char * tag_str __attribute__ ((unused)))
funcdecl_set_tag;
DEFUN_CMD_FUNC_TEXT(set_tag)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long tag = 0;
	const char *tag_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "tag")) {
			tag_str = argv[_i]->arg;
			char *_end;
			tag = strtol(argv[_i]->arg, &_end, 10);
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
	if (!tag_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "tag_str");
		return CMD_WARNING;
	}

	return set_tag_magic(self, vty, argc, argv, tag, tag_str);
}

/* no_set_tag => "no set tag [(1-4294967295)]" */
DEFUN_CMD_FUNC_DECL(no_set_tag)
#define funcdecl_no_set_tag static int no_set_tag_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long tag,\
	const char * tag_str __attribute__ ((unused)))
funcdecl_no_set_tag;
DEFUN_CMD_FUNC_TEXT(no_set_tag)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long tag = 0;
	const char *tag_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "tag")) {
			tag_str = argv[_i]->arg;
			char *_end;
			tag = strtol(argv[_i]->arg, &_end, 10);
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

	return no_set_tag_magic(self, vty, argc, argv, tag, tag_str);
}

/* rmap_onmatch_next => "on-match next" */
DEFUN_CMD_FUNC_DECL(rmap_onmatch_next)
#define funcdecl_rmap_onmatch_next static int rmap_onmatch_next_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)))
funcdecl_rmap_onmatch_next;
DEFUN_CMD_FUNC_TEXT(rmap_onmatch_next)
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

	return rmap_onmatch_next_magic(self, vty, argc, argv);
}

/* no_rmap_onmatch_next => "no on-match next" */
DEFUN_CMD_FUNC_DECL(no_rmap_onmatch_next)
#define funcdecl_no_rmap_onmatch_next static int no_rmap_onmatch_next_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)))
funcdecl_no_rmap_onmatch_next;
DEFUN_CMD_FUNC_TEXT(no_rmap_onmatch_next)
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

	return no_rmap_onmatch_next_magic(self, vty, argc, argv);
}

/* rmap_onmatch_goto => "on-match goto (1-65535)$rm_num" */
DEFUN_CMD_FUNC_DECL(rmap_onmatch_goto)
#define funcdecl_rmap_onmatch_goto static int rmap_onmatch_goto_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	long rm_num,\
	const char * rm_num_str __attribute__ ((unused)))
funcdecl_rmap_onmatch_goto;
DEFUN_CMD_FUNC_TEXT(rmap_onmatch_goto)
{
#if 1 /* anything to parse? */
	int _i;
#if 1 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	long rm_num = 0;
	const char *rm_num_str = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 1 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "rm_num")) {
			rm_num_str = argv[_i]->arg;
			char *_end;
			rm_num = strtol(argv[_i]->arg, &_end, 10);
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
	if (!rm_num_str) {
		vty_out(vty, "Internal CLI error [%s]\n", "rm_num_str");
		return CMD_WARNING;
	}

	return rmap_onmatch_goto_magic(self, vty, argc, argv, rm_num, rm_num_str);
}

/* no_rmap_onmatch_goto => "no on-match goto" */
DEFUN_CMD_FUNC_DECL(no_rmap_onmatch_goto)
#define funcdecl_no_rmap_onmatch_goto static int no_rmap_onmatch_goto_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)))
funcdecl_no_rmap_onmatch_goto;
DEFUN_CMD_FUNC_TEXT(no_rmap_onmatch_goto)
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

	return no_rmap_onmatch_goto_magic(self, vty, argc, argv);
}

/* rmap_call => "call WORD$name" */
DEFUN_CMD_FUNC_DECL(rmap_call)
#define funcdecl_rmap_call static int rmap_call_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * name)
funcdecl_rmap_call;
DEFUN_CMD_FUNC_TEXT(rmap_call)
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

	return rmap_call_magic(self, vty, argc, argv, name);
}

/* no_rmap_call => "no call [NAME]" */
DEFUN_CMD_FUNC_DECL(no_rmap_call)
#define funcdecl_no_rmap_call static int no_rmap_call_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * call)
funcdecl_no_rmap_call;
DEFUN_CMD_FUNC_TEXT(no_rmap_call)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *call = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "call")) {
			call = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
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

	return no_rmap_call_magic(self, vty, argc, argv, call);
}

/* rmap_description => "description LINE..." */
DEFUN_CMD_FUNC_DECL(rmap_description)
#define funcdecl_rmap_description static int rmap_description_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * line)
funcdecl_rmap_description;
DEFUN_CMD_FUNC_TEXT(rmap_description)
{
#if 1 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *line = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

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
	if (!line) {
		vty_out(vty, "Internal CLI error [%s]\n", "line");
		return CMD_WARNING;
	}

	return rmap_description_magic(self, vty, argc, argv, line);
}

/* route_map_optimization => "[no] route-map RMAP_NAME$name optimization" */
DEFUN_CMD_FUNC_DECL(route_map_optimization)
#define funcdecl_route_map_optimization static int route_map_optimization_magic(\
	const struct cmd_element *self __attribute__ ((unused)),\
	struct vty *vty __attribute__ ((unused)),\
	int argc __attribute__ ((unused)),\
	struct cmd_token *argv[] __attribute__ ((unused)),\
	const char * no,\
	const char * name)
funcdecl_route_map_optimization;
DEFUN_CMD_FUNC_TEXT(route_map_optimization)
{
#if 2 /* anything to parse? */
	int _i;
#if 0 /* anything that can fail? */
	unsigned _fail = 0, _failcnt = 0;
#endif
	const char *no = NULL;
	const char *name = NULL;

	for (_i = 0; _i < argc; _i++) {
		if (!argv[_i]->varname)
			continue;
#if 0 /* anything that can fail? */
		_fail = 0;
#endif

		if (!strcmp(argv[_i]->varname, "no")) {
			no = (argv[_i]->type == WORD_TKN) ? argv[_i]->text : argv[_i]->arg;
		}
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

	return route_map_optimization_magic(self, vty, argc, argv, no, name);
}

