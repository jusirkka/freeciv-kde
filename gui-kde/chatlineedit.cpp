#include "chatlineedit.h"
#include "application.h"
#include "logging.h"

#include <QEvent>
#include <QKeyEvent>
#include <QCompleter>

#include "player.h"
#include "chat.h"
#include "game.h"

#include "chatline_common.h"
#include "client_main.h"
#include "options.h"

using namespace KV;

QStringList ChatLineEdit::m_history = QStringList();

ChatLineEdit::ChatLineEdit(QWidget* parent)
  : QLineEdit(parent)
  , m_historyPosition(HISTORY_END)
{
  connect(this, &ChatLineEdit::returnPressed, this, &ChatLineEdit::send);
  connect(Application::instance(), &Application::completionListChanged,
          this, &ChatLineEdit::updateCompletions);
}

bool ChatLineEdit::event(QEvent* event) {
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
    if (keyEvent->key() == Qt::Key_Up) {
      setText(backInHistory());
      event->accept();
      return true;
    }
    if (keyEvent->key() == Qt::Key_Down) {
      setText(forwardInHistory());
      event->accept();
      return true;
    }
    if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
      event->accept();
      emit returnPressed();
      return true;
    }
  }
  return QLineEdit::event(event);
}


void ChatLineEdit::updateCompletions(const QStringList& completions) {
  delete completer();
  auto c = new QCompleter(completions);
  c->setCaseSensitivity(Qt::CaseInsensitive);
  c->setCompletionMode(QCompleter::InlineCompletion);
  setCompleter(c);
}

void ChatLineEdit::send() {
  sendChatMessage(text());
  clear();
}

QString ChatLineEdit::backInHistory() {
  if (!m_history.empty() && m_historyPosition == HISTORY_END) {
    m_historyPosition = m_history.size() - 1;
  } else if (m_historyPosition > 0) {
    m_historyPosition--;
  }
  return m_history.empty() ? "" : m_history[m_historyPosition];
}

QString ChatLineEdit::forwardInHistory() {
  if (m_historyPosition == HISTORY_END) {
    return "";
  }
  m_historyPosition++;
  if (m_historyPosition >= m_history.size()) {
    m_historyPosition = HISTORY_END;
    return "";
  }
  return m_history[m_historyPosition];
}

void ChatLineEdit::resetHistoryPosition() {
  m_historyPosition = HISTORY_END;
}

static bool isPlainPublicMessage(QString s)
{
  auto str = s.trimmed();
  auto p = str[0];
  if (p == SERVER_COMMAND_PREFIX
      || p == CHAT_ALLIES_PREFIX
      || p == CHAT_DIRECT_PREFIX) {
    return false;
  }

  // Search for private message
  if (!str.contains(CHAT_DIRECT_PREFIX)) {
    return true;
  }

  int i = str.indexOf(CHAT_DIRECT_PREFIX);
  str = str.left(i);

  // Compare all players and connections looking for match
  conn_list_iterate(game.all_connections, pconn) {
    QString name = pconn->username;
    if (name.length() < i) continue;

    if (QString::compare(name.left(i), str, Qt::CaseInsensitive) == 0) {
      return false;
    }
  } conn_list_iterate_end;

  players_iterate(pplayer) {
    QString name = pplayer->name;
    if (name.length() < i) continue;

    if (QString::compare(name.left(i), str, Qt::CaseInsensitive) == 0) {
      return false;
    }
  } players_iterate_end;

  return true;
}

static QString serverCommand(const QString& s) {
  return QString("%1%2").arg(SERVER_COMMAND_PREFIX).arg(s);
}

void ChatLineEdit::sendChatMessage(const QString& message) {

  if (message.isEmpty()) return;

  m_history << message;
  resetHistoryPosition();

  // If client send commands to take ai, set /away to disable AI
  if (message.startsWith(serverCommand("take "))) {
    auto s = message;
    s = s.remove(serverCommand("take "));
    players_iterate(pplayer) {
      if (!is_ai(pplayer)) continue;

      auto p = QString("\"%1\"").arg(pplayer->name);
      if (p.compare(s) == 0) {
          send_chat(message.toLocal8Bit());
          send_chat(serverCommand("away").toLocal8Bit());
          return;
      }
    } players_iterate_end;
  }

  // Option to send to allies by default
  if (client_state() >= C_S_RUNNING
      && gui_options.gui_qt_allied_chat_only
      && isPlainPublicMessage(message)) {
    send_chat(QString("%1 %2").arg(CHAT_ALLIES_PREFIX).arg(message).toLocal8Bit());
  } else {
    send_chat(message.toLocal8Bit());
  }
}












