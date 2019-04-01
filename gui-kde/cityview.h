#pragma once

#include <QDialog>
#include <QSortFilterProxyModel>
#include <QItemDelegate>
#include <QAbstractTableModel>

struct city;

namespace Ui {
class CityView;
}

namespace KV {

class CityModel: public QAbstractTableModel
{
  Q_OBJECT

public:

  CityModel(QObject *parent = nullptr);

  int rowCount(const QModelIndex &index = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
  bool setHeaderData(int section,
                     Qt::Orientation orientation,
                     const QVariant &value, int role = Qt::DisplayRole) override;

  void reset();
  void updateCity(city* c);
  QModelIndex toIndex(city* c, int col = 0) const;

private:

  QVector<city*> m_cities;

};

class CityFilter {
public:
  virtual const QString& name() const = 0;
  virtual bool apply(const city* c) const = 0;
  virtual ~CityFilter() = default;
};


class CityFilterModel: public QSortFilterProxyModel
{
  Q_OBJECT
public:
  CityFilterModel(QObject* parent = nullptr);
  ~CityFilterModel();
  void addFilter(CityFilter* filter);
  void removeFilter(const QString& name);
private:
  bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
  bool filterAcceptsRow(int row, const QModelIndex &parent) const override;
private:

  QMap<QString, CityFilter*> m_filters;
};


class CityDelegate: public QItemDelegate {
  Q_OBJECT

public:
  CityDelegate(QObject *parent = nullptr);

  void paint(QPainter *painter,
             const QStyleOptionViewItem &options,
             const QModelIndex &index) const override;
  QSize sizeHint(const QStyleOptionViewItem& options,
                 const QModelIndex & index) const override;

private:

  int m_height;
};


class CityView : public QDialog
{
  Q_OBJECT

public:
  CityView(QWidget* parent = nullptr);
  ~CityView();

  bool hasPrev(city* c) const;
  bool hasNext(city* c) const;
  city* next(city* c) const;
  city* prev(city* c) const;

signals:

  void orderingChanged();
  void manageCity(city* c);

private slots:

  void on_filterButton_clicked();
  void popupHeaderMenu(const QPoint& pos);
  void popupListMenu(const QPoint& pos);
  void gotoCity(const QModelIndex& cityIndex);

private:

  void readSettings();
  void writeSettings();

private:

  Ui::CityView *m_ui;
  CityFilterModel* m_filter;
  CityModel* m_cities;

};

}

