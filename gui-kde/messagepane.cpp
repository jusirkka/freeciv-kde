#include "messagepane.h"
#include "application.h"
#include "logging.h"
#include <QTimer>
#include <QVBoxLayout>
#include "messageconfigdialog.h"

#include "messagewin_common.h"


using namespace KV;

ErrorWidget::ErrorWidget(const QString& e, QWidget* p)
  : QLabel(e, p)
{
  setWindowFlags(Qt::X11BypassWindowManagerHint |
                 Qt::FramelessWindowHint |
                 Qt::WindowStaysOnTopHint);
  setAttribute(Qt::WA_DeleteOnClose);
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
    m_errorWidget->close();
    m_errorWidget = nullptr;
  }
  m_errorWidget = new ErrorWidget(msg, this);
  m_errorWidget->move(mapFromGlobal(QCursor::pos()));
  m_errorWidget->show();
  QTimer::singleShot(4000, [=] () {
    m_errorWidget->hide();
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
  : IOutputPane()
  , m_mainWidget(new QWidget)
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
  m_browser->updateMessages();
}

void MessagePane::configureOutput() {
  if (m_config == nullptr) {
    m_config = new MessageConfigDialog(MW_MESSAGES, "Messages", m_mainWidget);
    connect(m_config, &MessageConfigDialog::finished, this, [=] () {
      m_config = nullptr;
    });
  }
  m_config->show();
}

void MessagePane::visibilityChanged(bool /*visible*/) {
  // noop
}

bool MessagePane::canConfigure() const {
  return true;
}

bool MessagePane::canRefresh() const {
  return true;
}
