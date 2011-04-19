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

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>

#include "preload.h"

typedef int (*entry_t)(int, char **);

int g_debugPrinting;

//! Load libraries from the given array
static void loadLibraries(const char * const libs[], unsigned int numLibs)
{
    for (unsigned int i = 0; i < numLibs; i++)
    {
        const char * lib = libs[i];
        int len = strlen(lib);
        if (lib[0] != '#' && len > 1)
        {
            int flags = 0;
            int skipChar = 0;

            // "now"
            if (lib[0] == 'N')
            {
                skipChar = 1;
                flags = RTLD_NOW | RTLD_GLOBAL;
            }
            // "lazy"
            else if (lib[0] == 'L')
            {
                skipChar = 1;
                flags = RTLD_LAZY | RTLD_GLOBAL;
            }
            // "deep"
            else if (lib[0] == 'D')
            {
                skipChar = 1;
                flags = RTLD_DEEPBIND | RTLD_GLOBAL;
            }
            // "default"
            else
            {
                skipChar = 0;
                flags =  RTLD_NOW | RTLD_GLOBAL;
            }

            // Open the library. Print possible errors only in debug mode.
            dlerror();

            // coverity[leaked_storage : FALSE]
            if (!dlopen(lib + skipChar, flags) && g_debugPrinting)
            {
                fprintf(stderr, "Warning: can't preload %s\n", lib + skipChar);
            }
        }
    }
}

/*! Invoke launcher daemon library and run main()
 *
 *  Note that argv must have enough dummy space so that
 *  the argument vector can be altered afterwards.
 *
 *  See e.g. Booster::renameProcess(int parentArgc, char** parentArgv)
 *           in launcherlib/booster.cpp
 */
static int invokeLauncherLib(int argc, char ** argv)
{
    // Clear any existing error
    dlerror();

    void * handle = dlopen(LAUNCHER_LIBRARY, RTLD_LAZY | RTLD_LOCAL);
    if (handle)
    {
        char * error = NULL;
        
        // Clear any existing error
        dlerror();
        
        // Find out address of main
        entry_t entry = (entry_t)dlsym(handle, "main");

        // Check error        
        if ((error = dlerror()) != NULL)
        {
            fprintf(stderr, "%s\n", error);
            dlclose(handle);
            return 0;
        }

        entry(argc, argv);
        dlclose(handle);

        return 1;
    }
    else
    {
        fprintf(stderr, "%s\n", dlerror());
        return 0;
    }
    
    return 1;
}

//! Entry point
int main(int argc, char ** argv)
{
    // Exit if DISPLAY is missing. This would result in dying
    // boosters and applauncherd would keep on re-starting them.
    if (!getenv("DISPLAY"))
    {
        fprintf(stderr, "FATAL!!: DISPLAY environment variable not set.\n");
        return EXIT_FAILURE;
    }

    // Parse command line
    g_debugPrinting = 0;

    int helpWanted = 0;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--debug") == 0)
            g_debugPrinting = 1;

        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
            helpWanted = 1;
    }

    // Set environment. Because applauncherd is usually a privileged
    // process, TMPDIR variable might be unset by the C library. In
    // case it's not set, we set it to /var/tmp, which usually is not
    // a RAM disk.
    setenv("TMPDIR", "/var/tmp", 0);

    // Preload libraries
    if (!helpWanted)
        loadLibraries(gLibs, sizeof(gLibs) / sizeof(char *));

    // Start the real applauncherd.
    if (!invokeLauncherLib(argc, argv))
    {
        fprintf(stderr, "FATAL!!: Failed to load the launcher library\n");
        return EXIT_FAILURE;
    }
   
    return EXIT_SUCCESS;
}
