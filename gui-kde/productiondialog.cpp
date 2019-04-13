#include "productiondialog.h"
#include "ui_productiondialog.h"
#include "application.h"
#include "cityview.h"
#include "buildables.h"
#include "workmodel.h"
#include <QMenu>
#include <QMimeData>
#include "inputbox.h"
#include "logging.h"
#include "messagebox.h"
#include "citydialog.h"

#include "city.h"
#include "client_main.h"
#include "global_worklist.h"
#include "citydlg_common.h"

using namespace KV;

ProductionDialog::ProductionDialog(CityView* cities, QWidget *parent)
  : QDialog(parent)
  , m_ui(new Ui::ProductionDialog)
  , m_cities(cities)
{
  m_ui->setupUi(this);
  setWindowFlag(Qt::WindowStaysOnTopHint, false);
  setWindowFlag(Qt::Dialog, false);
  setWindowFlag(Qt::Window, true);

  // cut, copy, paste, open, save, delete
  m_ui->cutButton->setDefaultAction(m_ui->actionCut);
  m_ui->copyButton->setDefaultAction(m_ui->actionCopy);
  m_ui->pasteButton->setDefaultAction(m_ui->actionPaste);
  m_ui->openButton->setDefaultAction(m_ui->actionOpen);
  m_ui->saveButton->setDefaultAction(m_ui->actionSave);
  m_ui->deleteButton->setDefaultAction(m_ui->actionDelete);

  m_ui->actionCut->setDisabled(true);
  m_ui->actionCopy->setDisabled(true);
  m_ui->actionPaste->setDisabled(true);
  m_ui->actionOpen->setEnabled(can_client_issue_orders());
  m_ui->actionSave->setEnabled(can_client_issue_orders());
  m_ui->actionDelete->setEnabled(can_client_issue_orders());


  // production header
  connect(this, &ProductionDialog::cityChanged,
          m_ui->productionHeader, &ProductionHeader::changeCity);

  // worklist
  m_workModel = new WorkModel(this);
  m_ui->workList->setModel(m_workModel);
  connect(this, &ProductionDialog::cityChanged, this, [=] (city* c) {
    m_workModel->changeCity(c);
    m_ui->workList->reset();
  });

  m_ui->workList->header()->setSectionResizeMode(0, QHeaderView::Stretch);
  m_ui->workList->header()->setSectionResizeMode(1, QHeaderView::Fixed);
  m_ui->workList->header()->resizeSection(1, fontMetrics().width("9999") + 4);

  connect(m_ui->workList->selectionModel(), &QItemSelectionModel::selectionChanged,
          this, &ProductionDialog::updateEditButtons);

  // filter checkbuttons
  m_ui->futureCheckBox->setIcon(Application::Icon("future"));
  m_ui->unitsCheckBox->setIcon(Application::Icon("units"));
  m_ui->buildingsCheckBox->setIcon(Application::Icon("building"));
  m_ui->wondersCheckBox->setIcon(Application::Icon("wonder"));

  m_ui->futureCheckBox->setText("");
  m_ui->unitsCheckBox->setText("");
  m_ui->buildingsCheckBox->setText("");
  m_ui->wondersCheckBox->setText("");

  // buildables
  auto buildables = new BuildablesModel(m_city, this);
  int flags =
      BuildablesFilter::Buildings |
      BuildablesFilter::Units |
      BuildablesFilter::Wonders;
  m_filter = new BuildablesFilter(m_city, flags, this);
  m_filter->setSourceModel(buildables);
  auto source = new BuildablesDragModel(this);
  source->setSourceModel(m_filter);
  m_ui->buildablesList->setModel(source);

  connect(this, &ProductionDialog::cityChanged, this, [=] (city* c) {
    m_filter->changeCity(c);
    m_ui->buildablesList->reset();
  });

  // next/prev buttons
  m_ui->nextButton->setEnabled(false);
  m_ui->previousButton->setEnabled(false);


  connect(m_cities, &CityView::orderingChanged,
          this, &ProductionDialog::updateCityButtons);

  connect(m_ui->nextButton, &QPushButton::clicked, this, [=] () {
    changeCity(m_cities->next(m_city));
  });

  connect(m_ui->previousButton, &QPushButton::clicked, this, [=] () {
    changeCity(m_cities->prev(m_city));
  });

}


void ProductionDialog::changeCity(city *c) {

  // cut, copy, paste
  m_ui->workList->selectionModel()->clear();
  updateEditButtons(QItemSelection(), QItemSelection());
  if (c != m_city) {
    delete m_clipboard;
    m_clipboard = nullptr;
    m_ui->actionPaste->setDisabled(true);
  }

  m_city = c;

  emit cityChanged(m_city);

  // open, save
  m_ui->actionOpen->setEnabled(can_client_issue_orders());
  m_ui->actionSave->setEnabled(can_client_issue_orders());
  m_ui->actionDelete->setEnabled(can_client_issue_orders());
  // production header - signalled
  // worklist - signalled
  // filter buttons - inert to city changes
  // buildables - signalled
  // next/prev buttons
  updateCityButtons();

  setWindowTitle(QString("%1: Production").arg(CityDialog::Title(m_city)));
}

