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
#include <cstdlib>

using std::atoi;

#include <iostream>

using std::cout;
using std::endl;

#include <list>

using std::list;

class WindowsMatchingPid
{
public:

    WindowsMatchingPid(Display *display, Window wRoot, unsigned long pid)
        : m_display(display)
        , m_pid(pid)
    {
        // Get the PID property atom.
        m_atomPID = XInternAtom(display, "_NET_WM_PID", False);
        search(wRoot);
    }

    const list<Window> &result() const { return m_result; }

private:

    Display       *m_display;
    unsigned long  m_pid;
    Atom           m_atomPID;
    list<Window>   m_result;

    void search(Window w)
    {
        // Get the PID for the current Window.
        Atom           type;
        int            format;
        unsigned long  nItems;
        unsigned long  bytesAfter;
        unsigned char *propPID = 0;
        
        if(Success == XGetWindowProperty(m_display, w, m_atomPID, 0, 1, False, XA_CARDINAL,
                                         &type, &format, &nItems, &bytesAfter, &propPID))
        {
            if(propPID != 0)
            {
                // If the PID matches, add this window to the result set.
                if(m_pid == *((unsigned long *)propPID))
                    m_result.push_back(w);

                XFree(propPID);
            }
        }

        // Recurse into child windows.
        Window    wRoot;
        Window    wParent;
        Window   *wChild;
        unsigned  numChildren;
        
        if(XQueryTree(m_display, w, &wRoot, &wParent, &wChild, &numChildren) != 0)
        {
            for(unsigned i = 0; i < numChildren; i++)
                search(wChild[i]);
        }
    }
};

int main(int argc, char **argv)
{
    if(argc < 2) return EXIT_FAILURE;

    const int pid = atoi(argv[1]);

    // Start with the root window.
    Display *display = XOpenDisplay(0);

    WindowsMatchingPid match(display, XDefaultRootWindow(display), pid);

    // Print the resulting window id's
    const list<Window> &result = match.result();
    for(list<Window>::const_iterator it = result.begin(); it != result.end(); it++)
        cout << "0x" << std::hex << (unsigned long)(*it) << std::dec << endl;

    return EXIT_SUCCESS;
}

