#pragma once
#include <QAbstractListModel>
#include "worklist.h"

struct city;

namespace KV {


class WorkModel : public QAbstractTableModel
{
  Q_OBJECT

public:

  static const int NameColumn = 0;
  static const int CostColumn = 1;

  explicit WorkModel(QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

  QMimeData* mimeData(const QModelIndexList &indexes) const override;
  QStringList mimeTypes() const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;

  bool canDropMimeData(const QMimeData *data,
                       Qt::DropAction action,
                       int row, int column,
                       const QModelIndex &parent) const override;

  bool dropMimeData(const QMimeData *data,
                    Qt::DropAction action,
                    int row, int column,
                    const QModelIndex &parent) override;

  Qt::DropActions supportedDropActions() const override;



  void changeCity(city* c);

private:

  city* m_city = nullptr;
  QStringList m_mimeTypes;
  worklist m_queue;

};

}
