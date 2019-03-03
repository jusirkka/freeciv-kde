#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QTextBrowser>

struct text_tag_list;
class QUrl;

namespace KV {

class ChatWindow: public QTextBrowser
{

  Q_OBJECT

public:

  ChatWindow(QWidget* parent = nullptr);


private slots:

  void receive(const QString& message);
  void handleAnchorClick(const QUrl &link);


};

} // namespace KV

#endif // CHATWINDOW_H