void ProductionDialog::refresh(city* c) {
  if (c == m_city) {
    changeCity(c);
  }
}

void ProductionDialog::updateCityButtons() {

  bool canGo = m_cities->hasNext(m_city);
  m_ui->nextButton->setEnabled(canGo);
  m_ui->nextButton->setText(canGo ? city_name_get(m_cities->next(m_city)) : "Next");

  canGo = m_cities->hasPrev(m_city);
  m_ui->previousButton->setEnabled(canGo);
  m_ui->previousButton->setText(canGo ? city_name_get(m_cities->prev(m_city)) : "Previous");

}


void ProductionDialog::updateEditButtons(const QItemSelection &s,
                                         const QItemSelection &) {
  bool noCanDo = s.isEmpty() || client_is_observer();

  m_ui->actionCut->setDisabled(noCanDo || m_workModel->rowCount() == 1);
  m_ui->actionCopy->setDisabled(noCanDo);
}

void ProductionDialog::on_actionCut_triggered() {

  auto selection = m_ui->workList->selectionModel()->selection();
  if (selection.isEmpty()) return;

  delete m_clipboard;
  m_clipboard = m_workModel->mimeData(selection.indexes());
  m_ui->actionPaste->setEnabled(true);

  for (auto s: selection.indexes()) {
    if (s.column() > 0) continue;
    m_workModel->removeRow(s.row());
  }
}

void ProductionDialog::on_actionCopy_triggered() {
  auto selection = m_ui->workList->selectionModel()->selection();
  if (selection.isEmpty()) return;

  delete m_clipboard;
  QModelIndexList sel;
  for (auto s: selection.indexes()) {
    if (s.column() > 0) continue;
    sel << s;
  }
  m_clipboard = m_workModel->mimeData(sel);
  m_ui->actionPaste->setEnabled(true);

}

void ProductionDialog::on_actionPaste_triggered() {
  auto selection = m_ui->workList->selectionModel()->selection();
  if (selection.isEmpty()) {
    m_workModel->dropMimeData(m_clipboard,
                              Qt::CopyAction,
                              m_workModel->rowCount(), 0,
                              QModelIndex());
  } else {
    for (auto s: selection.indexes()) {
      if (s.column() > 0) continue;
      m_workModel->dropMimeData(m_clipboard,
                                Qt::CopyAction,
                                selection.indexes().first().row(), 0,
                                QModelIndex());
    }
  }
}

void ProductionDialog::on_actionOpen_triggered() {
  QMenu menu;
  global_worklists_iterate(wl) {
    auto a = new QAction(global_worklist_name(wl));
    connect(a, &QAction::triggered, this, [=] () {
      city_set_queue(m_city, global_worklist_get(wl));
    });
    menu.addAction(a);
  } global_worklists_iterate_end;
  menu.exec(QCursor::pos());
}

void ProductionDialog::on_actionSave_triggered() {
  KV::InputBox ask(this,
                   _("What should we name new worklist?"),
                   _("Save current worklist"),
                   _("New worklist"));
  if (ask.exec() == QDialog::Accepted) {
    auto name = ask.input();
    if (!name.isEmpty()) {
      auto gw = global_worklist_new(name.toUtf8());
      worklist queue;
      city_get_queue(m_city, &queue);
      global_worklist_set(gw, &queue);
    }
  }
}

void ProductionDialog::on_actionDelete_triggered() {
  QMenu menu;
  global_worklists_iterate(wl) {
    int id = global_worklist_id(wl);
    auto a = new QAction(global_worklist_name(wl));
    connect(a, &QAction::triggered, this, [this, id] () {
      StandardMessageBox ask(this,
                             QString("Do you really want delete %1?")
                             .arg(global_worklist_name(global_worklist_by_id(id))),
                             "Delete worklist");
      if (ask.exec() == QMessageBox::Ok) {
        global_worklist_destroy(global_worklist_by_id(id));
      }
    });
    menu.addAction(a);
  } global_worklists_iterate_end;

  menu.exec(QCursor::pos());
}

static int updateFlag(int flags, int bit, bool on) {
  if (on) {
    flags |= bit;
  } else {
    flags &= ~ bit;
  }
  return flags;
}

void ProductionDialog::on_futureCheckBox_toggled(bool on) {
  m_filter->setFilterFlags(updateFlag(m_filter->filterFlags(), BuildablesFilter::Future, on));
}

void ProductionDialog::on_unitsCheckBox_toggled(bool on) {
  m_filter->setFilterFlags(updateFlag(m_filter->filterFlags(), BuildablesFilter::Units, on));
}

void ProductionDialog::on_wondersCheckBox_toggled(bool on) {
  m_filter->setFilterFlags(updateFlag(m_filter->filterFlags(), BuildablesFilter::Wonders, on));
}

void ProductionDialog::on_buildingsCheckBox_toggled(bool on) {
  m_filter->setFilterFlags(updateFlag(m_filter->filterFlags(), BuildablesFilter::Buildings, on));
}

ProductionDialog::~ProductionDialog() {
  delete m_ui;
}
