#include "workmodel.h"
#include "sprite.h"
#include "buildables.h"
#include <QMimeData>
#include <QApplication>

#include "citydlg_common.h"
#include "tilespec.h"
#include "climisc.h"

using namespace KV;

WorkModel::WorkModel(QObject *parent)
  : QAbstractTableModel(parent)
  , m_mimeTypes{BuildablesDragModel::CidMimeType}
{
  worklist_init(&m_queue);
}

int WorkModel::columnCount(const QModelIndex &parent) const {
  if (parent.isValid()) return 0;
  return 2;
}

int WorkModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid()) return 0;
  return worklist_length(&m_queue);
}

QVariant WorkModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid() || m_city == nullptr) return QVariant();

  if (role == Qt::UserRole && index.column() != NameColumn) return QVariant();

  auto u = m_queue.entries[index.row()];

  if (role == Qt::UserRole) {
    return cid_encode(u);
  }

  if (index.column() == NameColumn) {

    if (role == Qt::DisplayRole) {
      if (u.kind == VUT_UTYPE) {
        return QString(utype_values_translation(u.value.utype));
      }
      return QString(city_improvement_name_translation(m_city, u.value.building));
    }

    if (role == Qt::DecorationRole) {
      int h = QApplication::fontMetrics().height() + 6;
      QPixmap pix;
      if (u.kind == VUT_UTYPE) {
        pix = get_unittype_sprite(get_tileset(),
                                  u.value.utype,
                                  direction8_invalid())->pm;
      } else {
        pix = get_building_sprite(get_tileset(), u.value.building)->pm;
      }
      return pix.scaledToHeight(h);
    }

    return QVariant();
  }


  if (index.column() == CostColumn) {

    if (role == Qt::TextAlignmentRole) {
      return Qt::AlignRight;
    }

    if (role == Qt::DisplayRole) {
      if (u.kind == VUT_UTYPE) {
        return utype_build_shield_cost(m_city, u.value.utype);
      }
      if (improvement_has_flag(u.value.building, IF_GOLD)) {
        return -1;
      }
      return impr_build_shield_cost(m_city, u.value.building);
    }
    return QVariant();
  }

  return QVariant();
}

bool WorkModel::removeRows(int row, int count, const QModelIndex &parent)
{
  beginRemoveRows(parent, row, row + count - 1);
  // we are removing at least once
  bool ok = count > 0 && row < worklist_length(&m_queue);
  while (count > 0 && row < worklist_length(&m_queue)) {
    worklist_remove(&m_queue, row);
    count--;
  }
  endRemoveRows();
  city_set_queue(m_city, &m_queue);
  return ok;
}


QMimeData* WorkModel::mimeData(const QModelIndexList &indices) const {
  auto data = new QMimeData;
  QByteArray encoded;
  QDataStream stream(&encoded, QIODevice::WriteOnly);

  for (auto idx: indices) {
    if (idx.isValid()) {
      auto d = idx.data(Qt::UserRole);
      if (d.isValid()) {
        cid id = d.value<cid>();
        stream << id;
      }
    }
  }
  data->setData(BuildablesDragModel::CidMimeType, encoded);

  return data;
}

QStringList WorkModel::mimeTypes() const {
  return m_mimeTypes;
}

bool WorkModel::canDropMimeData(const QMimeData *data,
                     Qt::DropAction /*action*/,
                     int /*row*/, int column,
                     const QModelIndex &/*parent*/) const {

  if (!data->hasFormat(BuildablesDragModel::CidMimeType)) return false;
  if (column > 0) return false;
  return true;
}

bool WorkModel::dropMimeData(const QMimeData *data,
                  Qt::DropAction action,
                  int row, int column,
                  const QModelIndex &parent) {
  if (!canDropMimeData(data, action, row, column, parent)) return false;

  if (action == Qt::IgnoreAction) return true;

  int beginRow;
  if (row >= 0 && row < rowCount()) {
    beginRow = row;
  } else if (parent.isValid()) {
    beginRow = parent.row();
  } else {
    beginRow = rowCount();
  }

  QByteArray encodedData = data->data(BuildablesDragModel::CidMimeType);
  QDataStream stream(&encodedData, QIODevice::ReadOnly);

  bool ok = false;

  while (!stream.atEnd()) {
    cid id;
    stream >> id;
    auto u = cid_decode(id);
    if (worklist_insert(&m_queue, &u, beginRow)) {
      beginInsertRows(QModelIndex(), beginRow, beginRow);
      ok = true;
      beginRow++;
      endInsertRows();
    }
  }

  if (action == Qt::CopyAction) {
    city_set_queue(m_city, &m_queue);
  }

  return ok;
}

Qt::DropActions WorkModel::supportedDropActions() const {
  return Qt::MoveAction | Qt::CopyAction;
}

Qt::ItemFlags WorkModel::flags(const QModelIndex &index) const {
  Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);


  if (index.isValid()) {
    if (index.column() > 0) return defaultFlags;
    return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
  }

  return Qt::ItemIsDropEnabled | defaultFlags;
}


void WorkModel::changeCity(city* c) {
  beginResetModel();
  m_city = c;
  city_get_queue(m_city, &m_queue);
  endResetModel();
}

