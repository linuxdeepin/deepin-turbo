/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of applauncherd
**
** If you have questions regarding the use of this file, please contact
** Nokia at directui@nokia.com.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <bits/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <limits.h>
#include <getopt.h>

#include "report.h"
#include "protocol.h"
#include "invokelib.h"
#include "search.h"

#ifdef HAVE_CREDS
    #include <sys/creds.h>
#endif

// Delay before exit
static const int DEFAULT_DELAY = 0;

// Enumeration of possible application types:
// M_APP: MeeGo Touch application
// QT_APP: Qt/generic application
//
enum APP_TYPE { M_APP, QT_APP, UNKNOWN_APP };

// Environment
extern char ** environ;

// pid of invoked process
static pid_t invoked_pid = -1;


static void sig_forwarder(int sig)
{
    if (invoked_pid >= 0)
    {
        if (kill(invoked_pid, sig) != 0)
        {
            die(1, "Can't send signal to application: %s \n", strerror(errno));
        }
    }
}

static void sigs_set(struct sigaction *sig)
{
    sigaction(SIGABRT,   sig, NULL);
    sigaction(SIGALRM,   sig, NULL);
    sigaction(SIGBUS,    sig, NULL);
    sigaction(SIGCHLD,   sig, NULL);
    sigaction(SIGCONT,   sig, NULL);
    sigaction(SIGHUP,    sig, NULL);
    sigaction(SIGINT,    sig, NULL);
    sigaction(SIGIO,     sig, NULL);
    sigaction(SIGIOT,    sig, NULL);
    sigaction(SIGPIPE,   sig, NULL);
    sigaction(SIGPROF,   sig, NULL);
    sigaction(SIGPWR,    sig, NULL);
    sigaction(SIGQUIT,   sig, NULL);
    sigaction(SIGSEGV,   sig, NULL);
    sigaction(SIGSYS,    sig, NULL);
    sigaction(SIGTERM,   sig, NULL);
    sigaction(SIGTRAP,   sig, NULL);
    sigaction(SIGTSTP,   sig, NULL);
    sigaction(SIGTTIN,   sig, NULL);
    sigaction(SIGTTOU,   sig, NULL);
    sigaction(SIGUSR1,   sig, NULL);
    sigaction(SIGUSR2,   sig, NULL);
    sigaction(SIGVTALRM, sig, NULL);
    sigaction(SIGWINCH,  sig, NULL);
    sigaction(SIGXCPU,   sig, NULL);
    sigaction(SIGXFSZ,   sig, NULL);
}

static void sigs_init(void)
{
    struct sigaction sig;

    memset(&sig, 0, sizeof(sig));
    sig.sa_flags = SA_RESTART;
    sig.sa_handler = sig_forwarder;

    sigs_set(&sig);
}

static void sigs_restore(void)
{
    struct sigaction sig;

    memset(&sig, 0, sizeof(sig));
    sig.sa_flags = SA_RESTART;
    sig.sa_handler = SIG_DFL;

    sigs_set(&sig);
}


/*
 * Show a list of credentials that the client has
 */
static void show_credentials(void)
{
#ifdef HAVE_CREDS
    creds_t creds;
    creds_value_t value;
    creds_type_t type;
    int i;

    creds = creds_gettask(0);
    for (i = 0; (type = creds_list(creds, i,  &value)) != CREDS_BAD; ++i) {
        char buf[200];
        (void)creds_creds2str(type, value, buf, sizeof(buf));
        buf[sizeof(buf)-1] = 0;
        printf("\t%s\n", buf);
    }
    creds_free(creds);
#else
    printf("Security credential information isn't available.\n");
#endif

    exit(0);
}

static bool invoke_recv_ack(int fd)
{
    uint32_t action;

    // Receive ACK.
    invoke_recv_msg(fd, &action);

    if (action == INVOKER_MSG_BAD_CREDS)
    {
        die(1, "Security credential check failed.\n");
    }
    else if (action != INVOKER_MSG_ACK)
    {
        die(1, "Received wrong ack (%08x)\n", action);
    }

    return true;
}

static int invoker_init(enum APP_TYPE app_type)
{
    int fd;
    struct sockaddr_un sun;

    fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
    {
        die(1, "Failed to open invoker socket.\n");
    }

    sun.sun_family = AF_UNIX;  //AF_FILE;

    const int maxSize = sizeof(sun.sun_path) - 1;
    if(app_type == M_APP)
    {
        strncpy(sun.sun_path, INVOKER_M_SOCK, maxSize);
    }
    else if (app_type == QT_APP)
    {
        strncpy(sun.sun_path, INVOKER_QT_SOCK, maxSize);
    }
    else
    {
        die(1, "Unknown type of application: %d\n", app_type);
    }

    sun.sun_path[maxSize] = '\0';

    if (connect(fd, (struct sockaddr *)&sun, sizeof(sun)) < 0)
    {
        die(1, "Failed to initiate connect on the socket.\n");
    }

    return fd;
}

