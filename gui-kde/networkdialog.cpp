#include "networkdialog.h"
#include "ui_networkdialog.h"
#include "logging.h"
#include <QApplication>
#include <QItemSelection>
#include <QTimer>

#include "fcintl.h"
#include "fcthread.h"
#include "servers.h"
#include "client_main.h"

using namespace KV;

NetworkDialog::NetworkDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::NetworkDialog)
    , m_localScan(nullptr)
    , m_globalScan(nullptr)
    , m_localScanDone(true)
    , m_globalScanDone(false)
    , m_holdingMutex(false)
    , m_scanTimer(new QTimer(this))
{
  m_ui->setupUi(this);

  setWindowTitle(QApplication::applicationName());

  connect(m_ui->detailsButton, &QPushButton::toggled,
          this, &NetworkDialog::setDetailsButtonText);

  QStringList serverLabels{_("Server Name"),_("Port"), _("Version"), _("Status"), Q_("?count:Players"), _("Comment")};
  m_ui->localTable->setHorizontalHeaderLabels(serverLabels);
  m_ui->internetTable->setHorizontalHeaderLabels(serverLabels);

  QStringList playerLabels{_("Name"), _("Type"), _("Host"), _("Nation")};
  m_ui->playerTable->setHorizontalHeaderLabels(playerLabels);

  const QList<QHeaderView*> headers{
    m_ui->localTable->horizontalHeader(),
    m_ui->internetTable->horizontalHeader(),
    m_ui->playerTable->horizontalHeader()
  };
  for (auto header: headers) {
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setStretchLastSection(true);
  }

  connect(m_ui->internetTable->selectionModel(), &QItemSelectionModel::selectionChanged,
          this, &NetworkDialog::serverChanged);
  connect(m_ui->localTable->selectionModel(), &QItemSelectionModel::selectionChanged,
          this, &NetworkDialog::serverChanged);

  connect(m_scanTimer, &QTimer::timeout, this, &NetworkDialog::checkServerScans);


  resize(640, 480);

  initWidget();
}

void NetworkDialog::initWidget() {
  m_ui->detailsButton->setChecked(false);
  m_ui->playerBox->setVisible(m_ui->detailsButton->isChecked());
  m_ui->detailsButton->setText("Details >>");
  setResult(QDialog::Rejected);
  m_ui->passEdit->setEchoMode(QLineEdit::Password);
  m_ui->passEdit->setDisabled(true);

  m_ui->serverEdit->setText("localhost");
  m_ui->portSpin->setValue(5556);
  m_ui->userEdit->setText(user_name);

}

void NetworkDialog::init() {
  initWidget();
  m_localScanDone = false;
  m_globalScanDone = false;
  m_localScan = server_scan_begin(SERVER_SCAN_LOCAL,
                                  [] (struct server_scan *, const char *message) {
    qCWarning(FC) << message;
  });
  m_globalScan = server_scan_begin(SERVER_SCAN_GLOBAL,
                                  [] (struct server_scan *, const char *message) {
    qCWarning(FC) << message;
  });
  checkServerScans();
  m_scanTimer->start(500);
}

void NetworkDialog::final() {
  m_scanTimer->stop();
  server_scan_finish(m_localScan);
  m_localScan = nullptr;
  m_localScanDone = true;
  server_scan_finish(m_globalScan);
  m_globalScan = nullptr;
  m_globalScanDone = true;
}

void NetworkDialog::checkServerScans() {

  if (!m_localScanDone) checkServerScan(m_localScan);
  if (!m_globalScanDone) checkServerScan(m_globalScan);

  if (m_localScanDone && m_globalScanDone) {
    m_scanTimer->stop();
  }
}

