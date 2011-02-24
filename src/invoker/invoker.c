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
#include <sys/stat.h>
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
static const unsigned int DEFAULT_DELAY     = 0;
static const unsigned int RESPAWN_DELAY     = 3;
static const unsigned int MAX_RESPAWN_DELAY = 10;

static const unsigned char EXIT_STATUS_APPLICATION_CONNECTION_LOST = 0xfa;
static const unsigned char EXIT_STATUS_APPLICATION_NOT_FOUND = 0x7f;

// Enumeration of possible application types:
// M_APP     : MeeGo Touch application
// QT_APP    : Qt/generic application
// QDECL_APP : QDeclarative (QML) application
// EXEC_APP  : Executable generic application (can be used with splash screen)
//
enum APP_TYPE { M_APP, QT_APP, QDECL_APP, EXEC_APP, UNKNOWN_APP };

// Environment
extern char ** environ;

// pid of the invoked process
static pid_t g_invoked_pid = -1;

static void sigs_restore(void);

// Forwards Unix signals from invoker to the invoked process
static void sig_forwarder(int sig)
{
    if (g_invoked_pid >= 0)
    {
        if (kill(g_invoked_pid, sig) != 0)
        {
            report(report_error, "Can't send signal to application: %s \n", strerror(errno));
        }
        sigs_restore();
        raise(sig);
    }
}

// Sets signal actions for Unix signals
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

// Sets up the signal forwarder
static void sigs_init(void)
{
    struct sigaction sig;

    memset(&sig, 0, sizeof(sig));
    sig.sa_flags = SA_RESTART;
    sig.sa_handler = sig_forwarder;

    sigs_set(&sig);
}

// Sets up the default signal handler
static void sigs_restore(void)
{
    struct sigaction sig;

    memset(&sig, 0, sizeof(sig));
    sig.sa_flags = SA_RESTART;
    sig.sa_handler = SIG_DFL;

    sigs_set(&sig);
}

// Shows a list of credentials that the client has
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

// Receive ACK
static bool invoke_recv_ack(int fd)
{
    uint32_t action;

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

// Inits a socket connection for the given application type
static int invoker_init(enum APP_TYPE app_type)
{
    int fd;
    struct sockaddr_un sun;

    fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
    {
        warning("Failed to open invoker socket.\n");
        return -1;
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
    else if (app_type == QDECL_APP)
    {
      strncpy(sun.sun_path, INVOKER_QDECL_SOCK, maxSize);
    }
    else if (app_type == EXEC_APP)
    {
      strncpy(sun.sun_path, INVOKER_EXEC_SOCK, maxSize);
    }
    else
    {
        die(1, "Unknown type of application: %d\n", app_type);
    }

    sun.sun_path[maxSize] = '\0';

    if (connect(fd, (struct sockaddr *)&sun, sizeof(sun)) < 0)
    {
        warning("Failed to initiate connect on the socket.\n");
        return -1;
    }

    return fd;
}

// Receives pid of the invoked process.
// Invoker doesn't know it, because the launcher daemon
// is the one who forks.
static uint32_t invoker_recv_pid(int fd)
{
  uint32_t action, pid;

   // Receive action.
  invoke_recv_msg(fd, &action);

  if (action != INVOKER_MSG_PID)
      die(1, "Received a bad pid (%08x)\n", action);

  // Receive pid.
  invoke_recv_msg(fd, &pid);
  return pid;
}

// Receives exit status of the invoked process
static uint32_t invoker_recv_exit(int fd)
{
  uint32_t action, status;

  // Receive action.
  invoke_recv_msg(fd, &action);

  if (action != INVOKER_MSG_EXIT)
  {
      // Boosted application process was killed somehow.
      // Let's give applauncherd process some time to cope 
      // with this situation.
      sleep(2);

      // If nothing happend, return
      return EXIT_STATUS_APPLICATION_CONNECTION_LOST;
  }
  
  // Receive exit status.
  invoke_recv_msg(fd, &status);
  return status;
}

// Sends magic number / protocol version
static void invoker_send_magic(int fd, uint32_t options)
{
    // Send magic.
    invoke_send_msg(fd, INVOKER_MSG_MAGIC | INVOKER_MSG_MAGIC_VERSION | options);
}

// Sends the process name to be invoked.
static void invoker_send_name(int fd, char *name)
{
    invoke_send_msg(fd, INVOKER_MSG_NAME);
    invoke_send_str(fd, name);
}

static void invoker_send_splash_file(int fd, char *filename)
{
    invoke_send_msg(fd, INVOKER_MSG_SPLASH);
    invoke_send_str(fd, filename);
}

static void invoker_send_exec(int fd, char *exec)
{
    invoke_send_msg(fd, INVOKER_MSG_EXEC);
    invoke_send_str(fd, exec);
}

static void invoker_send_args(int fd, int argc, char **argv)
{
    int i;

    invoke_send_msg(fd, INVOKER_MSG_ARGS);
    invoke_send_msg(fd, argc);
    for (i = 0; i < argc; i++)
    {
        debug("param %d %s \n", i, argv[i]);
        invoke_send_str(fd, argv[i]);
    }
}

static void invoker_send_prio(int fd, int prio)
{
    invoke_send_msg(fd, INVOKER_MSG_PRIO);
    invoke_send_msg(fd, prio);
}

// Sends booster respawn delay
static void invoker_send_delay(int fd, int delay)
{
    invoke_send_msg(fd, INVOKER_MSG_DELAY);
    invoke_send_msg(fd, delay);
}

// Sends UID and GID
static void invoker_send_ids(int fd, int uid, int gid)
{
    invoke_send_msg(fd, INVOKER_MSG_IDS);
    invoke_send_msg(fd, uid);
    invoke_send_msg(fd, gid);
}

// Sends the environment variables
static void invoker_send_env(int fd)
{
    int i, n_vars;

    // Count environment variables.
    for (n_vars = 0; environ[n_vars] != NULL; n_vars++) ;

    invoke_send_msg(fd, INVOKER_MSG_ENV);
    invoke_send_msg(fd, n_vars);

    for (i = 0; i < n_vars; i++)
    {
        invoke_send_str(fd, environ[i]);
    }

    return;
}

// Sends I/O descriptors
static void invoker_send_io(int fd)
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
    }

    return;
}