static uint32_t invoker_recv_pid(int fd)
{
  uint32_t action, pid;

  /* Receive action. */
  invoke_recv_msg(fd, &action);

  if (action != INVOKER_MSG_PID)
      die(1, "receiving bad pid (%08x)\n", action);

  /* Receive pid. */
  invoke_recv_msg(fd, &pid);
  return pid;
}

static uint32_t invoker_recv_exit(int fd)
{
  uint32_t action, status;

  /* Receive action. */
  invoke_recv_msg(fd, &action);

  if (action != INVOKER_MSG_EXIT)
  {
      // probably boosted application process was killed somehow
      // let's get applauncherd process some time to cope with this situation
      sleep(1);

      // if nothing happend, just exit with error message
      die(1, "receiving bad exit status (%08x)\n", action);
  }
  /* Receive pid. */
  invoke_recv_msg(fd, &status);
  return status;
}


static bool invoker_send_magic(int fd, int options)
{
    // Send magic.
    invoke_send_msg(fd, INVOKER_MSG_MAGIC | INVOKER_MSG_MAGIC_VERSION | options);
    invoke_recv_ack(fd);

    return true;
}

static bool invoker_send_name(int fd, char *name)
{
    // Send action.
    invoke_send_msg(fd, INVOKER_MSG_NAME);
    invoke_send_str(fd, name);
    invoke_recv_ack(fd);

    return true;
}

static bool invoker_send_exec(int fd, char *exec)
{
    // Send action.
    invoke_send_msg(fd, INVOKER_MSG_EXEC);
    invoke_send_str(fd, exec);
    invoke_recv_ack(fd);

    return true;
}

static bool invoker_send_args(int fd, int argc, char **argv)
{
    int i;

    // Send action.
    invoke_send_msg(fd, INVOKER_MSG_ARGS);
    invoke_send_msg(fd, argc);
    for (i = 0; i < argc; i++)
    {
        debug("param %d %s \n", i, argv[i]);
        invoke_send_str(fd, argv[i]);
    }
    invoke_recv_ack(fd);

    return true;
}

static bool invoker_send_prio(int fd, int prio)
{
    // Send action.
    invoke_send_msg(fd, INVOKER_MSG_PRIO);
    invoke_send_msg(fd, prio);

    invoke_recv_ack(fd);

    return true;
}

static bool invoker_send_ids(int fd, int uid, int gid)
{
    // Send action.
    invoke_send_msg(fd, INVOKER_MSG_IDS);
    invoke_send_msg(fd, uid);
    invoke_send_msg(fd, gid);

    invoke_recv_ack(fd);

    return true;
}

static bool invoker_send_env(int fd)
{
    int i, n_vars;

    // Count the amount of environment variables.
    for (n_vars = 0; environ[n_vars] != NULL; n_vars++) ;

    // Send action.
    invoke_send_msg(fd, INVOKER_MSG_ENV);
    invoke_send_msg(fd, n_vars);

    for (i = 0; i < n_vars; i++)
    {
        invoke_send_str(fd, environ[i]);
    }

    return true;
}

static bool invoker_send_io(int fd)
{
    struct msghdr msg;
    struct cmsghdr *cmsg = NULL;
    int io[3] = { 0, 1, 2 };
    char buf[CMSG_SPACE(sizeof(io))];
    struct iovec iov;
    int dummy;

    memset(&msg, 0, sizeof(struct msghdr));

    iov.iov_base = &dummy;
    iov.iov_len = 1;

    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_len = CMSG_LEN(sizeof(io));
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;

    memcpy(CMSG_DATA(cmsg), io, sizeof(io));

    msg.msg_controllen = cmsg->cmsg_len;

    invoke_send_msg(fd, INVOKER_MSG_IO);
    if (sendmsg(fd, &msg, 0) < 0)
    {
        warning("sendmsg failed in invoker_send_io: %s \n", strerror(errno));
        return  false;
    }

    return true;
}

static bool invoker_send_end(int fd)
{
    // Send action.
    invoke_send_msg(fd, INVOKER_MSG_END);
    invoke_recv_ack(fd);

    return true;
}

static void usage(int status)
{
    printf("\nUsage: %s [options] [--type=TYPE]  [file] [args]\n"
           "Launch m or qt application.\n\n"
           "TYPE chooses the type of booster used. Qt-booster may be used to launch anything.\n"
           "Possible values for TYPE: \n"
           "  m                   Launch a MeeGo Touch application.\n"
           "  qt                  Launch a Qt application.\n\n"
           "Options:\n"
           "  -c, --creds             Print Aegis security credentials (if enabled).\n"
           "  -d, --delay SECS        After invoking sleep for SECS seconds (default %d).\n"
           "  -w, --wait-term         Wait for launched process to terminate.\n"
           "  -h, --help              Print this help message.\n\n"
           "Example: %s --type=m /usr/bin/helloworld \n",
           PROG_NAME, DEFAULT_DELAY, PROG_NAME);

    exit(status);
}

