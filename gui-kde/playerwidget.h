#pragma once

#include <QDialog>
#include <QAbstractListModel>
#include <QItemSelection>

#include "player.h"

namespace Ui {
class PlayerWidget;
}

class QSortFilterProxyModel;

namespace KV {


class PlayerModel: public QAbstractListModel {

  Q_OBJECT

public:

  void reset();

  PlayerModel(QObject* parent = nullptr);
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int, Qt::Orientation, int role = Qt::DisplayRole) const override;
  bool setHeaderData(int section, Qt::Orientation orientation,
                     const QVariant &value, int role = Qt::DisplayRole) override;

private:


  QVector<player*> m_players;

};


class PlayerWidget : public QWidget
{
  Q_OBJECT

public:
  explicit PlayerWidget(QWidget *parent = nullptr);
  ~PlayerWidget();


signals:

  void closeRequest();

private slots:

  void on_closeButton_clicked();
  void on_meetButton_clicked();
  void on_withdrawVisionButton_clicked();
  void on_cancelTreatyButton_clicked();
  void updatePlayers();
  void playerSelected(const QItemSelection &s, const QItemSelection &);

private:

  void popupHeaderMenu(const QPoint&);
  void readSettings();
  void writeSettings();

private:
  Ui::PlayerWidget *m_ui;
  PlayerModel* m_players;
  QSortFilterProxyModel* m_filter;
};

}

