#include "proxytablemodel.h"
#include <QAbstractItemModel>

using namespace KV;

ProxyTableModel::ProxyTableModel(int columns, QObject* parent)
  : QAbstractProxyModel(parent)
  , m_columns(columns)
{}

void ProxyTableModel::setSourceModel(QAbstractItemModel *model) {
  beginResetModel();
  if (sourceModel() != nullptr) {
    disconnect(sourceModel(), &QAbstractItemModel::rowsInserted,
               this, &ProxyTableModel::sourceRowsInserted);
    disconnect(sourceModel(), &QAbstractItemModel::rowsRemoved,
               this, &ProxyTableModel::sourceRowsRemoved);
    disconnect(sourceModel(), &QAbstractItemModel::modelReset,
               this, &ProxyTableModel::modelReset);
  }
  QAbstractProxyModel::setSourceModel(model);
  connect(sourceModel(), &QAbstractItemModel::rowsInserted,
             this, &ProxyTableModel::sourceRowsInserted);
  connect(sourceModel(), &QAbstractItemModel::rowsRemoved,
             this, &ProxyTableModel::sourceRowsRemoved);
  connect(sourceModel(), &QAbstractItemModel::modelReset,
             this, &ProxyTableModel::modelReset);
  endResetModel();
}

int ProxyTableModel::columnCount(const QModelIndex &parent) const {
  int sourceRows = sourceModel()->rowCount(parent);
  if (sourceRows < m_columns) {
    return sourceRows;
  }
  return m_columns;
}

int ProxyTableModel::rowCount(const QModelIndex &parent) const {
  int sourceRows = sourceModel()->rowCount(parent);
  return (sourceRows + m_columns - 1) / m_columns;
}

QModelIndex ProxyTableModel::mapFromSource(const QModelIndex &sourceIndex) const {
  if (sourceIndex.column() > 0) return QModelIndex();
  int col = sourceIndex.row() % m_columns;
  int row = sourceIndex.row() / m_columns;
  return createIndex(row, col);
}

QModelIndex ProxyTableModel::mapToSource(const QModelIndex &proxyIndex) const {
  int row = proxyIndex.row() * m_columns + proxyIndex.column();
  if (row >= sourceModel()->rowCount()) return QModelIndex();
  return sourceModel()->index(row, 0);
}

QModelIndex ProxyTableModel::parent(const QModelIndex &/*child*/) const {
  return QModelIndex();
}

QModelIndex ProxyTableModel::index(int row, int column, const QModelIndex &parent) const {
  return createIndex(row, column);
}

void ProxyTableModel::sourceRowsRemoved(const QModelIndex &/*parent*/, int first, int last) {
  auto topLeft = index(first / m_columns, 0);
  auto bottomRight = index(sourceModel()->rowCount() - 1 / m_columns, m_columns - 1);
  emit dataChanged(topLeft, bottomRight);
}

void ProxyTableModel::sourceRowsInserted(const QModelIndex &/*parent*/, int first, int last) {
  auto topLeft = index(first / m_columns, 0);
  auto bottomRight = index(sourceModel()->rowCount() - 1 / m_columns, m_columns - 1);
  emit dataChanged(topLeft, bottomRight);
}
