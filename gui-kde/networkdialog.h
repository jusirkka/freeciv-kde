#ifndef NETWORK_H
#define NETWORK_H

#include <QDialog>

namespace Ui {
class NetworkDialog;
}

class QItemSelection;
class QTimer;
struct server_scan;
struct server_list;


namespace KV {

class NetworkDialog: public QDialog
{
    Q_OBJECT

public:

  NetworkDialog(QWidget *parent);

  ~NetworkDialog() override;

  void init();
  void final();

  QString server() const;
  QString user() const;
  int port() const;

private slots:

  void setDetailsButtonText(bool checked);
  void serverChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void checkServerScans();

private:

  void initWidget();
  void updateServerList(server_scan* scan, const server_list* servers);
  void checkServerScan(server_scan* scan);

private:

    Ui::NetworkDialog* m_ui;
    server_scan* m_localScan;
    server_scan* m_globalScan;
    bool m_localScanDone;
    bool m_globalScanDone;
    bool m_holdingMutex;
    QTimer* m_scanTimer;
};
}
#endif // NETWORK_H
