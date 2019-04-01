#pragma once

#include <QDialog>
#include <QVector>
#include <QAbstractListModel>
#include "options.h"


namespace Ui {
class MessageConfigDialog;
}

namespace KV {

class MessageConfigModel;

class MessageConfigDialog: public QDialog {
  Q_OBJECT

public:

  MessageConfigDialog(int flag, const QString& title, QWidget* parent = nullptr);
  ~MessageConfigDialog();

private slots:

  void saveConfig();

private:

  Ui::MessageConfigDialog* m_ui;
  int m_flag;
  MessageConfigModel* m_model;

};

class MessageConfigModel: public QAbstractListModel {
  Q_OBJECT
public:
  explicit MessageConfigModel(int flags, QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;


private:

  const int m_rowCount = E_COUNT;
  QVector<bool> m_checked;
};

} // namespace KV

