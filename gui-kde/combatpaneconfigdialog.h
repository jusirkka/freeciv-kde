#pragma once

#include <QDialog>

namespace Ui {
class CombatPaneConfigDialog;
}

namespace KV {

class CombatPaneConfigDialog : public QDialog
{
  Q_OBJECT

public:
  explicit CombatPaneConfigDialog(QWidget *parent = nullptr);
  ~CombatPaneConfigDialog();

  int combatCount();

private:
  Ui::CombatPaneConfigDialog *m_ui;
};

}
