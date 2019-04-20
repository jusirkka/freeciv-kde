#ifndef CHATLINEEDIT_H
#define CHATLINEEDIT_H


#include <QLineEdit>


struct tile;

namespace KV {

namespace Chat {
QString makeServerCommand(const QString& s);
void sendServerCommand(const QString& cmd);
QString makeDirectCommand(const QString& s);
void sendDirectCommand(const QString& s);
QString makeAlliesCommand(const QString& s);
void sendAlliesCommand(const QString& s);
void sendCommand(const QString& cmd);
}

class ChatLineEdit : public QLineEdit
{
  Q_OBJECT

  static const int HISTORY_END = -1;

  static QStringList m_history;

public:

  explicit ChatLineEdit(QWidget* parent = nullptr);

  void makeLink(tile* ptile);

  bool event(QEvent* event) override;

private slots:

  void updateCompletions(const QStringList& completions);
  void send();

private:

  QString backInHistory();
  QString forwardInHistory();
  void resetHistoryPosition();
  void sendChatMessage(const QString& message);

private:

  int m_historyPosition;

};

} // namespace KV

#endif
