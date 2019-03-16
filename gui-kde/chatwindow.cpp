#include "chatwindow.h"
#include "application.h"
#include <QScrollBar>

#include "chatline_common.h"

using namespace KV;


ChatWindow::ChatWindow(QWidget* parent)
  : TextBrowser(parent)
{
  connect(Application::instance(), &Application::chatMessage,
          this, &ChatWindow::receive);
}


void ChatWindow::receive(const QString &message) {
  append(message);
  verticalScrollBar()->setSliderPosition(verticalScrollBar()->maximum());
  emit flash();
}

void ChatWindow::handleError(const QString &msg) {
  output_window_append(ftc_client, msg.toUtf8());
}

