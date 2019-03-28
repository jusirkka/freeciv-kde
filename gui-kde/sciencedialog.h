#pragma once

#include <QDialog>

namespace Ui {
class ScienceDialog;
}

namespace KV {


class ResearchTreeWidget;

class ScienceDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ScienceDialog(QWidget *parent = nullptr);
  ~ScienceDialog();

public slots:

  void updateReport();

private slots:

  void on_researchingCombo_currentIndexChanged(int idx);
  void on_goalCombo_currentIndexChanged(int idx);

private:
  Ui::ScienceDialog *m_ui;
  ResearchTreeWidget* m_researchTree;
};


} // namespace KV
