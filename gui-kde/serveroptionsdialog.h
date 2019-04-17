#pragma once

#include <QDialog>
#include <QModelIndex>

namespace Ui {
class ServerOptionsDialog;
}


namespace KV {

class OptionWidget;
class OptionModel;
class CategoriesFilter;

class ServerOptionsDialog: public QDialog {
    Q_OBJECT

public:

  ServerOptionsDialog(QWidget *parent = nullptr);

  void checkAndShow();
  void reset();

  ~ServerOptionsDialog();

private slots:

  void on_defaultsButton_clicked();
  void on_resetButton_clicked();
  void on_applyButton_clicked();

  void on_pageView_currentPageChanged(const QModelIndex& curr = QModelIndex(),
                                      const QModelIndex& prev = QModelIndex());
  void on_searchLine_textEdited(const QString& filter);

  void optionModel_edited(OptionWidget* opt, bool edited);
  void optionModel_defaulted(OptionWidget* opt, bool defaulted);

private:

  void updateState();

private:

  using OptionVector = QVector<OptionWidget*>;

  Ui::ServerOptionsDialog* m_ui;
  OptionVector m_edits;
  OptionVector m_nonDefaults;
  OptionModel* m_model;
  CategoriesFilter* m_filter;
};

}
