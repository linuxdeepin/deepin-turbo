/***************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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

#include "ut_boosterpluginregistry.h"
#include "boosterpluginregistry.h"
#include <dlfcn.h>

Ut_BoosterPluginRegistry::Ut_BoosterPluginRegistry()
{}

Ut_BoosterPluginRegistry::~Ut_BoosterPluginRegistry()
{}

void Ut_BoosterPluginRegistry::initTestCase()
{}

void Ut_BoosterPluginRegistry::cleanupTestCase()
{}

void Ut_BoosterPluginRegistry::testValidateAndRegisterPlugin()
{
    void * handle = NULL;
    //positive
    handle = dlopen(TEST_LIB_PLUGIN_PATH, RTLD_NOW);
    QVERIFY(handle);
    char pluginType = BoosterPluginRegistry::validateAndRegisterPlugin(handle);
    QVERIFY(pluginType == 'z');

    //negative invalid handle
    handle = NULL;
    QVERIFY(BoosterPluginRegistry::validateAndRegisterPlugin(handle) == false);

    //negative no "type" symbol in the plugin (only create)
    handle = dlopen(TEST_LIB_CREATE_PATH, RTLD_NOW);
    QVERIFY(handle);
    QVERIFY(BoosterPluginRegistry::validateAndRegisterPlugin(handle) == false);

    //negative no "socketName" symbol in the plugin (only create and type)
    handle = dlopen(TEST_LIB_CREATETYPE_PATH, RTLD_NOW);
    QVERIFY(handle);
    QVERIFY(BoosterPluginRegistry::validateAndRegisterPlugin(handle) == false);

    //negative no "temporaryProcessName" symbol in the plugin (only create, type and sockeName)
    handle = dlopen(TEST_LIB_CREATETYPESOCKETNAME_PATH, RTLD_NOW);
    QVERIFY(handle);
    QVERIFY(BoosterPluginRegistry::validateAndRegisterPlugin(handle) == false);
}

void Ut_BoosterPluginRegistry::testRegisterPlugin()
{

    void * handle = dlopen(TEST_LIB_PLUGIN_PATH, RTLD_NOW);
    QVERIFY(handle);
    dlerror();

    create_func_t createFunc = (create_func_t)dlsym(handle, "create");
    QVERIFY(dlerror() == NULL);

    sn_func_t socketNameFunc = (sn_func_t)dlsym(handle, "socketName");
    QVERIFY(dlerror() == NULL);

    tpn_func_t temporaryProcessNameFunc = (tpn_func_t)dlsym(handle, "temporaryProcessName");
    QVERIFY(dlerror() == NULL);

    BoosterPluginRegistry::registerPlugin('0', createFunc, socketNameFunc, temporaryProcessNameFunc); //register new plugin with type '0'
    BoosterPluginEntry * pluginEntry = BoosterPluginRegistry::pluginEntry('0');
    QVERIFY(pluginEntry);
    QVERIFY(pluginEntry->type == '0');
}

void Ut_BoosterPluginRegistry::testPluginEntry()
{
    //positive
    void * handle = dlopen(TEST_LIB_PLUGIN_PATH, RTLD_NOW);
    QVERIFY(handle);
    BoosterPluginRegistry::validateAndRegisterPlugin(handle);
    BoosterPluginEntry * pluginEntry = BoosterPluginRegistry::pluginEntry('z');
    QVERIFY(pluginEntry);
    QVERIFY(pluginEntry->type == 'z');

    //negative with negative index for int type constructor
    QVERIFY(BoosterPluginRegistry::pluginEntry(-100) == false);

    //negative with index larger then registered plugins count
    int index;
    index = BoosterPluginRegistry::pluginCount() + 1;
    QVERIFY(BoosterPluginRegistry::pluginEntry(index) == false);

    //negative with unregistered plugin type
    QVERIFY(BoosterPluginRegistry::pluginEntry('1') == false);
}


QTEST_APPLESS_MAIN(Ut_BoosterPluginRegistry)
