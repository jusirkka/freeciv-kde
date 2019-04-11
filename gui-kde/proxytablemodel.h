#pragma once

#include <QAbstractProxyModel>

namespace KV {

class ProxyTableModel: public QAbstractProxyModel {
  Q_OBJECT

public:

  ProxyTableModel(int columns, QObject* parent = nullptr);
  void setSourceModel(QAbstractItemModel *sourceModel) override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
  QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
  QModelIndex parent(const QModelIndex &child) const override;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

private:

  void sourceRowsRemoved(const QModelIndex &parent, int first, int last);
  void sourceRowsInserted(const QModelIndex &parent, int first, int last);

private:

  int m_columns;
};


} // namespace KV
