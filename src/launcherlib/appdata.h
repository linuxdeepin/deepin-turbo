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

#ifndef APPDATA_H
#define APPDATA_H

#ifdef HAVE_CREDS
    #include <sys/creds.h>
#endif

#include <stdint.h>

#include <string>

using std::string;

#include <vector>

using std::vector;

typedef int (*entry_t)(int, char **);

//! Structure for application data read from the invoker
class AppData
{
public:

    //! Constructor
    AppData();

    //! Destructor
    ~AppData();

    //! Set options
    void setOptions(uint32_t options);

    //! Return options
    int options() const;

    //! Return whether or not RTLD_GLOBAL should be used in dlopen
    bool dlopenGlobal() const;

    //! Return whether or not RTLD_DEEPBIND should be used in dlopen
    bool dlopenDeep() const;

    //! Return whether or not application should be launched as a single instance application
    bool singleInstance() const;

    //! Return whether or not disable default out of memory killing adjustments for application process 
    bool disableOutOfMemAdj() const;

    //! Set argument count
    void setArgc(int argc);

    //! Return argument count
    int argc() const;

    //! Set address of the argument vector
    void setArgv(const char ** argv);

    //! Return address of the argument vector
    const char ** argv() const;

    //! Set application name
    void setAppName(const string & appName);

    //! Return application name
    const string & appName() const;

    //! Set file name
    void setFileName(const string & fileName);

    //! Return file name
    const string & fileName() const;

    //! Set file name of the image shown as splash screen
    void setSplashFileName(const string & fileName);

    //! Return file name of the image shown as splash screen
    const string & splashFileName() const;

    //! Set file name of the image shown as landscape splash screen
    void setLandscapeSplashFileName(const string & fileName);

    //! Return file name of the image shown as landscape splash screen
    const string & landscapeSplashFileName() const;

    //! Set priority
    void setPriority(int priority);

    //! Return priority
    int priority() const;

    //! Set booster respawn delay
    void setDelay(int delay);

    //!Return respawn delay
    int delay() const;

    //! Set entry point for the application
    void setEntry(entry_t entry);

    //! Get the entry point
    entry_t entry() const;

    //! Get I/O descriptors
    const vector<int> & ioDescriptors() const;

    //! Set I/O descriptors
    void setIODescriptors(const vector<int> & ioDescriptors);

    //! Set user ID and group ID of calling process
    void setIDs(uid_t userId, gid_t groupId);

    //! Get user ID of calling process
    uid_t userId() const;

    //! Get group ID of calling process
    gid_t groupId() const;

    //! Frees the memory reserved for argv
    void deleteArgv();

#if defined (HAVE_CREDS)
    //! Store security credentials
    void setPeerCreds(creds_t peerCreds);

    //! Get the stored credentials
    creds_t peerCreds() const;
    
    //! Free the memory reserved for credentials
    void deletePeerCreds();
#endif

private:

    AppData(const AppData & r);
    AppData & operator= (const AppData & r);

    uint32_t    m_options;
    int         m_argc;
    const char ** m_argv;
    string      m_appName;
    string      m_fileName;
    int         m_prio;
    int         m_delay;
    entry_t     m_entry;
    vector<int> m_ioDescriptors;
    gid_t       m_gid;
    uid_t       m_uid;
    string      m_splashFileName;
    string      m_landscapeSplashFileName;

#if defined (HAVE_CREDS)
    creds_t     m_peerCreds;
#endif

};

#endif // APPDATA_H
