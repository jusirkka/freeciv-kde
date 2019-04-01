#pragma once

#include <QAbstractProxyModel>

namespace KV {

class ProxyTableModel: public QAbstractProxyModel {
  Q_OBJECT

public:

  ProxyTableModel(int columns, QObject* parent = nullptr);
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
  QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
  QModelIndex parent(const QModelIndex &child) const override;
  QModelIndex index(int row, int column, const QModelIndex &parent) const override;

private:

  int m_columns;
};


} // namespace KV
