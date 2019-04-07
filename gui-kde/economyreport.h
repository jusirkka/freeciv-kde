#pragma once

#include <QDialog>
#include <QAbstractItemModel>
#include <QStyledItemDelegate>

struct universal;

namespace Ui {
class EconomyReport;
}

namespace KV {

class EconomyTreeModel;

class EconomyReport : public QDialog
{
  Q_OBJECT

public:
  explicit EconomyReport(QWidget *parent = nullptr);

  ~EconomyReport();

  void showAsTable();

private slots:

  void on_sellButton_clicked();
  void on_obsoleteButton_clicked();

private:

  Ui::EconomyReport *m_ui;
  EconomyTreeModel* m_model;
};

class EconomyTreeModel: public QAbstractItemModel {

  Q_OBJECT


public:

  using UniversalId = int;
  using CityId = int;
  using UnitId = int;
  using CityOrUnitId = int;
  using CityOrUnitVector = QVector<CityOrUnitId>;
  using CityOrUnitMap = QMap<UniversalId, CityOrUnitVector>;
  using CityOrUnitIterator = QMapIterator<UniversalId, CityOrUnitVector>;

  CityOrUnitMap selected() const;
  bool hasSelection() const;

  CityOrUnitMap obsoletes() const;
  bool hasObsoletes() const;

  EconomyTreeModel(QObject* parent = nullptr);

  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &index) const override;

  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  int rowCount(const QModelIndex &parent) const override;

  QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role) override;

  Qt::ItemFlags flags(const QModelIndex &index) const override;

public slots:

  void reset();

private:

  void initUniversal(UniversalId u);

private:

  struct CityData {
    CityId id;
    UnitId unit;
    bool redundant;
  };

  struct Data {
    QVector<CityData> cities;
    QPixmap decoration;
  };

  using DataMap = QMap<UniversalId, Data>;
  using DataIterator = QMapIterator<UniversalId, Data>;

  using UniversalVector = QVector<UniversalId>;
  using CountMap = QMap<UniversalId, QVector<int>>;
  using IndexVector = QVector<int>;
  using UniversalMap = QMap<UniversalId, IndexVector>;
  using UniversalIterator = QMapIterator<UniversalId, IndexVector>;

  const unsigned int rootId = 0xffffffff;

  const int RedundantColumn = 1;
  const int CostColumn = 2;
  const int CountColumn = 3;
  const int UpkeepColumn = 4;
  const int NumColumns = 5;

  QVector<QString> m_headers;
  DataMap m_universalMap;
  CountMap m_stats;
  UniversalMap m_selected;
  UniversalVector m_universals;
};

class EcoDelegate: public QStyledItemDelegate {
  Q_OBJECT
public:
  EcoDelegate(QObject* parent = nullptr);
protected:
  void paint(QPainter *p,
             const QStyleOptionViewItem &opt,
             const QModelIndex &index) const override;
  bool editorEvent(QEvent *ev,
                   QAbstractItemModel *model,
                   const QStyleOptionViewItem &opt,
                   const QModelIndex &index) override;
};

}
