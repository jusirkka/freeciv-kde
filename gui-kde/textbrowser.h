#pragma once

#include <QTextBrowser>

namespace KV {

class TextBrowser: public QTextBrowser
{

public:

  TextBrowser(QWidget* parent = nullptr);

protected:

  void handleAnchorClick(const QUrl &link);
  virtual void handleError(const QString& msg) = 0;

};

}
