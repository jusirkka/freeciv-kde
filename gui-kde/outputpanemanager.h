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

#include <QToolButton>
#include <QFrame>

class QAction;
class QLabel;
class QStackedWidget;
class QTimeLine;

namespace KV {

class IOutputPane;

class MainWindow;
class OutputPaneToggleButton;
class OutputPaneManageButton;
class OutputPaneResizeButton;

class OutputPaneData
{
public:
    OutputPaneData(IOutputPane *pane = nullptr) : pane(pane) {}

    IOutputPane *pane = nullptr;
    QString id;
    OutputPaneToggleButton *button = nullptr;
    QAction *action = nullptr;
    bool buttonVisible = false;
};


class OutputPaneManager : public QWidget
{
    Q_OBJECT

public:

  void updateStatusButtons(bool visible);

private:

  using Panes = QVector<IOutputPane*>;

  explicit OutputPaneManager(const Panes& panes, MainWindow *parent = nullptr);
  ~OutputPaneManager() override = default;

  friend class MainWindow;
  friend class OutputPaneManageButton;

  void shortcutTriggered(int idx);
  void clearPane();
  void popupMenu();
  void showPane(int idx);
  int currentIndex() const;
  void setCurrentIndex(int idx);
  void buttonTriggered(int idx);
  void hidePane();
  void configOutput();
  void refreshOutput();

  QLabel *m_titleLabel = nullptr;
  OutputPaneManageButton *m_manageButton = nullptr;

  QAction *m_clearAction = nullptr;
  QToolButton *m_clearButton = nullptr;

  OutputPaneResizeButton *m_resizeButton = nullptr;

  QAction *m_configAction = nullptr;
  QToolButton *m_configButton = nullptr;

  QAction *m_refreshAction = nullptr;
  QToolButton *m_refreshButton = nullptr;

  QWidget *m_toolBar = nullptr;

  QStackedWidget *m_outputWidgetPane = nullptr;
  QStackedWidget *m_opToolBarWidgets = nullptr;

  QWidget *m_buttonsWidget = nullptr;

  MainWindow* m_parent;

  QVector<OutputPaneData> m_outputPanes;

};

class BadgeLabel
{
public:
    BadgeLabel();
    void paint(QPainter *p, int x, int y, bool isChecked, const QPalette& pal);
    void setText(const QString &text);
    QString text() const;
    QSize sizeHint() const;

private:
    void calculateSize();

    QSize m_size;
    QString m_text;
    QFont m_font;
    static const int m_padding = 6;
};

class OuputPaneSeparator: public QFrame
{
  Q_OBJECT
public:
  OuputPaneSeparator(QWidget* parent = nullptr);
};

class OutputPaneToggleButton : public QToolButton
{
    Q_OBJECT
public:
    OutputPaneToggleButton(int number, const QString &text, QAction *action,
                           QWidget *parent = nullptr);
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent*) override;
    void flash(int count = 3);
    void setIconBadgeNumber(int number);

private:
    void updateToolTip();
    void checkStateSet() override;

    QString m_number;
    QString m_text;
    QAction *m_action;
    QTimeLine *m_flashTimer;
    BadgeLabel m_badgeNumberLabel;
};

class OutputPaneManageButton : public QToolButton
{
    Q_OBJECT
public:
    OutputPaneManageButton();
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent*) override;
};

class OutputPaneResizeButton : public QToolButton
{
  Q_OBJECT
public:
  OutputPaneResizeButton(QWidget* target, QWidget* parent = nullptr);
protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
private:

  QWidget* m_target;
  bool m_resizing = false;
  QSize m_anchorSize;
  QPoint m_anchor;
  QPoint m_anchorPos;
};

}
