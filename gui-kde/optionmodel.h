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

  QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &child) const override;
  int rowCount(const QModelIndex &index = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  void reset();

protected:

  OptionModel(const option_set *oset, QObject *parent = nullptr);

public slots:

  void checkOptions(const QModelIndex& pageIndex);
  void addCategory(const QString& name);

signals:

  void edited(OptionWidget* opt, bool yes);
  void defaulted(OptionWidget* opt, bool yes);

protected slots:

  void updateOption(const void* opt);
  void addOption(void* opt);
  void delOption(const void* opt);

protected:

  static OptionWidget* createOptionWidget(option* opt);

  using PageMap = QMap<QString, QScrollArea*>;
  using PageIterator = QMapIterator<QString, QScrollArea*>;
  using IconNameMap = QMap<QString, QString>;

  const option_set* m_optionSet;
  PageMap m_categories;
  QStringList m_names;
  IconNameMap m_icons;
};

class ServerOptionModel: public OptionModel {
  Q_OBJECT
public:
  ServerOptionModel(QObject *parent = nullptr);
};

class LocalOptionModel: public OptionModel {
  Q_OBJECT
public:
  LocalOptionModel(QObject *parent = nullptr);
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


  void reset(); // proxy to editor
  void apply(); // editor to proxy & options
  void updateIt(); // options to proxy & editor
  void defaultIt(); // defaults to proxy, editor & options
  bool defaultable() const;
  bool enablable() const;

protected:

  virtual void doReset() = 0;
  virtual void doApply() = 0;
  virtual void doDefault() = 0;
  virtual void doUpdate() = 0;
  virtual bool canDefault() const = 0;
  virtual bool canEnable() const;
  virtual bool canApply() const = 0;

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
  void doUpdate() override;
  bool canDefault() const override;
  bool canApply() const override;
private:
  QCheckBox* m_box;
  bool m_proxy;
};

class IntegerWidget: public OptionWidget {
  Q_OBJECT
public:
  IntegerWidget(option* opt, QWidget* parent = nullptr);
protected:
  void doReset() override;
  void doApply() override;
  void doDefault() override;
  void doUpdate() override;
  bool canDefault() const override;
  bool canApply() const override;
private:
  QSpinBox* m_box;
  int m_proxy;
};

class StringWidget: public OptionWidget {
  Q_OBJECT
public:
  StringWidget(option* opt, QWidget* parent = nullptr);
protected:
  void doReset() override;
  void doApply() override;
  void doDefault() override;
  void doUpdate() override;
  bool canDefault() const override;
  bool canApply() const override;
private:
  QLineEdit* m_line;
  QString m_proxy;
};

class StringComboWidget: public OptionWidget {
  Q_OBJECT
public:
  StringComboWidget(option* opt, QWidget* parent = nullptr);
protected:
  void doReset() override;
  void doApply() override;
  void doDefault() override;
  void doUpdate() override;
  bool canDefault() const override;
  bool canApply() const override;
private:
  QComboBox* m_box;
  QString m_proxy;
};

class EnumWidget: public OptionWidget {
  Q_OBJECT
public:
  EnumWidget(option* opt, QWidget* parent = nullptr);
protected:
  void doReset() override;
  void doApply() override;
  void doDefault() override;
  void doUpdate() override;
  bool canDefault() const override;
  bool canApply() const override;
private:
  QComboBox* m_box;
  QString m_proxy;
};

class FlagsWidget: public OptionWidget {
  Q_OBJECT
public:
  FlagsWidget(option* opt, QWidget* parent = nullptr);
protected:
  void doReset() override;
  void doApply() override;
  void doDefault() override;
  void doUpdate() override;
  bool canDefault() const override;
  bool canApply() const override;
private:
  unsigned value() const;
private:
  QVector<QCheckBox*> m_boxes;
  unsigned m_proxy;
};

class ColorWidget: public OptionWidget {
  Q_OBJECT
public:
  ColorWidget(option* opt, QWidget* parent = nullptr);
protected:
  void doReset() override;
  void doApply() override;
  void doDefault() override;
  void doUpdate() override;
  bool canDefault() const override;
  bool canApply() const override;
private slots:
  void buttonClicked();
private:
  void setColor(QPushButton* but, const QColor& c);
private:
  QPushButton* m_fgButton;
  QPushButton* m_bgButton;
  QColor m_fgProxy;
  QColor m_bgProxy;
};

}
