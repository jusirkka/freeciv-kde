#include "serveroptionsdialog.h"
#include "ui_serveroptionsdialog.h"
#include <KConfigGroup>
#include <KWindowConfig>
#include <KSharedConfig>
#include <QWindow>
#include <QWhatsThis>
#include <QAction>
#include <QAbstractItemView>
#include "logging.h"
#include <KPageWidgetModel>
#include "optionmodel.h"

#include "options.h"

using namespace KV;

ServerOptionsDialog::ServerOptionsDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::ServerOptionsDialog)
    , m_model(new ServerOptionModel)
    , m_filter(new CategoriesFilter)
{
  m_ui->setupUi(this);
  setWindowTitle(QString("%1: Server Options").arg(qAppName()));

  m_ui->openButton->setDefaultAction(m_ui->actionOpen);
  m_ui->saveButton->setDefaultAction(m_ui->actionSave);
  m_ui->deleteButton->setDefaultAction(m_ui->actionDelete);

  auto a = QWhatsThis::createAction(this);
  a->setIcon(QIcon::fromTheme("help-contextual"));
  m_ui->helpButton->setDefaultAction(a);

  m_filter->setSourceModel(m_model);
  m_ui->pageView->setModel(m_filter);

  connect(m_model, &OptionModel::edited,
          this, &ServerOptionsDialog::optionModel_edited);
  connect(m_model, &OptionModel::defaulted,
          this, &ServerOptionsDialog::optionModel_defaulted);

  create(); // ensure there's a window created
  const KConfigGroup cnf(KSharedConfig::openConfig(), "ServerOptionsDialog");
  KWindowConfig::restoreWindowSize(windowHandle(), cnf);
  resize(windowHandle()->size());

  on_pageView_currentPageChanged();
}

ServerOptionsDialog::~ServerOptionsDialog() {
  KConfigGroup cnf(KSharedConfig::openConfig(), "ServerOptionsDialog");
  KWindowConfig::saveWindowSize(windowHandle(), cnf);
  delete m_ui;
}

void ServerOptionsDialog::checkAndShow() {
  on_pageView_currentPageChanged();
  show();
}

void ServerOptionsDialog::reset() {
  m_model->reset();
}

void ServerOptionsDialog::on_searchLine_textEdited(const QString &s) {
  if (s.length() < 3) {
    m_filter->setFilterRegularExpression(QRegularExpression());
  } else {
    m_filter->setFilterRegularExpression(QRegularExpression(s, QRegularExpression::CaseInsensitiveOption));
  }
}

void ServerOptionsDialog::optionModel_edited(OptionWidget *opt, bool edited) {
  if (edited && !m_edits.contains(opt)) {
    // qCDebug(FC) << "adding" << opt->description() << "to edits";
    m_edits.append(opt);
  } else if (!edited && m_edits.contains(opt)) {
    // qCDebug(FC) << "removing" << opt->description() << "from edits";
    m_edits.removeAll(opt);
  }
  updateState();
}

void ServerOptionsDialog::optionModel_defaulted(OptionWidget *opt, bool defaulted) {
  if (defaulted && m_nonDefaults.contains(opt)) {
    qCDebug(FC) << "removing" << opt->description() << "from non-defaults";
    m_nonDefaults.removeAll(opt);
  } else if (!defaulted && !m_edits.contains(opt)) {
    qCDebug(FC) << "adding" << opt->description() << "to non-defaults";
    m_nonDefaults.append(opt);
  }
  updateState();
}

void ServerOptionsDialog::on_pageView_currentPageChanged(const QModelIndex& curr,
                                                        const QModelIndex& prev) {
  auto idx = m_filter->mapToSource(m_ui->pageView->currentPage());
  if (!idx.isValid()) return;

  m_model->checkOptions(idx);

  if (!m_edits.isEmpty()) {
    if (prev.isValid() && curr != prev) {
      qCWarning(FC) << "applying unsaved edits";
      on_applyButton_clicked();
    }
  }
  m_edits.clear();

  m_nonDefaults.clear();
  auto page = idx.data(KPageModel::WidgetRole).value<QWidget*>();
  auto opts = page->findChildren<OptionWidget*>();
  for (auto opt: opts) {
    if (opt->defaultable()) {
      qCDebug(FC) << "adding" << opt->description() << "to non-defaults";
      m_nonDefaults.append(opt);
    }
  }
  updateState();
}

void ServerOptionsDialog::updateState() {
  bool editing = !m_edits.isEmpty();
  bool canDefault = !m_nonDefaults.isEmpty();

  auto view = m_ui->pageView->findChild<QAbstractItemView*>();
  if (view != nullptr) {
    view->setEnabled(!editing);
  }
  m_ui->defaultsButton->setEnabled(!editing && canDefault);
  m_ui->resetButton->setEnabled(editing);
  m_ui->applyButton->setEnabled(editing);
  m_ui->closeButton->setEnabled(!editing);
}

void ServerOptionsDialog::on_defaultsButton_clicked() {
  while (!m_nonDefaults.isEmpty()) {
    auto opt = m_nonDefaults.takeFirst();
    qCDebug(FC) << "defaulting" << opt->description();
    opt->defaultIt();
  }
  desired_settable_options_update();
  options_save(nullptr);
  updateState();
}

void ServerOptionsDialog::on_resetButton_clicked() {
  while (!m_edits.isEmpty()) {
    auto opt = m_edits.takeFirst();
    opt->reset();
  }
  desired_settable_options_update();
  options_save(nullptr);
  updateState();
}

void ServerOptionsDialog::on_applyButton_clicked() {
  while (!m_edits.isEmpty()) {
    auto opt = m_edits.takeFirst();
    opt->apply();
  }
  desired_settable_options_update();
  options_save(nullptr);
  updateState();
}


