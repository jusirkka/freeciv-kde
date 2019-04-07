#pragma once

#include <QDialog>
#include <QAbstractItemModel>
#include <QStyledItemDelegate>

struct unit_type;

namespace Ui {
class UnitReport;
}

namespace KV {

class UnitTreeModel;

class UnitReport : public QDialog
{
  Q_OBJECT

public:
  explicit UnitReport(QWidget *parent = nullptr);

  ~UnitReport();

  void showAsTable();

private slots:

  void on_upgradeButton_clicked();

private:

  Ui::UnitReport *m_ui;
  UnitTreeModel* m_model;
};

class UnitTreeModel: public QAbstractItemModel {

  Q_OBJECT

  using TypeId = int;

public:

  using UnitId = int;
  using UnitVector = QVector<UnitId>;


  UnitTreeModel(QObject* parent = nullptr);

  bool canUpgrade() const;
  UnitVector upgradables() const;

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

  void initUnit(unit_type* utype);

private:

  using CityId = int;

  struct UnitData {
    UnitId unit;
    CityId city;
    int stepsFromCity;
  };

  struct Data {
    QVector<UnitData> units;
    QPixmap decoration;
  };
  using DataMap = QMap<TypeId, Data>;
  using DataIterator = QMapIterator<TypeId, Data>;

  using TypeVector = QVector<TypeId>;
  using CountMap = QMap<TypeId, QVector<int>>;
  using UpgradableMap = QMap<TypeId, UnitVector>;
  using UpgradableIterator = QMapIterator<TypeId, QVector<UnitId>>;

  const unsigned int rootId = 0xffffffff;

  const int InProgColumn = 1;
  const int ActiveColumn = 2;
  const int ShieldColumn = 3;
  const int FoodColumn = 4;
  const int GoldColumn = 5;
  const int NumColumns = 6;

  QVector<QString> m_headers;
  DataMap m_units;
  CountMap m_stats;
  UpgradableMap m_upgradables;
  TypeVector m_types;
};

class UnitDelegate: public QStyledItemDelegate {
  Q_OBJECT
public:
  UnitDelegate(QObject* parent = nullptr);
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









