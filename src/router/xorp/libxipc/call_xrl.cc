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



#include "xrl_module.h"

#include "libxorp/xorp.h"
#include "libxorp/xlog.h"

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "xrl_std_router.hh"
#include "xrl_args.hh"
#include "xrl_parser_input.hh"


static const char* ROUTER_NAME = "call_xrl";

static int wait_time = 1000;    // Time to wait for the callback in ms.
static int retry_count = 0; // Number of times to resend xrl on error.
static bool stdin_forever = false;

enum
{
    // Return values from call_xrl
    OK = 0, BADXRL = -1, NOCALLBACK = -2
};

static void
response_handler(const XrlError& e,
                 XrlArgs*    response,
                 bool*       done_flag,
                 bool*       resolve_failed,
                 Xrl*        xrl)
{
    UNUSED(xrl);

    if (e == XrlError::RESOLVE_FAILED())
    {
        XLOG_ERROR("Failed.  Reason: %s (\"%s\")",
                   e.str().c_str(), xrl->str().c_str());
        *resolve_failed = true;
        return;
    }
    if (e != XrlError::OKAY())
    {
        XLOG_ERROR("Failed.  Reason: %s (\"%s\")",
                   e.str().c_str(), xrl->str().c_str());
        exit(3);
    }
    if (!response->str().empty())
    {
        printf("%s\n", response->str().c_str());
        fflush(stdout);
    }
    *done_flag = true;
}

void usage()
{
    fprintf(stderr,
            "Usage: call_xrl [options] "
            "<[-E] -f file1 ... fileN | xrl1 ... xrl>\n"
            "where -f reads XRLs from a file rather than the command line\n"
            "and   -E only passes XRLs through the preprocessor\n"
            "Options:\n"
            "  -F <host>[:<port>]   Specify Finder host and port\n"
            "  -r <retries>         Specify number of retry attempts\n"
            "  -w <time ms>         Time to wait for a callback\n");
}

static int
call_xrl(EventLoop& e, XrlRouter& router, const char* request)
{
    try
    {
        Xrl x(request);

        int tries;
        bool done, resolve_failed;

        tries = 0;
        done = false;
        resolve_failed = true;

        while (xorp_do_run && done == false && tries <= retry_count)
        {
            resolve_failed = false;
            router.send(x, callback(&response_handler,
                                    &done,
                                    &resolve_failed,
                                    &x));

            bool timed_out = false;
            XorpTimer timeout = e.set_flag_after_ms(wait_time, &timed_out);
            while (xorp_do_run && timed_out == false && done == false)
            {
                // NB we don't test for resolve failed here because if
                // resolved failed we want to wait before retrying.
                e.run();
            }
            tries++;

            if (resolve_failed)
            {
                sleep(1);
                continue;
            }

            if (timed_out)
            {
                XLOG_WARNING("request: %s no response waited %d ms", request,
                             wait_time);
                continue;
            }

            if (router.connected() == false)
            {
                XLOG_ERROR("Lost connection to finder\n");
                xorp_do_run = 0;
                break;
            }
        }//while

        if (resolve_failed)
        {
            XLOG_WARNING("request: %s resolve failed", request);
        }

        if (false == done && true == resolve_failed)
            XLOG_WARNING("request: %s failed after %d retries",
                         request, retry_count);
        return done == true ? OK : NOCALLBACK;
    }
    catch (const InvalidString& s)
    {
        cerr << s.str() << endl;
        return BADXRL;
    }
}

static void
preprocess_file(XrlParserFileInput& xfp)
{
    try
    {
        while (!xfp.eof())
        {
            string l;
            if (xfp.getline(l) == false)
            {
                continue;
            }
            /* if preprocessing only print line and continue. */
            cout << l << endl;
        }
    }
    catch (...)
    {
        xorp_catch_standard_exceptions();
    }
}

static int
input_file(EventLoop&          eventloop,
           XrlRouter&          router,
           XrlParserFileInput& xfp)
{

    while (!xfp.eof())
    {
        string l;
        if (xfp.getline(l) == true)
        {
            continue;
        }
        /* if line length is zero or line looks like a preprocessor directive
         * continue. */
        if (l.length() == 0 || l[0] == '#')
        {
            continue;
        }
        int err = call_xrl(eventloop, router, l.c_str());
        if (err)
        {
            cerr << xfp.stack_trace() << endl;
            cerr << "Xrl failed: " << l;
            return err;
        }
    }
    return 0;
}

