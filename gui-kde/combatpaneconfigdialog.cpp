#include "combatpaneconfigdialog.h"
#include "ui_combatpaneconfigdialog.h"
#include "conf_combatpane.h"
#include <QWhatsThis>
#include <QAction>

using namespace KV;

CombatPaneConfigDialog::CombatPaneConfigDialog(QWidget *parent) :
  QDialog(parent),
  m_ui(new Ui::CombatPaneConfigDialog)
{
  m_ui->setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle(QString("%1: Configure combat pane").arg(qAppName()));
  m_ui->spinBox->setValue(Conf::CombatPane::combatsCount());

  auto a = QWhatsThis::createAction(this);
  a->setIcon(QIcon::fromTheme("help-contextual"));
  m_ui->toolButton->setDefaultAction(a);
}

CombatPaneConfigDialog::~CombatPaneConfigDialog()
{
  delete m_ui;
}

int CombatPaneConfigDialog::combatCount() {
  return m_ui->spinBox->value();
}
