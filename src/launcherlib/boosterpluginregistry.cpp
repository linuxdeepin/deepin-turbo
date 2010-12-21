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

#include "boosterpluginregistry.h"

#include <dlfcn.h>

BoosterPluginRegistry::RegistryType BoosterPluginRegistry::m_registry;

char BoosterPluginRegistry::validateAndRegisterPlugin(void * handle)
{
    // Check if create() is there
    dlerror();
    create_func_t create = (create_func_t)dlsym(handle, "create");

    // Check error
    char * error = NULL;
    if ((error = dlerror()) != NULL)
    {
        return 0;
    }

    // Check if type() is there
    dlerror();
    type_func_t type = (type_func_t)dlsym(handle, "type");

    // Check error
    if ((error = dlerror()) != NULL)
    {
        return 0;
    }

    // Check if socketName() is there
    dlerror();
    sn_func_t socketName = (sn_func_t)dlsym(handle, "socketName");

    // Check error
    if ((error = dlerror()) != NULL)
    {
        return 0;
    }

    // Check if temporaryProcessName() is there
    dlerror();
    tpn_func_t temporaryProcessName = (tpn_func_t)dlsym(handle, "temporaryProcessName");

    // Check error
    if ((error = dlerror()) != NULL)
    {
        return 0;
    }

    registerPlugin(type(), create, socketName, temporaryProcessName);
    return type();
}

void BoosterPluginRegistry::registerPlugin(char type,
                                           create_func_t createFunc,
                                           sn_func_t socketNameFunc,
                                           tpn_func_t temporaryProcessNameFunc)

{
    BoosterPluginEntry * entry = pluginEntry(type);
    if (!entry)
    {
        entry = new BoosterPluginEntry;
        m_registry << shared_ptr<BoosterPluginEntry>(entry);
    }

    entry->type = type;
    entry->createFunc = createFunc;
    entry->socketNameFunc = socketNameFunc;
    entry->temporaryProcessNameFunc = temporaryProcessNameFunc;
}

int BoosterPluginRegistry::pluginCount()
{
    return m_registry.length();
}

BoosterPluginEntry * BoosterPluginRegistry::pluginEntry(int index)
{
    if (index < 0 || index >= m_registry.size())
    {
        return NULL;
    }

    return m_registry.at(index).get();
}

BoosterPluginEntry * BoosterPluginRegistry::pluginEntry(char type)
{
    BoosterPluginRegistry::RegistryType::const_iterator i = m_registry.begin();
    while (i != m_registry.end())
    {
        if ((*i).get()->type == type)
        {
            return (*i).get();
        }

        i++;
    }

    return NULL;
}
