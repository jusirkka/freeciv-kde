#pragma once

#include "textbrowser.h"

namespace KV {

class ChatWindow: public TextBrowser
{

  Q_OBJECT

public:

  ChatWindow(QWidget* parent = nullptr);

protected:

  void handleError(const QString &msg) override;

signals:

  void flash();

private slots:

  void receive(const QString& message);

};

}

