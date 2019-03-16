#pragma once

#include <QDialog>
#include <QAbstractListModel>
#include <QItemSelection>

#include "player.h"

namespace Ui {
class PlayerDialog;
}

class QSortFilterProxyModel;

namespace KV {


class PlayerModel: public QAbstractListModel {

  Q_OBJECT

public:

  void reset();

  PlayerModel(QObject* parent = nullptr);
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int, Qt::Orientation, int role = Qt::DisplayRole) const override;

private:


  QVector<player*> m_players;

};


class PlayerDialog : public QDialog
{
  Q_OBJECT

public:
  explicit PlayerDialog(QWidget *parent = nullptr);
  ~PlayerDialog();

  void on_closeButton_clicked();
  void on_meetButton_clicked();
  void on_withdrawVisionButton_clicked();
  void on_cancelTreatyButton_clicked();

private:

  void popupHeaderMenu(const QPoint&);
  void playerSelected(const QItemSelection &s, const QItemSelection &);

private:
  Ui::PlayerDialog *m_ui;
  PlayerModel* m_players;
  QSortFilterProxyModel* m_filter;
};

}