// Sends the END message
static void invoker_send_end(int fd)
{
    invoke_send_msg(fd, INVOKER_MSG_END);
    invoke_recv_ack(fd);

}

// Prints the usage and exits with given status
static void usage(int status)
{
    printf("\nUsage: %s [options] [--type=TYPE] [file] [args]\n\n"
           "Launch m, qt, or qdeclarative application compiled as a shared library (-shared) or\n"
           "a position independent executable (-pie) through %s.\n\n"
           "TYPE chooses the type of booster used. Qt-booster may be used to\n"
           "launch anything. Possible values for TYPE:\n"
           "  m                      Launch a MeeGo Touch application.\n"
           "  q (or qt)              Launch a Qt application.\n"
           "  d                      Launch a Qt Declarative (QML) application.\n"
           "  e                      Launch any application, even if it's not a library.\n"
           "                         Can be used if only splash screen is wanted.\n\n"
           "Options:\n"
           "  -c, --creds            Print Aegis security credentials (if enabled).\n"
           "  -d, --delay SECS       After invoking sleep for SECS seconds\n"
           "                         (default %d).\n"
           "  -r, --respawn SECS     After invoking respawn new booster after SECS seconds\n"
           "                         (default %d, max %d).\n"
           "  -w, --wait-term        Wait for launched process to terminate (default).\n"
           "  -n, --no-wait          Do not wait for launched process to terminate.\n"
           "  -G, --global-syms      Places symbols in the application binary and its\n"
           "                         libraries to the global scope.\n"
           "                         See RTLD_GLOBAL in the dlopen manual page.\n"
           "  -s, --single-instance  Launch the application as a single instance.\n"
           "                         The existing application window will be activated\n"
           "                         if already launched.\n"
           "  -S, --splash FILE      Show splash screen from the FILE.\n"
           "  -L, --splash-landscape LANDSCAPE-FILE\n"
           "                         Show splash screen from the LANDSCAPE-FILE\n"
           "                         in case the device is in landscape orientation.\n"
           "                         (To be implemented)\n"
           "  -h, --help             Print this help.\n\n"
           "Example: %s --type=m /usr/bin/helloworld\n\n",
           PROG_NAME_INVOKER, PROG_NAME_LAUNCHER, DEFAULT_DELAY, RESPAWN_DELAY, MAX_RESPAWN_DELAY, PROG_NAME_INVOKER);

    exit(status);
}

