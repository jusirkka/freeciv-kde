#ifndef NETWORK_H
#define NETWORK_H

#include <QDialog>

namespace Ui {
class Network;
}

class QItemSelection;
class QTimer;
struct server_scan;
struct server_list;


namespace KV {

class Network: public QDialog
{
    Q_OBJECT

public:

  Network(QWidget *parent);

  ~Network() override;

  void init();
  void final();

private slots:

  void setDetailsButtonText(bool checked);
  void serverChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void checkServerScans();

private:

  void initWidget();
  void updateServerList(server_scan* scan, const server_list* servers);
  void checkServerScan(server_scan* scan);

private:

    Ui::Network* m_UI;
    server_scan* m_localScan;
    server_scan* m_globalScan;
    bool m_localScanDone;
    bool m_globalScanDone;
    bool m_holdingMutex;
    QTimer* m_scanTimer;
};
}
#endif // NETWORK_H
