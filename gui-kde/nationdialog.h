#ifndef NATIONDIALOG_H
#define NATIONDIALOG_H

#include <QDialog>
#include <QAbstractListModel>


namespace Ui {
class NationDialog;
}

struct player;
class QAbstractButton;
class QItemSelection;

namespace KV {

class NationDialog : public QDialog
{
  Q_OBJECT

public:
  explicit NationDialog(QWidget *parent = nullptr);
  ~NationDialog();

  void init(const player* p);

public slots:

  void refresh(bool nationsetChange);

private slots:

  void nationsetChanged(const QString& s);
  void nationGroupSelected();
  void filterChanged(const QString& s);
  void nationSelected(const QItemSelection& c, const QItemSelection& p);
  void leaderChanged(int idx);
  void randomNation();
  void setNation();

private:

  void initNationGroups();

private:

  Ui::NationDialog* m_ui;
  const player* m_player;


};


class NationModel: public QAbstractListModel {

  Q_OBJECT

public:

  NationModel(QObject* parent = nullptr);
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
  bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
  bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

private:

  struct Nation {
    QString name;
    QPixmap flag;
    int id;
  };

  QVector<Nation> m_nations;
  QMap<int, QFont> m_fonts;

};

}
#endif // NATIONDIALOG_H
