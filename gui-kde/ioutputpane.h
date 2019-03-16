/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/
#pragma once


#include <QObject>
#include <QVector>
#include <QString>

class QWidget;

namespace KV {

class IOutputPane: public QObject
{
    Q_OBJECT

public:
    IOutputPane(QObject *parent = nullptr);
    ~IOutputPane() override;

    virtual QWidget *outputWidget(QWidget *parent) = 0;
    virtual QVector<QWidget*> toolBarWidgets() const = 0;
    virtual QString displayName() const = 0;

    virtual int priorityInStatusBar() const = 0;

    virtual void clearContents() = 0;
    virtual void refreshContents() = 0;
    virtual void configureOutput() = 0;
    virtual void visibilityChanged(bool visible) = 0;

    virtual bool canConfigure() const = 0;
    virtual bool canRefresh() const = 0;

public slots:

    void popup() {emit showPage();}

    void hide() {emit hidePage();}
    void toggle() {emit togglePage();}
    void flash() {emit flashButton();}
    void setIconBadgeNumber(int number) {emit setBadgeNumber(number);}

signals:
    void showPage();
    void hidePage();
    void togglePage();
    void navigateStateUpdate();
    void flashButton();
    void setBadgeNumber(int number);
};

} // namespace KV

