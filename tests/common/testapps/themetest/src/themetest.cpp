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

#include <MApplication>
#include <MApplicationPage>
#include <MApplicationWindow>
#include <MLabel>
#include <MButton>
#include <QDebug>
#include <MLinearLayoutPolicy>
#include <MLayout>
#include <MTheme>


#include <mcomponentcache.h>

extern "C" __attribute__((visibility("default"))) int main(int, char**);

int main(int argc, char ** argv)
{
    MApplication *app = MComponentCache::mApplication(argc, argv, "fala_ft_themetest");
    MApplicationWindow *window = MComponentCache::mApplicationWindow();
    MApplicationPage page;
    page.setTitle("fala_ft_themetest");

    MLayout *layout = new MLayout;
    MLinearLayoutPolicy *policy = new MLinearLayoutPolicy(layout, Qt::Vertical);
    policy->setSpacing(10);

    MLabel *label = new MLabel(QString("Hello, themed world!"));
    label->setObjectName("themedlabel");
    label->setAlignment(Qt::AlignCenter);
    policy->addItem(label);

    label = new MLabel(QString("With sheep"));
    label->setObjectName("themedlabelwithgraphics");
    label->setAlignment(Qt::AlignLeft);
    policy->addItem(label);

    page.centralWidget()->setLayout(layout);
    page.appear();
    window->show();
    _exit(app->exec());
}