static unsigned int get_delay(char *delay_arg)
{
    unsigned int delay;

    if (delay_arg)
    {
        errno = 0; // To distinguish success/failure after call
        delay = strtoul(delay_arg, NULL, 10);

        // Check for various possible errors
        if ((errno == ERANGE && delay == ULONG_MAX) || delay == 0)
        {
            report(report_error, "Wrong value of delay parameter: %s\n", delay_arg);
            usage(1);
        }
    }
    else
        delay = DEFAULT_DELAY;

    return delay;
}

static int invoke(int prog_argc, char **prog_argv, char *prog_name,
                   enum APP_TYPE app_type, int magic_options, bool wait_term)
{
    int status = 0;

    if (prog_name && prog_argv)
    {
        errno = 0;
        int prog_prio = getpriority(PRIO_PROCESS, 0);

        if (errno && prog_prio < 0)
        {
            prog_prio = 0;
        }

        int uid = getuid();
        int gid = getgid();

        int fd = invoker_init(app_type);

        invoker_send_magic(fd, magic_options);
        invoker_send_name(fd, prog_argv[0]);
        invoker_send_exec(fd, prog_name);
        invoker_send_args(fd, prog_argc, prog_argv);
        invoker_send_prio(fd, prog_prio);
        invoker_send_ids(fd, uid, gid);
        invoker_send_io(fd);
        invoker_send_env(fd);
        invoker_send_end(fd);

        if (prog_name)
        {
            free(prog_name);
        }

        // Wait for launched process to exit
        if (wait_term)
        {
            invoked_pid = invoker_recv_pid(fd);
            debug("Booster's pid is %d \n ", invoked_pid);

            // forward signals to invoked process
            sigs_init();

            // wait for exit status from invoked application
            status = invoker_recv_exit(fd);

            // restore default signal handlers
            sigs_restore();
        }

        close(fd);
    }
    return status;
}

int main(int argc, char *argv[])
{
    enum APP_TYPE app_type      = UNKNOWN_APP;
    int           prog_argc     = 0;
    int           magic_options = 0;
    bool          wait_term     = false;
    unsigned int  delay         = DEFAULT_DELAY;
    char        **prog_argv     = NULL;
    char         *prog_name     = NULL;

    if (!strstr(argv[0], PROG_NAME) )
    {
        // Called with a different name, old way of using invoker
        die(1,
            "Incorrect use of invoker, don't use symlinks. "
            "Run invoker explicitly from e.g. a D-Bus service file instead.\n");
    }

    // stops args parsing as soon as a non-option argument is encountered
    putenv("POSIXLY_CORRECT=1");

    struct option longopts[] = {
        {"help", no_argument, NULL, 'h'},
        {"creds", no_argument, NULL, 'c'},
        {"wait-term", no_argument, NULL, 'w'},
        {"type", required_argument, NULL, 't'},
        {"delay", required_argument, NULL, 'd'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "hcwd:t:", longopts, NULL)) != -1)
    {
        switch(opt)
        {
        case 'h':
            usage(0);
            break;

        case 'c':
            show_credentials();
            break;

        case 'w':
            wait_term = true;
            magic_options = INVOKER_MSG_MAGIC_OPTION_WAIT;
            break;

        case 't':
            if (strcmp(optarg, "qt") == 0)
                app_type = QT_APP;
            else if (strcmp(optarg, "m") == 0)
                app_type = M_APP;
            else
            {
                report(report_error, "Unknown application type: %s \n", optarg);
                usage(1);
            }
            break;

        case 'd':
            delay = get_delay(optarg);
            break;

        case '?':
            usage(1);
        }
    }

    // option processing stops as soon as application name is encountered
    if (optind < argc)
    {
        prog_name = search_program(argv[optind]);
        if (!prog_name)
        {
            report(report_error, "Can't find application to invoke.\n");
            usage(0);
        }

        char *period = strstr(argv[optind], ".launch");
        if (period)
        {
            *period = '\0';
        }

        prog_argc = argc - optind;
        prog_argv = &argv[optind];
    }

    if (!prog_name)
    {
        die(1, "Application's name is unknown.\n");
    }

    if (app_type == UNKNOWN_APP)
    {
        die(1, "Application's type is unknown. \n");
    }
    // Send commands to the launcher daemon
    info("Invoking execution: '%s'\n", prog_name);
    int ret_val = invoke(prog_argc, prog_argv, prog_name, app_type, magic_options, wait_term);

    // Sleep for delay before exiting
    if (delay)
    {
        // DBUS cannot cope some times if the invoker exits too early.
        debug("Delaying exit for %d seconds..\n", delay);
        sleep(delay);
    }

    return ret_val;
}
