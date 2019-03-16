#include "reportpane.h"
#include "logging.h"
#include <QVBoxLayout>
#include "application.h"


#include "client_main.h"

using namespace KV;

ReportWidget::ReportWidget(QString c, QString n, report_type t)
  : QTextBrowser()
  , m_caption(c)
  , m_displayName(n)
  , m_reportType(t)
{
  connect(Application::instance(), &Application::updateReport,
          this, &ReportWidget::updateReport);
}


QString ReportWidget::displayName() const {
  return m_displayName;
}

report_type ReportWidget::reportType() const {
  return m_reportType;
}

void ReportWidget::updateReport(const QStringList& report) {
  QString caption = report[0];
  if (caption == "Traveller's Report:") {
    caption = report[1];
  }
  if (m_caption != caption) return;
  clear();
  append(report[1]);
  append("\n");
  append(report[2]);
}

Top5Cities::Top5Cities()
  : ReportWidget("The Five Greatest Cities in the World!", "Top 5 Cities", REPORT_TOP_5_CITIES)
{}

Wonders::Wonders()
  : ReportWidget("Wonders of the World", "Wonders of the World", REPORT_WONDERS_OF_THE_WORLD)
{}

Demographics::Demographics()
  : ReportWidget("Demographics Report:", "Demographics", REPORT_DEMOGRAPHIC)
{}

Achievements::Achievements()
  : ReportWidget("Achievements List:","Achievements", REPORT_ACHIEVEMENTS)
{}

ReportPane::ReportPane(ReportWidget* widget)
  : m_mainWidget(new QWidget)
  , m_report(widget)
{
  auto *layout = new QVBoxLayout;
  layout->setMargin(0);
  connect(m_report, &ReportWidget::flash, this, &ReportPane::flash);
  layout->addWidget(m_report);
  m_mainWidget->setLayout(layout);
}


QWidget* ReportPane::outputWidget(QWidget */*parent*/) {
  return m_mainWidget;
}

QVector<QWidget*> ReportPane::toolBarWidgets() const {
  return {};
}

QString ReportPane::displayName() const {
  return m_report->displayName();
}


void ReportPane::clearContents() {
  // noop
}

void ReportPane::refreshContents() {
  send_report_request(m_report->reportType());
}

bool ReportPane::canRefresh() const {
  return true;
}

void ReportPane::visibilityChanged(bool /*visible*/) {
  // noop
}


Top5CitiesPane::Top5CitiesPane()
  : ReportPane(new Top5Cities)
{}

bool Top5CitiesPane::canConfigure() const {
  return false;
}

void Top5CitiesPane::configureOutput() {
  // noop
}

int Top5CitiesPane::priorityInStatusBar() const {
  return 10;
}


WondersPane::WondersPane()
  : ReportPane(new Wonders)
{}

bool WondersPane::canConfigure() const {
  return false;
}

void WondersPane::configureOutput() {
  // noop
}

int WondersPane::priorityInStatusBar() const {
  return 20;
}

DemographicsPane::DemographicsPane()
  : ReportPane(new Demographics)
{}

bool DemographicsPane::canConfigure() const {
  return true;
}

void DemographicsPane::configureOutput() {
  // TODO
}

int DemographicsPane::priorityInStatusBar() const {
  return 30;
}

AchievementsPane::AchievementsPane()
  : ReportPane(new Achievements)
{}

bool AchievementsPane::canConfigure() const {
  return true;
}

void AchievementsPane::configureOutput() {
  // TODO
}

int AchievementsPane::priorityInStatusBar() const {
  return 5;
}
