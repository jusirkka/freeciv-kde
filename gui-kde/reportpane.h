#pragma once

#include "ioutputpane.h"
#include <QWidget>
#include <QTextBrowser>

#include "packets.h"

namespace KV {

class ReportWidget: public QTextBrowser {
  Q_OBJECT
public:
  ReportWidget(QString c, QString n, report_type t);

  QString displayName() const;
  report_type reportType() const;

signals:
  void flash();
protected:
  void updateReport(const QStringList& report);
protected:
  QString m_caption;
  QString m_displayName;
  report_type m_reportType;
};

class Top5Cities: public ReportWidget {
  Q_OBJECT
public:
  Top5Cities();
};

class Wonders: public ReportWidget {
  Q_OBJECT
public:
  Wonders();
};

class Demographics: public ReportWidget {
  Q_OBJECT
public:
  Demographics();
};

class Achievements: public ReportWidget {
  Q_OBJECT
public:
  Achievements();
};

class ReportPane : public IOutputPane
{
  Q_OBJECT

public:

  ReportPane(ReportWidget* widget);

  QWidget *outputWidget(QWidget *parent) override;
  QVector<QWidget*> toolBarWidgets() const override;

  QString displayName() const override;
  void clearContents() override;
  void refreshContents() override;
  void visibilityChanged(bool visible) override;

  bool canRefresh() const override;

protected:

  QWidget *m_mainWidget;
  ReportWidget *m_report;

};

class Top5CitiesPane : public ReportPane
{
  Q_OBJECT

public:

  Top5CitiesPane();
  bool canConfigure() const override;
  void configureOutput() override;
  int priorityInStatusBar() const override;

};

class WondersPane : public ReportPane
{
  Q_OBJECT

public:

  WondersPane();
  bool canConfigure() const override;
  void configureOutput() override;
  int priorityInStatusBar() const override;

};

class DemographicsPane : public ReportPane
{
  Q_OBJECT

public:

  DemographicsPane();
  bool canConfigure() const override;
  void configureOutput() override;
  int priorityInStatusBar() const override;

};

class AchievementsPane : public ReportPane
{
  Q_OBJECT

public:

  AchievementsPane();
  bool canConfigure() const override;
  void configureOutput() override;
  int priorityInStatusBar() const override;

};

}

