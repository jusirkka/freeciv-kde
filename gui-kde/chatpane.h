#pragma once

#include <QWidget>
#include <QLabel>
#include "ioutputpane.h"

namespace KV {

class ChatLineEdit;
class ChatWindow;
class MessageConfigDialog;

class ChatPane : public IOutputPane
{
  Q_OBJECT

public:

  ChatPane();

  ChatLineEdit* chatLine() {return m_chatLine;}

  QWidget *outputWidget(QWidget *parent) override;
  QVector<QWidget*> toolBarWidgets() const override;
  QString displayName() const override;

  int priorityInStatusBar() const override;

  void clearContents() override;
  void refreshContents() override;
  void configureOutput() override;
  void visibilityChanged(bool visible) override;

  bool canConfigure() const override;
  bool canRefresh() const override;

private:

  QWidget *m_mainWidget;
  ChatLineEdit* m_chatLine;
  ChatWindow* m_chatWindow;
  MessageConfigDialog* m_config = nullptr;


};

}