// Return delay as integer 
static unsigned int get_delay(char *delay_arg, char *param_name)
{
    unsigned int delay = DEFAULT_DELAY;

    if (delay_arg)
    {
        errno = 0; // To distinguish success/failure after call
        delay = strtoul(delay_arg, NULL, 10);

        // Check for various possible errors
        if ((errno == ERANGE && delay == ULONG_MAX) || delay == 0)
        {
            report(report_error, "Wrong value of %s parameter: %s\n", param_name, delay_arg);
            usage(1);
        }
    }
    
    return delay;
}

// Fallback for invoke if connection to the launcher is broken.
// Forks a new process if wait term is not used.
void invoke_fallback(char **prog_argv, char *prog_name, bool wait_term)
{
    // Connection with launcher is broken,
    // try to launch application via execve
    warning("Connection with launcher process is broken. \n");
    warning("Trying to start application as a binary executable without launcher...\n");

    // Fork if wait_term not set
    if(!wait_term)
    {
        // Fork a new process
        pid_t newPid = fork();

        if (newPid == -1)
        {
            report(report_error, "Invoker failed to fork\n");
            exit(EXIT_FAILURE);
        }
        else if (newPid != 0) /* parent process */
        {
            return;
        }
    }

    // Exec the process image
    execve(prog_name, prog_argv, environ);
    perror("execve");   /* execve() only returns on error */
    exit(EXIT_FAILURE);
}

// "normal" invoke through a socket connection
int invoke_remote(int fd, int prog_argc, char **prog_argv, char *prog_name,
                  uint32_t magic_options, bool wait_term, unsigned int respawn_delay,
                  char *splash_file)
{
    int status = 0;

    // Get process priority
    errno = 0;
    int prog_prio = getpriority(PRIO_PROCESS, 0);
    if (errno && prog_prio < 0)
    {
        prog_prio = 0;
    }

    // Connection with launcher process is established,
    // send the data.
    invoker_send_magic(fd, magic_options);
    invoker_send_name(fd, prog_argv[0]);
    invoker_send_exec(fd, prog_name);
    invoker_send_args(fd, prog_argc, prog_argv);
    invoker_send_prio(fd, prog_prio);
    invoker_send_delay(fd, respawn_delay);
    invoker_send_ids(fd, getuid(), getgid());
    if (( magic_options & INVOKER_MSG_MAGIC_OPTION_SPLASH_SCREEN ) != 0)
        invoker_send_splash_file(fd, splash_file);
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
        g_invoked_pid = invoker_recv_pid(fd);
        debug("Booster's pid is %d \n ", g_invoked_pid);

        // Forward signals to the invoked process
        sigs_init();

        // Wait for exit status from the invoked application
        status = invoker_recv_exit(fd);

        // Restore default signal handlers
        sigs_restore();
    }

    return status;
}

// Invokes the given application
static int invoke(int prog_argc, char **prog_argv, char *prog_name,
                  enum APP_TYPE app_type, uint32_t magic_options, bool wait_term, unsigned int respawn_delay,
                  char *splash_file)
{
    int status = 0;

    if (prog_name && prog_argv)
    {
        // If invoker cannot find the socket to connect to,
        // exit with an error message.
        int fd = invoker_init(app_type);
        if (fd == -1)
        {
            report(report_error, "Cannot find booster socket.\n");
            exit(EXIT_FAILURE);
        }
        // "normal" invoke through a socket connetion
        else
        {
            status = invoke_remote(fd, prog_argc, prog_argv, prog_name,
                                   magic_options, wait_term, respawn_delay,
                                   splash_file);
            close(fd);
        }
    }
    
    return status;
}