static int
input_files(EventLoop&  e,
            XrlRouter&  router,
            int     argc,
            char* const argv[],
            bool    pponly)
{
    do
    {
        if (argc == 0 || argv[0][0] == '-')
        {
            do
            {
                XrlParserFileInput xfp(&cin);
                if (pponly)
                {
                    preprocess_file(xfp);
                }
                else
                {
                    int err = input_file(e, router, xfp);
                    if (err)
                    {
                        return err;
                    }
                }
                TimerList::system_sleep(TimeVal(0, 250000));
            }
            while (stdin_forever);
        }
        else
        {
            XrlParserFileInput xfp(argv[0]);
            if (pponly)
            {
                preprocess_file(xfp);
            }
            else
            {
                input_file(e, router, xfp);
            }
        }
        argc--;
        argv++;
    }
    while (argc > 0);
    return 0;
}

static int
input_cmds(EventLoop&  e,
           XrlRouter&  router,
           int         argc,
           char* const argv[])
{
    for (int i = 0; i < argc; i++)
    {
        int err = call_xrl(e, router, argv[i]);
        switch (err)
        {
            case OK:
                break;
            case BADXRL:
                XLOG_ERROR("Bad XRL syntax: %s\nStopping.", argv[i]);
                return err;
                break;
            case NOCALLBACK:
                XLOG_ERROR("No callback: %s\nStopping.", argv[i]);
                return err;
                break;
        }
    }
    return 0;
}

// int
// main(int argc, char* const argv[])
// {
//     XorpUnexpectedHandler x(xorp_unexpected_handler);
//     //
//     // Initialize and start xlog
//     //
//     xlog_init(argv[0], NULL);
//     xlog_set_verbose(XLOG_VERBOSE_LOW);      // Least verbose messages
//     // XXX: verbosity of the error messages temporary increased
//     xlog_level_set_verbose(XLOG_LEVEL_ERROR, XLOG_VERBOSE_HIGH);
//     xlog_add_default_output();
//     xlog_start();

//     bool pponly = false; // Pre-process files only
//     bool fileinput = false;
//     string finder_host = FinderConstants::FINDER_DEFAULT_HOST().str();
//     uint16_t port = FinderConstants::FINDER_DEFAULT_PORT();
//     int c;
//     char *tmpport;
//     while ((c = getopt(argc, argv, "F:Efir:w:")) != -1) {
//  switch (c) {
//  case 'E':
//      pponly = true;
//      fileinput = true;
//      break;
//  case 'f':
//      fileinput = true;
//      break;
//  case 'i':
//      stdin_forever = true;
//      break;
//  case 'r':
//      retry_count = atoi(optarg);
//      break;
//  case 'w':
//      wait_time = atoi(optarg) * 1000;
//      break;
//  case 'F':
//      finder_host = optarg;
//      tmpport = strchr(optarg, ':');
//      if (tmpport != NULL) {
//      *tmpport++ = '\0';
//      port = atoi(tmpport);
//      }
//      break;
//  default:
//      usage();
//      return -1;
//  }
//     }
//     argc -= optind;
//     argv += optind;

//     setup_dflt_sighandlers();

//     int rv = 0;
//     try {
//  EventLoop e;
//  XrlStdRouter router(e, ROUTER_NAME, finder_host.c_str(), port);

//  router.finalize();

//  while (xorp_do_run && !router.failed() && !router.ready()) {
//      e.run();
//  }

//  if (router.failed()) {
//      XLOG_ERROR("Router failed to communicate with finder.\n");
//      rv = 1;
//  }
//  else if (!router.ready()) {
//      XLOG_ERROR("Connected to finder, but did not become ready.\n");
//      rv = 2;
//  }
//  else {
//      if (fileinput) {
//      if (input_files(e, router, argc, argv, pponly)) {
//          rv = -1;
//      }
//      } else if (argc != 0) {
//      if (input_cmds(e, router, argc, argv)) {
//          rv = -1;
//      }
//      } else {
//      usage();
//      }
//  }
//     } catch(...) {
//  xorp_catch_standard_exceptions();
//     }

//     //
//     // Gracefully stop and exit xlog
//     //
//     xlog_stop();
//     xlog_exit();

//     return rv;
// }
