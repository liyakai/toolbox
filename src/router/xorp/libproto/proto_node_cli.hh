// -*- c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t -*-
// vim:set sts=4 ts=8:

// Copyright (c) 2001-2011 XORP, Inc and Others
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License, Version
// 2.1, June 1999 as published by the Free Software Foundation.
// Redistribution and/or modification of this program under the terms of
// any other version of the GNU Lesser General Public License is not
// permitted.
// 
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For more details,
// see the GNU Lesser General Public License, Version 2.1, a copy of
// which can be found in the XORP LICENSE.lgpl file.
// 
// XORP, Inc, 2953 Bunker Hill Lane, Suite 204, Santa Clara, CA 95054, USA;
// http://xorp.net

// $XORP: xorp/libproto/proto_node_cli.hh,v 1.11 2008/10/02 21:57:18 bms Exp $


#ifndef __LIBPROTO_PROTO_NODE_CLI_HH__
#define __LIBPROTO_PROTO_NODE_CLI_HH__






#include "libxorp/xorp.h"
#include "libxorp/callback.hh"
#include "proto_unit.hh"


//
// Protocol generic CLI access
//


//
// Constants definitions
//

//
// Structures/classes, typedefs and macros
//

typedef XorpCallback1<int, const vector<string>& >::RefPtr CLIProcessCallback;


// 
/**
 * @short Base class for Protocol node CLI access.
 */
class ProtoNodeCli : public ProtoUnit {
public:
    /**
     * Constructor for a given address family and module ID.
     * 
     * @param init_family the address family (AF_INET or AF_INET6
     * for IPv4 and IPv6 respectively).
     * @param init_module_id the module ID XORP_MODULE_* (@ref xorp_module_id).
     */
    ProtoNodeCli(int init_family, xorp_module_id init_module_id);
    
    /**
     * Destructor
     */
    virtual ~ProtoNodeCli();
    
    /**
     * Add a CLI directory level that does not allow user to move to it.
     * 
     * The added directory level does not allow an user to move to it
     * (i.e., user cannot "cd" to that directory level).
     * 
     * @param dir_command_name the directory name to add.
     * @param dir_command_help the help message for the directory.
     * @return XORP_OK on success, otherwise XORP_ERROR.
     */
    int		add_cli_dir_command(const char *dir_command_name,
				    const char *dir_command_help);

    /**
     * Add a CLI directory level that may allow user to move to it.
     * 
     * The added directory level may allow an user to move to it
     * (i.e., user can type the directory name to "cd" to that directory
     * level). 
     * 
     * @param dir_command_name the directory name to add.
     * @param dir_command_help the help message for the directory.
     * @param is_allow_cd if true, allow user to "cd" to that directory.
     * @param dir_cd_prompt if user can "cd" to that directory, the prompt
     * to appear after the "cd".
     * @return XORP_OK on success, otherwise XORP_ERROR.
     */
    int		add_cli_dir_command(const char *dir_command_name,
				    const char *dir_command_help,
				    bool is_allow_cd,
				    const char *dir_cd_prompt);
    
    /**
     * Add a CLI command.
     * 
     * @param command_name the command name to add.
     * @param command_help the command help.
     * @param cli_process_callback the callback function that will be called
     * when this command is executed.
     * @return XORP_OK on success, otherwise XORP_ERROR.
     */
    int		add_cli_command(const char *command_name,
				const char *command_help,
				const CLIProcessCallback& cli_process_callback);

    /**
     * Delete a CLI command.
     * 
     * @param command_name the command name to delete.
     * @return XORP_OK on success, otherwise XORP_ERROR.
     */
    int		delete_cli_command(const char *command_name);

    /**
     * Delete all CLI commands that were added by this node.
     * 
     * @return XORP_OK on success, otherwise XORP_ERROR.
     */
    int		delete_all_cli_commands();

    /**
     * Process a CLI command.
     * 
     * This method is invoked when the CLI has detected a valid command
     * has been entered, and that command has been installed by this node.
     * 
     * @param processor_name the processor name for this command.
     * @param cli_term_name the terminal name the command was entered from.
     * @param cli_session_id the CLI session ID the command was entered from.
     * @param command_name the command name to process.
     * @param command_args the command arguments to process.
     * @param ret_processor_name the processor name to return back to the CLI.
     * @param ret_cli_term_name the terminal name to return back.
     * @param ret_cli_session_id the CLI session ID to return back.
     * @param ret_command_output the command output to return back.
     * @return XORP_OK on success, otherwise XORP_ERROR.
     */
    int		cli_process_command(const string& processor_name,
				    const string& cli_term_name,
				    const uint32_t& cli_session_id,
				    const string& command_name,
				    const string& command_args,
				    string& ret_processor_name,
				    string& ret_cli_term_name,
				    uint32_t& ret_cli_session_id,
				    string& ret_command_output);
    
    /**
     * Print a message to the CLI interface.
     * 
     * @param msg the message string to display.
     * @return the number of characters printed (not including
     * the trailing '\0').
     */
    int		cli_print(const string& msg);
    
    /**
     * Add a CLI command to the CLI manager.
     * 
     * This is a pure virtual function, and it must be implemented
     * by the particular protocol node class that inherits this base class.
     * 
     * @param command_name the command name to add.
     * @param command_help the help message for the command.
     * @param is_command_cd if true, this is a directory level that allows
     * user to "cd" to that directory.
     * @param command_cd_prompt if this is a directory user can "cd" to it,
     * the prompt to appear after the "cd".
     * @param is_command_processor if true, this is an oridinary command
     * that can be invoked for processing rather than a directory level.
     * @return XORP_OK on success, otherwise XORP_ERROR.
     */
    virtual int	add_cli_command_to_cli_manager(const char *command_name,
					       const char *command_help,
					       bool is_command_cd,
					       const char *command_cd_prompt,
					       bool is_command_processor) = 0;

    /**
     * Delete a CLI command from the CLI manager.
     * 
     * This is a pure virtual function, and it must be implemented
     * by the particular protocol node class that inherits this base class.
     * 
     * @param command_name the command name to delete.
     * @return XORP_OK on success, otherwise XORP_ERROR.
     */
    virtual int delete_cli_command_from_cli_manager(const char *command_name) = 0;
    
private:
    
    int		add_cli_command_entry(const char *command_name,
				      const char *command_help,
				      bool is_command_cd,
				      const char *command_cd_prompt,
				      bool is_command_processor,
				      const CLIProcessCallback& cli_process_callback);
    int	cli_process_dummy(const vector<string>&	/* argv */) { return (XORP_OK); }
    
    
    string		_cli_result_string; // The string with the CLI result
    map<string, CLIProcessCallback> _cli_callback_map; // Map with commands
    vector<string> _cli_callback_vector; // Keep commands in order of adding
};

//
// Global variables
//

//
// Global functions prototypes
//

#endif // __LIBPROTO_PROTO_NODE_CLI_HH__
