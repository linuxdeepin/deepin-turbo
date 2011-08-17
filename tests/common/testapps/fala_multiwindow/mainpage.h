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

#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <MApplicationPage>
#include <QString>

class MLabel;
class MLayout;
class MGridLayoutPolicy;

class MainPage : public MApplicationPage
{
    Q_OBJECT

public:
    MainPage(QString windowName);
    virtual ~MainPage();
    virtual void createContent();

private:
    QString              m_windowName;
    MLabel             * m_label;
    MLayout            * m_layout;
    MGridLayoutPolicy  * m_policy;
};

#endif // MAINPAGE_H
