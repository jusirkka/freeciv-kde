#include "messagepane.h"
#include "application.h"
#include <QTimer>
#include <QVBoxLayout>

#include "messagewin_common.h"


using namespace KV;

ErrorWidget::ErrorWidget(const QString& e)
  : QLabel(e)
{
  auto fm = fontMetrics();
  setFixedHeight(fm.height() + 6);
  setFixedWidth(fm.width(e) + 6);
}

BrowserWidget::BrowserWidget()
  : TextBrowser()
{

  connect(Application::instance(), &Application::updateMessages,
          this, &BrowserWidget::updateMessages);
  updateMessages();
}

void BrowserWidget::handleError(const QString &msg) {
  if (m_errorWidget) {
    m_errorWidget->hide();
    delete m_errorWidget;
    m_errorWidget = nullptr;
  }
  m_errorWidget = new ErrorWidget(msg);
  QPoint p = mapFromGlobal(QCursor::pos());
  m_errorWidget->move(p.x(), p.y() - m_errorWidget->height());
  m_errorWidget->show();
  QTimer::singleShot(2000, [this] () {
    this->m_errorWidget->hide();
  });
}

void BrowserWidget::updateMessages() {
  int num = meswin_get_num_messages();
  if (num < 1) return;
  clear();
  for (int i = 0; i < num; i++) {
      auto m = meswin_get_message(i);
      append(Application::ApplyTags(m->descr, m->tags));
  }
  emit flash();
}


MessagePane::MessagePane()
  : m_mainWidget(new QWidget)
  , m_browser(new BrowserWidget)
{
  auto *layout = new QVBoxLayout;
  layout->setMargin(0);
  connect(m_browser, &BrowserWidget::flash, this, &MessagePane::flash);
  layout->addWidget(m_browser);
  m_mainWidget->setLayout(layout);
}


QWidget* MessagePane::outputWidget(QWidget */*parent*/) {
  return m_mainWidget;
}

QVector<QWidget*> MessagePane::toolBarWidgets() const {
  return {};
}

QString MessagePane::displayName() const {
  return tr("Messages");
}

int MessagePane::priorityInStatusBar() const {
  return 100;
}

void MessagePane::clearContents() {
  m_browser->clear();
}

void MessagePane::refreshContents() {
  // noop
}

void MessagePane::configureOutput() {
  // noop
}

void MessagePane::visibilityChanged(bool /*visible*/) {
  // noop
}

bool MessagePane::canConfigure() const {
  return false;
}

bool MessagePane::canRefresh() const {
  return false;
}















