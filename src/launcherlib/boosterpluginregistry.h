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

#ifndef BOOSTERPLUGINREGISTRY_H
#define BOOSTERPLUGINREGISTRY_H

#include <vector>

using std::vector;

#include <tr1/memory>

using std::tr1::shared_ptr;

// Function pointer type for Booster::type()
typedef char (*type_func_t)();

// Function pointer type for Booster::create()
typedef void * (*create_func_t)();

// Function pointer type for Booster::socketName()
typedef const char * (*sn_func_t)();

// Function pointer type for Booster::temporaryProcessName()
typedef const char * (*tpn_func_t)();

//! Item in the registry
struct BoosterPluginEntry
{
    //! Booster type
    char type;

    //! Create function
    create_func_t createFunc;

    //! Socket name function
    sn_func_t socketNameFunc;

    //! Temporary process name function
    tpn_func_t temporaryProcessNameFunc;
};

/*! \class BoosterPluginRegistry
 *  \brief BoosterPluginRegistry holds needed data for registered plugins.
 */
class BoosterPluginRegistry
{
public:

    /*! Validate given plugin library handle and register to
     *  BoosterPluginRegistry. Returns the type of the registered
     *  plugin or 0 if failed.
     */
    static char validateAndRegisterPlugin(void * handle);

    //! Return number of registered plugins
    static int pluginCount();

    //! Return pointer to given booster plugin. NULL if not found.
    static BoosterPluginEntry * pluginEntry(int index);

    //! Return pointer to given booster plugin. NULL if not found.
    static BoosterPluginEntry * pluginEntry(char type);

private:

    //! Hidden constructor
    BoosterPluginRegistry() {};

    //! Register a booster plugin
    static void registerPlugin(char type,
                               create_func_t createFunc,
                               sn_func_t socketNameFunc,
                               tpn_func_t temporaryProcessNameFunc);

    //! Registry
    typedef vector<shared_ptr<BoosterPluginEntry> > RegistryType;
    static RegistryType m_registry;

#ifdef UNIT_TEST
    friend class Ut_BoosterPluginRegistry;
#endif
};

#endif // BOOSTERPLUGINREGISTRY_H
