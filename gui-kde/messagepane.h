#pragma once

#include <QWidget>
#include <QLabel>
#include "ioutputpane.h"
#include "textbrowser.h"

namespace KV {

class ErrorWidget: public QLabel {
  Q_OBJECT
public:
  ErrorWidget(const QString& e);
};

class BrowserWidget: public TextBrowser {
  Q_OBJECT
public:
  BrowserWidget();
protected:
  void handleError(const QString &msg) override;
signals:
  void flash();
private:
  void updateMessages();
private:
  ErrorWidget* m_errorWidget = nullptr;
};


class MessagePane : public IOutputPane
{
  Q_OBJECT

public:

  MessagePane();

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
  BrowserWidget *m_browser;

};

}

