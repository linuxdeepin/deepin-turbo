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

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <sys/stat.h> 

#define DECL_EXPORT extern "C" __attribute__ ((__visibility__("default")))

namespace
{
    int g_lockFd = -1;
    const std::string LOCK_PATH_BASE("/var/run/single-instance-locks/");
    const std::string LOCK_FILE_NAME("instance.lock");
}

//! Create a path and its components using mask 0777
static bool mkpath(const std::string & path)
{
    for (unsigned int i = 0; i < path.size(); i++)
    {
        if ((i + 1 < path.size() && path[i + 1] == '/' && path[i] != '/') ||
            (i + 1 == path.size()))
        {
            const std::string part = path.substr(0, i + 1);
            if (mkdir(part.c_str(), 0777) != -1)
            {
                // chmod again, because permissions set by mkdir()
                // are modified by umask
                if (chmod(part.c_str(), 0777) == -1)
                {
                    std::cerr << "ERROR!!: chmod() failed: " <<
                            strerror(errno) << std::endl;
                    return false;
                }
            }
        }
    }

    return true;
}

//! Print help.
static void printHelp()
{
    printf("\nUsage: %s [options] [application]\n"
           "Launch application as a single instance.\n"
           "If given application is already running, its window is raised.\n"
           "Give the full path to the application binary.\n\n"
           "Options:\n"
           "  -h, --help          Print this help message.\n\n"
           "Example: %s /usr/bin/helloworld\n\n",
           PROG_NAME_SINGLE_INSTANCE, PROG_NAME_SINGLE_INSTANCE);
}

//! Activate a window.
static int clientMsg(Display *disp, Window win, const char *msg, 
                     unsigned long data0, unsigned long data1,
                     unsigned long data2, unsigned long data3,
                     unsigned long data4)
{
    XEvent event;
    long mask = SubstructureRedirectMask | SubstructureNotifyMask;

    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    event.xclient.message_type = XInternAtom(disp, msg, True);
    event.xclient.window = win;
    event.xclient.format = 32;
    event.xclient.data.l[0] = data0;
    event.xclient.data.l[1] = data1;
    event.xclient.data.l[2] = data2;
    event.xclient.data.l[3] = data3;
    event.xclient.data.l[4] = data4;
    
    // XInternAtom will return None if atom does not exists
    if (event.xclient.message_type && XSendEvent(disp, DefaultRootWindow(disp), False, mask, &event))
    {
        return EXIT_SUCCESS;
    }
    else
    {
        std::cerr <<  "ERROR!!: Cannot send " << msg << " event." << std::endl;
        return EXIT_FAILURE;
    }
}

/*!
 * \brief Return binary name assigned to a process id.
 *
 * Source for the binary name is /proc/[pid]/cmdline.
 *
 * \param pid The process id.
 * \return Full path to the command on success, empty string on failure.
 */
static std::string binaryNameForPid(int pid)
{
    std::string cmdLine;

    std::stringstream ss;
    ss << "/proc/" << pid << "/cmdline";

    std::ifstream procFile;
    procFile.open(ss.str().c_str());
    if (procFile.is_open())
    {
        procFile >> cmdLine;

        size_t nul = cmdLine.find_first_of('\0');
        if (nul != std::string::npos)
        {
            cmdLine = cmdLine.substr(0, nul);
        }
    }

    return cmdLine;
}

/*!
 * \brief Return pid assigned to a window id.
 *
 * \param dpy The X11 display.
 * \param window The window id.
 * \return Pid on success, -1 on failure.
 */
static int windowPid(Display * dpy, Window window)
{
    if (dpy)
    {
        static Atom    pidAtom = XInternAtom(dpy, "_NET_WM_PID", False);
        Atom           type;
        int            format;
        unsigned long  nItems;
        unsigned long  bytesAfter;
        unsigned char *propPID = 0;

        // Get the PID of the window
        if(XGetWindowProperty(dpy, window, pidAtom, 0, 1, False, XA_CARDINAL,
                              &type, &format, &nItems, &bytesAfter, &propPID) == Success)
        {
            if(propPID != 0)
            {
                // If the PID matches, add this window to the result set.
                int pid = *(reinterpret_cast<int *>(propPID));
                XFree(propPID);
                return pid;
            }
        }
    }

    return -1;
}

//! Raise given window of the given display
void raiseWindow(Display *dpy, Window window)
{
    clientMsg(dpy, window, "_NET_ACTIVE_WINDOW", 0, 0, 0, 0, 0);    
    XSync(dpy, False);
}

