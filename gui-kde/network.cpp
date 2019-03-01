#include "network.h"
#include "ui_network.h"
#include "logging.h"
#include <QApplication>
#include <QItemSelection>
#include <QTimer>

#include "fcintl.h"
#include "fcthread.h"
#include "servers.h"
#include "client_main.h"

using namespace KV;

Network::Network(QWidget *parent)
    : QDialog(parent)
    , m_UI(new Ui::Network)
    , m_localScan(nullptr)
    , m_globalScan(nullptr)
    , m_localScanDone(true)
    , m_globalScanDone(false)
    , m_holdingMutex(false)
    , m_scanTimer(new QTimer(this))
{
  m_UI->setupUi(this);

  setWindowTitle(QApplication::applicationName());

  connect(m_UI->detailsButton, &QPushButton::toggled, this, &Network::setDetailsButtonText);

  QStringList serverLabels{_("Server Name"),_("Port"), _("Version"), _("Status"), Q_("?count:Players"), _("Comment")};
  m_UI->localTable->setHorizontalHeaderLabels(serverLabels);
  m_UI->internetTable->setHorizontalHeaderLabels(serverLabels);

  QStringList playerLabels{_("Name"), _("Type"), _("Host"), _("Nation")};
  m_UI->playerTable->setHorizontalHeaderLabels(playerLabels);

  const QList<QHeaderView*> headers{
    m_UI->localTable->horizontalHeader(),
    m_UI->internetTable->horizontalHeader(),
    m_UI->playerTable->horizontalHeader()
  };
  for (auto header: headers) {
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setStretchLastSection(true);
  }

  connect(m_UI->internetTable->selectionModel(), &QItemSelectionModel::selectionChanged,
          this, &Network::serverChanged);
  connect(m_UI->localTable->selectionModel(), &QItemSelectionModel::selectionChanged,
          this, &Network::serverChanged);

  connect(m_scanTimer, &QTimer::timeout, this, &Network::checkServerScans);


  resize(640, 480);

  initWidget();
}

void Network::initWidget() {
  m_UI->detailsButton->setChecked(false);
  m_UI->playerBox->setVisible(m_UI->detailsButton->isChecked());
  m_UI->detailsButton->setText("Details >>");
  setResult(QDialog::Rejected);
  m_UI->passEdit->setEchoMode(QLineEdit::Password);
  m_UI->passEdit->setDisabled(true);

  m_UI->serverEdit->setText("localhost");
  m_UI->portSpin->setValue(5556);
  m_UI->userEdit->setText(user_name);

}

void Network::init() {
  initWidget();
  m_localScanDone = false;
  m_globalScanDone = false;
  m_scanTimer->start(1000);
  m_localScan = server_scan_begin(SERVER_SCAN_LOCAL,
                                  [] (struct server_scan *, const char *message) {
    qCWarning(FC) << message;
  });
  m_globalScan = server_scan_begin(SERVER_SCAN_GLOBAL,
                                  [] (struct server_scan *, const char *message) {
    qCWarning(FC) << message;
  });
}

void Network::final() {
  m_scanTimer->stop();
  server_scan_finish(m_localScan);
  m_localScan = nullptr;
  m_localScanDone = true;
  server_scan_finish(m_globalScan);
  m_globalScan = nullptr;
  m_globalScanDone = true;
}

void Network::checkServerScans() {

  if (!m_localScanDone) checkServerScan(m_localScan);
  if (!m_globalScanDone) checkServerScan(m_globalScan);

  if (m_localScanDone && m_globalScanDone) {
    m_scanTimer->stop();
  }
}

void Network::checkServerScan(server_scan *scan) {
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

void Network::updateServerList(server_scan *scan, const server_list *servers) {

  QTableWidget* table = scan == m_localScan ? m_UI->localTable : m_UI->internetTable;
  table->clearContents();
  table->setRowCount(0);

  QString host = m_UI->serverEdit->text();
  int port = m_UI->portSpin->value();

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



Network::~Network() {
  delete m_UI;
}

void Network::setDetailsButtonText(bool checked) {
  if (checked) {
    m_UI->detailsButton->setText("Details <<");
  } else {
    m_UI->detailsButton->setText("Details >>");
  }
}


void Network::serverChanged(const QItemSelection &selected, const QItemSelection&) {
  QModelIndexList indices = selected.indexes();
  if (indices.isEmpty()) return;
  m_UI->serverEdit->setText(indices[0].data().toString());
  m_UI->portSpin->setValue(indices[1].data().toString().toInt());

  auto tw = qobject_cast<QItemSelectionModel *>(sender());

  struct server_scan* scan;
  if (tw == m_UI->internetTable->selectionModel()) {
    scan = m_globalScan;
    m_UI->localTable->clearSelection();
  } else {
    scan = m_localScan;
    m_UI->internetTable->clearSelection();
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

  m_UI->playerTable->clearContents();
  m_UI->playerTable->setRowCount(0);

  for (int k = 0; k < server->nplayers; k++) {
    m_UI->playerTable->insertRow(k);
    m_UI->playerTable->setItem(k, 0, new QTableWidgetItem(server->players[k].name));
    m_UI->playerTable->setItem(k, 1, new QTableWidgetItem(server->players[k].type));
    m_UI->playerTable->setItem(k, 2, new QTableWidgetItem(server->players[k].host));
    m_UI->playerTable->setItem(k, 3, new QTableWidgetItem(server->players[k].nation));
  }
}













