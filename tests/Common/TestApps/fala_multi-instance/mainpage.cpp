/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of libmeegotouch.
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
#include <MApplicationWindow>
#include <iostream>

#include "mainpage.h"


MainPage::MainPage()
{
}

MainPage::~MainPage()
{
}

void MainPage::setName(QString name)
{
   myName = name;
}

void MainPage::createContent()
{
    MLayout *layout = new MLayout(centralWidget());
    MGridLayoutPolicy *policy = new MGridLayoutPolicy(layout);
    policy->setSpacing(20.0);

    MLabel *label1 = new MLabel(myName);
    label1->setAlignment(Qt::AlignCenter);
    policy->addItem(label1, 1, 1);

    layout->setLandscapePolicy(policy);
    layout->setPortraitPolicy(policy);
}

void MainPage::exitDisplayEvent()
{
    std::cerr << myName.toStdString() << " MainPage hidden" << std::endl;
}

void MainPage::enterDisplayEvent()
{
    std::cerr << myName.toStdString() << " MainPage visible" << std::endl;
}

