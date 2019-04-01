#include "chatpane.h"
#include "chatwindow.h"
#include "chatlineedit.h"
#include "messageconfigdialog.h"

#include <QVBoxLayout>

#include "messagewin_common.h"


using namespace KV;


ChatPane::ChatPane()
  : m_mainWidget(new QWidget)
  , m_chatLine(new ChatLineEdit)
  , m_chatWindow(new ChatWindow)
{
  auto *layout = new QVBoxLayout;
  layout->setMargin(0);
  layout->addWidget(m_chatWindow);
  connect(m_chatWindow, &ChatWindow::flash, this, &ChatPane::flash);
  layout->addWidget(m_chatLine);
  m_mainWidget->setLayout(layout);
}


QWidget* ChatPane::outputWidget(QWidget */*parent*/) {
  return m_mainWidget;
}

QVector<QWidget*> ChatPane::toolBarWidgets() const {
  return {};
}

QString ChatPane::displayName() const {
  return tr("Chat");
}

int ChatPane::priorityInStatusBar() const {
  return 90;
}

void ChatPane::clearContents() {
  m_chatWindow->clear();
}

void ChatPane::refreshContents() {
  // noop
}

void ChatPane::configureOutput() {
  if (m_config == nullptr) {
    m_config = new MessageConfigDialog(MW_OUTPUT, "Chat Messages", m_mainWidget);
    connect(m_config, &MessageConfigDialog::finished, this, [=] () {
      m_config = nullptr;
    });
  }
  m_config->show();
}

void ChatPane::visibilityChanged(bool /*visible*/) {
  // noop
}

bool ChatPane::canConfigure() const {
  return true;
}

bool ChatPane::canRefresh() const {
  return false;
}