void NetworkDialog::checkServerScan(server_scan *scan) {
  auto stat = server_scan_poll(scan);

  if (stat >= SCAN_STATUS_PARTIAL) {
    auto serverList = server_scan_get_list(scan);
    fc_allocate_mutex(&serverList->mutex);
    m_holdingMutex = true;
    updateServerList(scan, serverList->servers);
    m_holdingMutex = false;
    fc_release_mutex(&serverList->mutex);
  }

  if (stat == SCAN_STATUS_ERROR) {
      qCWarning(FC) << (scan == m_localScan ? "Local" : "Global") << "scan error";
  } else if (stat == SCAN_STATUS_DONE) {
    if (scan == m_localScan) {
      qCInfo(FC) << "Local scan done";
      m_localScanDone = true;
    } else {
      qCInfo(FC) << "Global scan done";
      m_globalScanDone = true;
    }
  }
}

void NetworkDialog::updateServerList(server_scan *scan, const server_list *servers) {

  QTableWidget* table = scan == m_localScan ? m_ui->localTable : m_ui->internetTable;
  table->clearContents();
  table->setRowCount(0);

  QString host = m_ui->serverEdit->text();
  int port = m_ui->portSpin->value();

  int row = 0;
  server_list_iterate(servers, server) {
    table->insertRow(row);
    table->setItem(row, 0, new QTableWidgetItem(server->host));
    table->setItem(row, 1, new QTableWidgetItem(QString::number(server->port)));
    table->setItem(row, 2, new QTableWidgetItem(server->version));
    table->setItem(row, 3, new QTableWidgetItem(server->state));
    QString humans = server->humans >= 0 ? QString::number(server->humans) : "Unknown";
    table->setItem(row, 4, new QTableWidgetItem(humans));
    table->setItem(row, 5, new QTableWidgetItem(server->message));
    if (host == server->host && port == server->port) {
      table->selectRow(row);
    }
    row++;
  } server_list_iterate_end;

}



NetworkDialog::~NetworkDialog() {
  delete m_ui;
}

void NetworkDialog::setDetailsButtonText(bool checked) {
  if (checked) {
    m_ui->detailsButton->setText("Details <<");
  } else {
    m_ui->detailsButton->setText("Details >>");
  }
}


void NetworkDialog::serverChanged(const QItemSelection &selected, const QItemSelection&) {
  QModelIndexList indices = selected.indexes();
  if (indices.isEmpty()) return;
  m_ui->serverEdit->setText(indices[0].data().toString());
  m_ui->portSpin->setValue(indices[1].data().toString().toInt());

  auto tw = qobject_cast<QItemSelectionModel *>(sender());

  struct server_scan* scan;
  if (tw == m_ui->internetTable->selectionModel()) {
    scan = m_globalScan;
    m_ui->localTable->clearSelection();
  } else {
    scan = m_localScan;
    m_ui->internetTable->clearSelection();
  }

  auto serverList = server_scan_get_list(scan);
  if (!m_holdingMutex) {
    fc_allocate_mutex(&serverList->mutex);
  }

  auto server = server_list_get(serverList->servers, indices[0].row());

  if (!m_holdingMutex) {
    fc_release_mutex(&serverList->mutex);
  }

  if (!server || !server->players) {
    return;
  }

  m_ui->playerTable->clearContents();
  m_ui->playerTable->setRowCount(0);

  for (int k = 0; k < server->nplayers; k++) {
    m_ui->playerTable->insertRow(k);
    m_ui->playerTable->setItem(k, 0, new QTableWidgetItem(server->players[k].name));
    m_ui->playerTable->setItem(k, 1, new QTableWidgetItem(server->players[k].type));
    m_ui->playerTable->setItem(k, 2, new QTableWidgetItem(server->players[k].host));
    m_ui->playerTable->setItem(k, 3, new QTableWidgetItem(server->players[k].nation));
  }
}


QString NetworkDialog::user() const {
  return m_ui->userEdit->text();
}

QString NetworkDialog::server() const {
  return m_ui->serverEdit->text();
}

int NetworkDialog::port() const {
  return m_ui->portSpin->value();
}









