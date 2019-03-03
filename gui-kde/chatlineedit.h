#ifndef CHATLINEEDIT_H
#define CHATLINEEDIT_H


#include <QLineEdit>


namespace KV {

class ChatLineEdit : public QLineEdit
{
  Q_OBJECT

  static const int HISTORY_END = -1;

  static QStringList m_history;

public:

  explicit ChatLineEdit(QWidget* parent = nullptr);

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
