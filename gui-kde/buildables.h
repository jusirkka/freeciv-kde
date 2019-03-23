#pragma once

#include <QTableView>
#include <QItemDelegate>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QAbstractProxyModel>
#include <QIdentityProxyModel>

#include "climisc.h" // cid

struct city;
struct universal;

namespace KV {

class BuildablesModel : public QAbstractListModel {

  Q_OBJECT

public:

  BuildablesModel(city *c, QObject *parent = nullptr);
  int rowCount(const QModelIndex &index = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;


  void changeCity(city* c);

private:

  QString unitTooltip(unit_type* u) const;
  QString improvementTooltip(impr_type* i) const;
  QColor getItemColor(const universal& u) const;

private:

  QVector<cid> m_targets;
  city* m_city;

};

class BuildablesFilter: public QSortFilterProxyModel {

  Q_OBJECT

public:

  static const quint64 Units = 1;
  static const quint64 Buildings = 2;
  static const quint64 Wonders = 4;
  static const quint64 Future = 8;
  static const quint64 UnitsBuildingsWonders = 7;

  BuildablesFilter(city* c, int flags, QObject* parent = nullptr);
  bool filterAcceptsRow(int row, const QModelIndex &parent) const override;

  void setFilterFlags(int flags);
  int filterFlags() const;
  void changeCity(city* c);

private:

  city* m_city;
  int m_flags;
};

class BuildablesDragModel: public QIdentityProxyModel {
  Q_OBJECT

public:

  static const char* CidMimeType;

  BuildablesDragModel(QObject *parent = nullptr);
  QMimeData* mimeData(const QModelIndexList &indexes) const override;
  QStringList mimeTypes() const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;

private:

  QStringList m_mimeTypes;

};

class BuildablesTableModel: public QAbstractProxyModel {
  Q_OBJECT

public:

  BuildablesTableModel(int columns, QObject* parent = nullptr);
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
  QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
  QModelIndex parent(const QModelIndex &child) const override;
  QModelIndex index(int row, int column, const QModelIndex &parent) const override;

private:

  int m_columns;
};

class BuildablesDelegate: public QItemDelegate
{
  Q_OBJECT

public:
  BuildablesDelegate(city *city, QSize hint, QObject *parent = nullptr);
  void paint(QPainter *painter,
             const QStyleOptionViewItem &option,
             const QModelIndex &index) const override;
  QSize sizeHint(const QStyleOptionViewItem &option,
                 const QModelIndex &index) const override;

protected:
  void drawFocus(QPainter *painter,
                 const QStyleOptionViewItem &option,
                 const QRect &rect) const override;
private:


  static const int Sea = 1;
  static const int Flying = 2;
  static const int Neutral = 4;


  city* m_city;
  QSize m_hint;

};


class Buildables: public QTableView
{
  Q_OBJECT
public:
  Buildables(city* c, quint64 flags, QWidget* parent = nullptr);

protected:

  bool eventFilter(QObject *object, QEvent *event) override;

signals:

  void selected(universal);

private slots:

  void buildingSelected(const QItemSelection &selection, const QItemSelection &);

private:

  QSize computeCellSize(BuildablesFilter* filter) const;

private:

  city* m_city;

};

}
