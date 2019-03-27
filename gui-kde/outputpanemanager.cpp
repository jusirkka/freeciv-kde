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

#include "outputpanemanager.h"
#include "ioutputpane.h"
#include "application.h"
#include "mainwindow.h"
#include "logging.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QStyle>
#include <QStackedWidget>
#include <QToolButton>
#include <QTimeLine>
#include <QToolBar>
#include <QStatusBar>

using namespace KV;


IOutputPane::IOutputPane(QObject *parent)
    : QObject(parent)
{}

IOutputPane::~IOutputPane()
{}

const int buttonBorderWidth = 3;
const int numberAreaWidth = 19;


// Return shortcut as Alt+<number> if number is a non-zero digit
static QKeySequence paneShortCut(int number)
{
  if (number < 1 || number > 9) return QKeySequence();
  return QKeySequence(Qt::ALT | (Qt::Key_0 + number));
}




OutputPaneManager::OutputPaneManager(const Panes& panes, MainWindow *parent)
  : QWidget(parent)
  , m_titleLabel(new QLabel)
  , m_manageButton(new OutputPaneManageButton)
  , m_outputWidgetPane(new QStackedWidget)
  , m_opToolBarWidgets(new QStackedWidget)
  , m_parent(parent)
{
  setWindowFlag(Qt::WindowStaysOnTopHint);

  for (auto pane: panes) {
    m_outputPanes.append(OutputPaneData(pane));
  }

  setWindowTitle(tr("Output"));

  m_titleLabel->setContentsMargins(5, 0, 5, 0);

  m_clearAction = new QAction(this);
  m_clearAction->setIcon(QIcon::fromTheme("edit-clear"));
  m_clearAction->setText(tr("Clear"));
  connect(m_clearAction, &QAction::triggered, this, &OutputPaneManager::clearPane);

  m_configAction = new QAction(this);
  m_configAction->setIcon(QIcon::fromTheme("settings-configure"));
  m_configAction->setText(tr("Configure"));
  connect(m_configAction, &QAction::triggered, this, &OutputPaneManager::configOutput);

  m_refreshAction = new QAction(this);
  m_refreshAction->setIcon(QIcon::fromTheme("view-refresh"));
  m_refreshAction->setText(tr("Refresh view"));
  connect(m_refreshAction, &QAction::triggered, this, &OutputPaneManager::refreshOutput);



  auto toolLayout = new QHBoxLayout;
  toolLayout->setMargin(0);
  toolLayout->setSpacing(0);
  toolLayout->addWidget(m_titleLabel);
  toolLayout->addSpacing(200);

  toolLayout->addWidget(m_opToolBarWidgets);

  m_refreshButton = new QToolButton;
  m_refreshButton->setDefaultAction(m_refreshAction);
  toolLayout->addWidget(m_refreshButton);

  m_configButton = new QToolButton;
  m_configButton->setDefaultAction(m_configAction);
  toolLayout->addWidget(m_configButton);

  m_clearButton = new QToolButton;
  m_clearButton->setDefaultAction(m_clearAction);
  toolLayout->addWidget(m_clearButton);

  m_resizeButton = new OutputPaneResizeButton(this);
  m_resizeButton->setIcon(Application::Icon("resize"));
  toolLayout->addWidget(m_resizeButton);

  m_toolBar = new QWidget;
  m_toolBar->setLayout(toolLayout);
  auto mainlayout = new QVBoxLayout;
  mainlayout->setSpacing(0);
  mainlayout->setMargin(0);
  mainlayout->addWidget(m_toolBar);
  mainlayout->addWidget(m_outputWidgetPane, 10);
  setLayout(mainlayout);

  m_buttonsWidget = new QWidget;
  m_buttonsWidget->setLayout(new QHBoxLayout);
  m_buttonsWidget->layout()->setContentsMargins(5,0,0,0);
  m_buttonsWidget->layout()->setSpacing(4);

  m_parent->statusBar()->addWidget(m_buttonsWidget);


  QFontMetrics titleFm = m_titleLabel->fontMetrics();
  int minTitleWidth = 0;

  std::sort(m_outputPanes.begin(), m_outputPanes.end(),
            [](const OutputPaneData &d1, const OutputPaneData &d2) {
    return d1.pane->priorityInStatusBar() > d2.pane->priorityInStatusBar();
  });

  const int n = m_outputPanes.size();

  int shortcutNumber = 1;
  QString baseId("panes.");
  for (int i = 0; i < n; ++i) {
    OutputPaneData &data = m_outputPanes[i];
    IOutputPane* outPane = data.pane;
    const int idx = m_outputWidgetPane->addWidget(outPane->outputWidget(this));
    connect(outPane, &IOutputPane::showPage, this, [this, idx]() {
      showPane(idx);
    });
    connect(outPane, &IOutputPane::hidePage, this, &OutputPaneManager::hidePane);

    connect(outPane, &IOutputPane::togglePage, this, [this, idx]() {
      if (isVisible() && currentIndex() == idx)
        hidePane();
      else
        showPane(idx);
    });

    QWidget *toolButtonsContainer = new QWidget(m_opToolBarWidgets);
    auto toolButtonsLayout = new QHBoxLayout;
    toolButtonsLayout->setMargin(0);
    toolButtonsLayout->setSpacing(0);
    for (QWidget *toolButton: outPane->toolBarWidgets()) {
      toolButtonsLayout->addWidget(toolButton);
    }
    toolButtonsLayout->addStretch(5);
    toolButtonsContainer->setLayout(toolButtonsLayout);

    m_opToolBarWidgets->addWidget(toolButtonsContainer);

    minTitleWidth = qMax(minTitleWidth, titleFm.width(outPane->displayName()));

    QString suffix = outPane->displayName().simplified();
    suffix.remove(QLatin1Char(' '));
    data.id = baseId + suffix;
    data.action = new QAction(outPane->displayName(), this);
    data.action->setShortcut(paneShortCut(idx));
    auto button = new OutputPaneToggleButton(shortcutNumber, outPane->displayName(),
                                             data.action);
    data.button = button;

    connect(outPane, &IOutputPane::flashButton, button, [button] {button->flash(); });
    connect(outPane, &IOutputPane::setBadgeNumber,
            button, &OutputPaneToggleButton::setIconBadgeNumber);

    ++shortcutNumber;
    m_buttonsWidget->layout()->addWidget(data.button);
    connect(data.button, &QAbstractButton::clicked, this, [this, i] {
      buttonTriggered(i);
    });

    bool visible = outPane->priorityInStatusBar() != -1;
    data.button->setVisible(visible);
    data.buttonVisible = visible;

    connect(data.action, &QAction::triggered, this, [this, i] {
      buttonTriggered(i);
    });
  }

  m_titleLabel->setMinimumWidth(minTitleWidth + m_titleLabel->contentsMargins().left()
                                + m_titleLabel->contentsMargins().right());
  m_buttonsWidget->layout()->addWidget(m_manageButton);
  connect(m_manageButton, &QAbstractButton::clicked, this, &OutputPaneManager::popupMenu);

  setMinimumHeight(50);
  setMinimumWidth(300);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

  hidePane();
}

