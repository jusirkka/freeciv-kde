#pragma once

#include <KPageModel>
#include <QSortFilterProxyModel>
#include <QWidget>

struct option_set;
struct option;
class QHBoxLayout;
class QCheckBox;
class QSpinBox;
class QLineEdit;
class QComboBox;
class QPushButton;
class QLabel;
class QScrollArea;

namespace KV {

class OptionWidget;

class OptionModel: public KPageModel
{
  Q_OBJECT

public:

  OptionModel(const option_set* oset, QObject *parent = nullptr);

  QModelIndex index(int row, int column, const QModelIndex &parent) const override;
  QModelIndex parent(const QModelIndex &child) const override;
  int rowCount(const QModelIndex &index = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public slots:

  void checkOptions(const QModelIndex& pageIndex);

signals:

  void edited(OptionWidget* opt, bool yes);
  void defaulted(OptionWidget* opt, bool yes);

private slots:

  void updateOption(const void* opt);

private:

  static OptionWidget* createOptionWidget(option* opt);

  using PageMap = QMap<QString, QScrollArea*>;
  using PageIterator = QMapIterator<QString, QScrollArea*>;
  using IconNameMap = QMap<QString, QString>;

  const option_set* m_optionSet;
  PageMap m_categories;
  QStringList m_names;
  IconNameMap m_icons;
};

// CategoriesFilter

class CategoriesFilter: public QSortFilterProxyModel {
  Q_OBJECT
public:
  CategoriesFilter(QObject* parent = nullptr);
  bool filterAcceptsRow(int row, const QModelIndex &parent) const override;
};


// OptionWidget interface

class OptionWidget: public QWidget {
  Q_OBJECT
public:
  OptionWidget(option* opt, QWidget* parent = nullptr);

  const QString& description() const;
  bool highlight(const QRegularExpression& re);


  void reset();
  void apply();
  void defaultIt();
  bool defaultable() const;
  bool enablable() const;

protected:

  virtual void doReset() = 0;
  virtual void doApply() = 0;
  virtual void doDefault() = 0;
  virtual bool canDefault() const = 0;
  virtual bool canEnable() const;
  virtual bool canChange() const = 0;

  static bool inBlacklist(const QString& name);

signals:

  void edited(bool yes);
  void defaulted(bool yes);

protected:

  QHBoxLayout* m_lay;
  QLabel* m_label;
  QString m_description;
  option* m_option;
};

class BooleanWidget: public OptionWidget {
  Q_OBJECT
public:
  BooleanWidget(option* opt, QWidget* parent = nullptr);
protected:
  void doReset() override;
  void doApply() override;
  void doDefault() override;
  bool canDefault() const override;
  bool canChange() const override;
private:
  QCheckBox* m_box;
};

class IntegerWidget: public OptionWidget {
  Q_OBJECT
public:
  IntegerWidget(option* opt, QWidget* parent = nullptr);
protected:
  void doReset() override;
  void doApply() override;
  void doDefault() override;
  bool canDefault() const override;
  bool canChange() const override;
private:
  QSpinBox* m_box;
};

class StringWidget: public OptionWidget {
  Q_OBJECT
public:
  StringWidget(option* opt, QWidget* parent = nullptr);
protected:
  void doReset() override;
  void doApply() override;
  void doDefault() override;
  bool canDefault() const override;
  bool canChange() const override;
private:
  QLineEdit* m_line;
};

class StringComboWidget: public OptionWidget {
  Q_OBJECT
public:
  StringComboWidget(option* opt, QWidget* parent = nullptr);
protected:
  void doReset() override;
  void doApply() override;
  void doDefault() override;
  bool canDefault() const override;
  bool canChange() const override;
private:
  QComboBox* m_box;
};

class EnumWidget: public OptionWidget {
  Q_OBJECT
public:
  EnumWidget(option* opt, QWidget* parent = nullptr);
protected:
  void doReset() override;
  void doApply() override;
  void doDefault() override;
  bool canDefault() const override;
  bool canChange() const override;
private:
  QComboBox* m_box;
};


class ColorWidget: public OptionWidget {
  Q_OBJECT
public:
  ColorWidget(option* opt, QWidget* parent = nullptr);
protected:
  void doReset() override;
  void doApply() override;
  void doDefault() override;
  bool canDefault() const override;
  bool canChange() const override;
private slots:
  void buttonClicked();
private:
  void setColor(QPushButton* but, const QColor& c);
private:
  QPushButton* m_fgButton;
  QPushButton* m_bgButton;
};

}