int main(int argc, char *argv[])
{
    enum APP_TYPE app_type      = UNKNOWN_APP;
    int           prog_argc     = 0;
    uint32_t      magic_options = 0;
    bool          wait_term     = true;
    unsigned int  delay         = DEFAULT_DELAY;
    unsigned int  respawn_delay = RESPAWN_DELAY;
    char        **prog_argv     = NULL;
    char         *prog_name     = NULL;
    char         *splash_file   = NULL;
    struct stat   file_stat;

    // wait-term parameter by default
    magic_options |= INVOKER_MSG_MAGIC_OPTION_WAIT;

    // Called with a different name (old way of using invoker) ?
    if (!strstr(argv[0], PROG_NAME_INVOKER) )
    {
        die(1,
            "Incorrect use of invoker, don't use symlinks. "
            "Run invoker explicitly from e.g. a D-Bus service file instead.\n");
    }

    // Stops parsing args as soon as a non-option argument is encountered
    putenv("POSIXLY_CORRECT=1");

    // Options recognized
    struct option longopts[] = {
        {"help",      no_argument,       NULL, 'h'},
        {"creds",     no_argument,       NULL, 'c'},
        {"wait-term", no_argument,       NULL, 'w'},
        {"no-wait",   no_argument,       NULL, 'n'},
        {"global-syms", no_argument,     NULL, 'G'},
        {"deep-syms", no_argument,       NULL, 'D'},
        {"single-instance", no_argument, NULL, 's'},
        {"type",      required_argument, NULL, 't'},
        {"delay",     required_argument, NULL, 'd'},
        {"respawn",   required_argument, NULL, 'r'},
        {"splash",    required_argument, NULL, 'S'},
        {"splash-landscape", required_argument, NULL, 'L'},
        {0, 0, 0, 0}
    };

    // Parse options
    // TODO: Move to a function
    int opt;
    while ((opt = getopt_long(argc, argv, "hcwnGDsd:t:r:S:L:", longopts, NULL)) != -1)
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
            // nothing to do, it's by default now
            break;

        case 'n':
            wait_term = false;
            magic_options &= (~INVOKER_MSG_MAGIC_OPTION_WAIT);
            break;

        case 'G':
            magic_options |= INVOKER_MSG_MAGIC_OPTION_DLOPEN_GLOBAL;
            break;

        case 'D':
            magic_options |= INVOKER_MSG_MAGIC_OPTION_DLOPEN_DEEP;
            break;

        case 't':
            if (strcmp(optarg, "m") == 0)
                app_type = M_APP;
            else if (strcmp(optarg, "q") == 0 || strcmp(optarg, "qt") == 0)
                app_type = QT_APP;
            else if (strcmp(optarg, "d") == 0)
                app_type = QDECL_APP;
            else if (strcmp(optarg, "e") == 0)
                app_type = EXEC_APP;
            else
            {
                report(report_error, "Unknown application type: %s \n", optarg);
                usage(1);
            }
            break;

        case 'd':
            delay = get_delay(optarg, "delay");
            break;

        case 'r':
            respawn_delay = get_delay(optarg, "respawn delay");
            if (respawn_delay > MAX_RESPAWN_DELAY)
            {
                report(report_error, "Booster respawn delay exceeds max possible time.\n");
                usage(1);
            }
            break;

        case 's':
            magic_options |= INVOKER_MSG_MAGIC_OPTION_SINGLE_INSTANCE;
            break;

        case 'S':
            magic_options |= INVOKER_MSG_MAGIC_OPTION_SPLASH_SCREEN;
            splash_file = optarg;
            break;

        case 'L':
            // Just a placeholder for future development
            // of landscape splash screen
            break;

        case '?':
            usage(1);
        }
    }

    // Option processing stops as soon as application name is encountered
    if (optind < argc)
    {
        prog_name = search_program(argv[optind]);
        if (!prog_name)
        {
            report(report_error, "Can't find application to invoke.\n");
            usage(0);
        }

        prog_argc = argc - optind;
        prog_argv = &argv[optind];
    }

    // Check if application name isn't defined
    if (!prog_name)
    {
        report(report_error, "Application's name is not defined.\n");
        usage(1);
    }

    // Check if application exists
    if (stat(prog_name, &file_stat))
    {
        report(report_error, "%s: not found\n", prog_name);
        return EXIT_STATUS_APPLICATION_NOT_FOUND;
    }

    // Check that 
    if (!S_ISREG(file_stat.st_mode) && !S_ISLNK(file_stat.st_mode))
    {
        report(report_error, "%s: not a file\n", prog_name);
        return EXIT_STATUS_APPLICATION_NOT_FOUND;
    }

    // Check if application type is unknown
    if (app_type == UNKNOWN_APP)
    {
        report(report_error, "Application's type is unknown.\n");
        usage(1);
    }

    // Send commands to the launcher daemon
    info("Invoking execution: '%s'\n", prog_name);
    int ret_val = invoke(prog_argc, prog_argv, prog_name, app_type, magic_options, wait_term, respawn_delay, splash_file);

    // Sleep for delay before exiting
    if (delay)
    {
        // DBUS cannot cope some times if the invoker exits too early.
        debug("Delaying exit for %d seconds..\n", delay);
        sleep(delay);
    }

    return ret_val;
}