/*!
 * \brief Return window id for the given binary.
 *
 * This method first fetches _NET_CLIENT_LIST for window candidates,
 * and then finds the matching binary using /proc/[pid]/cmdline.
 * Proc fs is primarily used, because we cannot trust WM_COMMAND window property
 * in all cases. Anyhow we check also WM_COMMAND because proc fs does not work
 * with scripts (python etc.).
 *
 * \param dpy The X11 display.
 * \param binaryName Full path to the binary.
 * \return Window id on success, 0 on failure.
 */
Window windowIdForBinary(Display *dpy, const char *binaryName)
{
    Window retValue = 0;
    if (dpy) 
    {
        Atom           netClientListAtom = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
        Atom           type;
        int            format;
        unsigned long  nItems = 0;
        unsigned long  bytesAfter;
        unsigned char *prop = 0;
        char           **wmCommand = NULL;
        int            wmCommandCount = 0;

        // Get the client list of the root window
        int status = XGetWindowProperty(dpy, XDefaultRootWindow(dpy), netClientListAtom,
                                        0, 0x7fffffff, False, XA_WINDOW,
                                        &type, &format, &nItems, &bytesAfter, &prop);

        if ((status == Success) && (prop != NULL))
        {
            Window * clients = reinterpret_cast<Window *>(prop);
            for (unsigned long i = 0; i < nItems; i++) 
            {
                if (binaryNameForPid(windowPid(dpy, clients[i])) == binaryName ||
                    (XGetCommand (dpy, clients[i], &wmCommand, &wmCommandCount) != 0 && 
                     wmCommandCount > 0 && strcmp(wmCommand[0], binaryName) == 0))
                {
                    retValue = clients[i];
                    break;
                }

                if (wmCommand) {
                    XFreeStringList(wmCommand);
                }
            }
            
            XFree(prop);
        }
    }
    
    return retValue;
}


// **** Export these functions when used as a library ****
extern "C"
{
    /*!
     * \brief Try to acquire a lock file.
     *
     * Tries to acquire a lock currently at
     * /var/run/single-instance-locks/[binaryName]/instance.lock
     *
     * \param binaryName Full path to the binary.
     * \return true if succeeded, false on failure.
     */
    DECL_EXPORT bool lock(const char * binaryName)
    {
        std::string path(LOCK_PATH_BASE + binaryName);
        if (!mkpath(path))
        {
            std::cerr << "ERROR!!: Couldn't create dir " <<
                    path << std::endl;

            return false;
        }

        path += std::string("/") + LOCK_FILE_NAME;

        struct flock fl;

        fl.l_type   = F_WRLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start  = 0;
        fl.l_len    = 1;

        if((g_lockFd = open(path.c_str(), O_WRONLY | O_CREAT, 0666)) == -1)
        {
            std::cerr << "ERROR!!: Couldn't create/open lock file '" <<
                    path << "' : " << strerror(errno) << std::endl;

            return false;
        }

        if(fcntl(g_lockFd, F_SETLK, &fl) == -1)
        {
            close(g_lockFd);
            return false;
        }
        return true;
    }

    //! Close the lock file acquired by lock()
    DECL_EXPORT void unlock()
    {
        if (g_lockFd != -1)
        {
            close(g_lockFd);
            g_lockFd = -1;
        }
    }

    //! Activate existing application 
    DECL_EXPORT bool activateExistingInstance(const char * binaryName)
    {
        if (Display * dpy = XOpenDisplay(NULL)) 
        {
            if (Window winId = windowIdForBinary(dpy, binaryName))
            {
                raiseWindow(dpy, winId);
                XCloseDisplay(dpy);
                return true;
            }
            else
            {
                std::cerr << "ERROR!!: Lock reserved but no window id for binary name found." << std::endl;
                XCloseDisplay(dpy);
                return false;
            }
        }
        else
        {
            std::cerr << "ERROR!!: Failed to open display!" << std::endl;
            return false;
        }
    }
}

//! The main function
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printHelp();
        return EXIT_FAILURE;
    }
    else if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")
    {
        printHelp();
        return EXIT_SUCCESS;
    }
    else
    {
        if (!lock(argv[1]))
        {
            if (!activateExistingInstance(argv[1]))
            {
                return EXIT_FAILURE;
            }
        }
        else
        {
            if (execve(argv[1], argv + 1, environ) == -1)
            {
                std::cerr << "ERROR!!: Failed to exec binary '" <<
                    argv[1] << "' : " << strerror(errno) << std::endl;
                unlock();
                
                return EXIT_FAILURE;
            }
        }
    }
    
    return EXIT_SUCCESS;
}