void OutputPaneManager::paintEvent(QPaintEvent *)
{
  QStyleOption opt;
  opt.init(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}


void OutputPaneManager::updateStatusButtons(bool visible)
{
    int idx = currentIndex();
    if (idx == -1) return;
    const OutputPaneData &data = m_outputPanes.at(idx);
    data.button->setChecked(visible);
    data.pane->visibilityChanged(visible);
}


void OutputPaneManager::buttonTriggered(int idx)
{
  if (idx == currentIndex() && isVisible()) {
    hidePane();
  } else {
    showPane(idx);
  }
}


void OutputPaneManager::hidePane()
{
  setVisible(false);
  int idx = currentIndex();
  if (idx >= 0) {
    m_outputPanes.at(idx).button->setChecked(false);
    m_outputPanes.at(idx).pane->visibilityChanged(false);
  }
}


void OutputPaneManager::showPane(int idx)
{
  setCurrentIndex(idx);
  setVisible(true);
  raise();
}

void OutputPaneManager::setCurrentIndex(int idx)
{
    static int lastIndex = -1;

    if (lastIndex != -1) {
        m_outputPanes.at(lastIndex).button->setChecked(false);
        m_outputPanes.at(lastIndex).pane->visibilityChanged(false);
    }

    if (idx != -1) {
        m_outputWidgetPane->setCurrentIndex(idx);
        m_opToolBarWidgets->setCurrentIndex(idx);

        m_outputPanes.at(idx).button->setChecked(true);
        m_outputPanes.at(idx).pane->visibilityChanged(true);

        auto pane = m_outputPanes.at(idx).pane;
        m_configAction->setEnabled(pane->canConfigure());
        m_refreshAction->setEnabled(pane->canRefresh());
        m_titleLabel->setText(pane->displayName());
    }

    lastIndex = idx;
}

void OutputPaneManager::popupMenu()
{
    QMenu menu;
    int idx = 0;
    for (OutputPaneData &data : m_outputPanes) {
        QAction *act = menu.addAction(data.pane->displayName());
        act->setCheckable(true);
        act->setChecked(data.buttonVisible);
        act->setData(idx);
        ++idx;
    }
    QAction *result = menu.exec(QCursor::pos());
    if (!result) return;
    idx = result->data().toInt();
    OutputPaneData &data = m_outputPanes[idx];
    if (data.buttonVisible) {
        data.pane->visibilityChanged(false);
        data.button->setChecked(false);
        data.button->hide();
        data.buttonVisible = false;
    } else {
        data.button->show();
        data.buttonVisible = true;
        showPane(idx);
    }
}


void OutputPaneManager::clearPane() {
    int idx = currentIndex();
    if (idx >= 0) m_outputPanes.at(idx).pane->clearContents();
}

void OutputPaneManager::refreshOutput() {
    int idx = currentIndex();
    if (idx >= 0) m_outputPanes.at(idx).pane->refreshContents();
}

void OutputPaneManager::configOutput() {
    int idx = currentIndex();
    if (idx >= 0) m_outputPanes.at(idx).pane->configureOutput();
}

int OutputPaneManager::currentIndex() const
{
    return m_outputWidgetPane->currentIndex();
}


// OutputPaneToolButton

OutputPaneToggleButton::OutputPaneToggleButton(int number, const QString &text,
                                               QAction *action, QWidget *parent)
  : QToolButton(parent)
  , m_number(QString::number(number))
  , m_text(text)
  , m_action(action)
  , m_flashTimer(new QTimeLine(1000, this))
  , m_flashColor(Qt::red)
{
  setFocusPolicy(Qt::NoFocus);
  setCheckable(true);
  QFont fnt = QApplication::font();
  setFont(fnt);
  if (m_action) {
    connect(m_action, &QAction::changed, this, &OutputPaneToggleButton::updateToolTip);
  }
  m_flashTimer->setDirection(QTimeLine::Forward);
  m_flashTimer->setCurveShape(QTimeLine::SineCurve);
  m_flashTimer->setFrameRange(0, 92);
  auto updateSlot = static_cast<void (QWidget::*)()>(&QWidget::update);
  connect(m_flashTimer, &QTimeLine::valueChanged, this, updateSlot);
  connect(m_flashTimer, &QTimeLine::finished, this, [=] () {
    if (!isChecked()) m_unseen = true;
    update();
  });
  updateToolTip();
}

void OutputPaneToggleButton::updateToolTip()
{
  setToolTip(m_action->toolTip());
}


QSize OutputPaneToggleButton::sizeHint() const
{
    ensurePolished();

    QSize s = fontMetrics().size(Qt::TextSingleLine, m_text);

    // Expand to account for border image
    s.rwidth() += numberAreaWidth + 1 + buttonBorderWidth + buttonBorderWidth;

    if (!m_badgeNumberLabel.text().isNull())
        s.rwidth() += m_badgeNumberLabel.sizeHint().width() + 1;

    return s.expandedTo(QApplication::globalStrut());
}

void OutputPaneToggleButton::paintEvent(QPaintEvent*)
{
  const QFontMetrics fm = fontMetrics();
  const int baseLine = (height() - fm.height() + 1) / 2 + fm.ascent();
  const int numberWidth = fm.width(m_number);

  QPainter p(this);

  QRect r = rect().adjusted(numberAreaWidth, 1, -1, -1);

  if (isChecked()) {
    p.fillRect(r, palette().color(QPalette::Active, QPalette::Background));
  }

  if (m_flashTimer->state() == QTimeLine::Running) {
    m_flashColor.setAlpha(m_flashTimer->currentFrame());
    p.fillRect(r, m_flashColor);
  } else if (m_unseen) {
    m_flashColor.setAlpha(92);
    p.fillRect(r, m_flashColor);
  }


  p.setFont(font());
  p.setPen(palette().color(QPalette::Active, QPalette::Text));
  p.drawText((numberAreaWidth - numberWidth) / 2, baseLine, m_number);
  if (!isChecked()) {
    p.setPen(palette().color(QPalette::Inactive, QPalette::Text));
  }
  int leftPart = numberAreaWidth + buttonBorderWidth;
  int labelWidth = 0;
  if (!m_badgeNumberLabel.text().isEmpty()) {
    const QSize labelSize = m_badgeNumberLabel.sizeHint();
    labelWidth = labelSize.width() + 3;
    m_badgeNumberLabel.paint(&p, width() - labelWidth,
                             (height() - labelSize.height()) / 2,
                             isChecked(), palette());
  }
  p.drawText(leftPart, baseLine, fm.elidedText(m_text, Qt::ElideRight, width() - leftPart - 1 - labelWidth));
}

void OutputPaneToggleButton::checkStateSet()
{
  // Stop flashing when button is checked
  m_flashTimer->stop();
  m_unseen = false;
  QToolButton::checkStateSet();
}

void OutputPaneToggleButton::flash(int count)
{
  // Start flashing if button is not checked
  if (!isChecked()) {
    m_flashTimer->setLoopCount(count);
    if (m_flashTimer->state() != QTimeLine::Running) {
      m_flashColor = QColor(Qt::red);
      m_flashTimer->start();
    }
    update();
  }
}

void OutputPaneToggleButton::setIconBadgeNumber(int number)
{
    QString text = (number ? QString::number(number) : QString());
    m_badgeNumberLabel.setText(text);
    updateGeometry();
}

OutputPaneManageButton::OutputPaneManageButton(QWidget *p)
  : QToolButton(p)
{
  setFocusPolicy(Qt::NoFocus);
  setCheckable(true);
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
}

QSize OutputPaneManageButton::sizeHint() const
{
  ensurePolished();
  return QSize(numberAreaWidth, QApplication::globalStrut().height());
}

void OutputPaneManageButton::paintEvent(QPaintEvent*)
{
  QPainter p(this);
  QStyle *s = style();
  QStyleOption arrowOpt;
  arrowOpt.initFrom(this);
  arrowOpt.rect = QRect(6, rect().center().y() - 3, 8, 8);
  arrowOpt.rect.translate(0, -3);
  s->drawPrimitive(QStyle::PE_IndicatorArrowUp, &arrowOpt, &p, this);
  arrowOpt.rect.translate(0, 6);
  s->drawPrimitive(QStyle::PE_IndicatorArrowDown, &arrowOpt, &p, this);
}

BadgeLabel::BadgeLabel()
{
    m_font = QApplication::font();
    m_font.setBold(true);
    m_font.setPixelSize(11);
}

void BadgeLabel::paint(QPainter *p, int x, int y, bool isChecked, const QPalette& pal)
{
    const QRectF rect(QRect(QPoint(x, y), m_size));
    p->save();

    p->setBrush(isChecked ? pal.color(QPalette::Active, QPalette::Background)
                          : pal.color(QPalette::Inactive, QPalette::Background));
    p->setPen(Qt::NoPen);
    p->setRenderHint(QPainter::Antialiasing, true);
    p->drawRoundedRect(rect, m_padding, m_padding, Qt::AbsoluteSize);

    p->setFont(m_font);
    p->setPen(isChecked ? pal.color(QPalette::Active, QPalette::Text)
                        : pal.color(QPalette::Inactive, QPalette::Text));
    p->drawText(rect, Qt::AlignCenter, m_text);

    p->restore();
}

void BadgeLabel::setText(const QString &text)
{
    m_text = text;
    calculateSize();
}

QString BadgeLabel::text() const
{
    return m_text;
}

QSize BadgeLabel::sizeHint() const
{
    return m_size;
}

void BadgeLabel::calculateSize()
{
    const QFontMetrics fm(m_font);
    m_size = fm.size(Qt::TextSingleLine, m_text);
    m_size.setWidth(m_size.width() + m_padding * 1.5);
    m_size.setHeight(2 * m_padding + 1); // Needs to be uneven for pixel perfect vertical centering in the button
}


OutputPaneResizeButton::OutputPaneResizeButton(QWidget *target, QWidget* parent)
  : QToolButton(parent)
  , m_target(target)
{
  setIcon(Application::Icon("resize"));
  setCursor(Qt::SizeBDiagCursor);
}

void OutputPaneResizeButton::mousePressEvent(QMouseEvent *event) {
  m_resizing = true;
  m_anchor = event->globalPos();
  m_anchorSize = m_target->size();
  m_anchorPos = m_target->pos();
}

void OutputPaneResizeButton::mouseReleaseEvent(QMouseEvent */*event*/) {
  m_resizing = false;
}

void OutputPaneResizeButton::mouseMoveEvent(QMouseEvent *event) {
  if (!m_resizing) return;
  QPoint d = event->globalPos() - m_anchor;
  QPoint n;
  n.setX(d.x() + m_anchorSize.width());
  n.setY(-d.y() + m_anchorSize.height());
  if (n.x() < m_target->minimumWidth()) return;
  if (n.y() < m_target->minimumHeight()) return;
  if (n.x() > m_target->parentWidget()->width() * 0.75) return;
  if (n.y() > m_target->parentWidget()->height() * 0.75) return;

  m_target->move(m_anchorPos.x(), m_anchorPos.y() + d.y());
  m_target->resize(n.x(), n.y());
}


