/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of applifed.
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

#include <MLayout>
#include <MLabel>
#include <MGridLayoutPolicy>
#include <MDebug>

#include "mainpage.h"

MainPage::MainPage(QString windowName) :
    m_windowName(windowName)
{
    setTitle(windowName);

    m_layout = new MLayout(centralWidget());
    m_policy = new MGridLayoutPolicy(m_layout);
    m_policy->setSpacing(20.0);

    m_label = new MLabel(m_windowName);
    m_label->setAlignment(Qt::AlignCenter);
    m_policy->addItem(m_label, 0, 0);

}

MainPage::~MainPage()
{
    delete m_policy;
    m_policy = NULL;
}

void MainPage::createContent()
{}


