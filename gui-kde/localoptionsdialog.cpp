#include "localoptionsdialog.h"
#include "ui_localoptionsdialog.h"
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

LocalOptionsDialog::LocalOptionsDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::LocalOptionsDialog)
    , m_model(new LocalOptionModel)
    , m_filter(new CategoriesFilter)
{
  m_ui->setupUi(this);
  setWindowTitle(QString("%1: Local Options").arg(qAppName()));

  auto a = QWhatsThis::createAction(this);
  a->setIcon(QIcon::fromTheme("help-contextual"));
  m_ui->helpButton->setDefaultAction(a);

  m_filter->setSourceModel(m_model);
  m_ui->pageView->setModel(m_filter);

  connect(m_model, &OptionModel::edited,
          this, &LocalOptionsDialog::optionModel_edited);
  connect(m_model, &OptionModel::defaulted,
          this, &LocalOptionsDialog::optionModel_defaulted);

  create(); // ensure there's a window created
  const KConfigGroup cnf(KSharedConfig::openConfig(), "LocalOptionsDialog");
  KWindowConfig::restoreWindowSize(windowHandle(), cnf);
  resize(windowHandle()->size());

  on_pageView_currentPageChanged();
}

LocalOptionsDialog::~LocalOptionsDialog() {
  KConfigGroup cnf(KSharedConfig::openConfig(), "LocalOptionsDialog");
  KWindowConfig::saveWindowSize(windowHandle(), cnf);
  delete m_ui;
}

void LocalOptionsDialog::checkAndShow() {
  on_pageView_currentPageChanged();
  show();
}

void LocalOptionsDialog::on_searchLine_textEdited(const QString &s) {
  if (s.length() < 3) {
    m_filter->setFilterRegularExpression(QRegularExpression());
  } else {
    m_filter->setFilterRegularExpression(QRegularExpression(s, QRegularExpression::CaseInsensitiveOption));
  }
}

void LocalOptionsDialog::optionModel_edited(OptionWidget *opt, bool edited) {
  if (edited && !m_edits.contains(opt)) {
    // qCDebug(FC) << "adding" << opt->description() << "to edits";
    m_edits.append(opt);
  } else if (!edited && m_edits.contains(opt)) {
    // qCDebug(FC) << "removing" << opt->description() << "from edits";
    m_edits.removeAll(opt);
  }
  updateState();
}

void LocalOptionsDialog::optionModel_defaulted(OptionWidget *opt, bool defaulted) {
  if (defaulted && m_nonDefaults.contains(opt)) {
    // qCDebug(FC) << "removing" << opt->description() << "from defaults";
    m_nonDefaults.removeAll(opt);
  } else if (!defaulted && !m_edits.contains(opt)) {
    // qCDebug(FC) << "adding" << opt->description() << "to defaults";
    m_nonDefaults.append(opt);
  }
  updateState();
}

void LocalOptionsDialog::on_pageView_currentPageChanged(const QModelIndex& curr,
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
      // qCDebug(FC) << "adding" << opt->description() << "to defaults";
      m_nonDefaults.append(opt);
    }
  }
  updateState();
}

void LocalOptionsDialog::updateState() {
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

void LocalOptionsDialog::on_defaultsButton_clicked() {
  while (!m_nonDefaults.isEmpty()) {
    auto opt = m_nonDefaults.takeFirst();
    opt->defaultIt();
  }
  desired_settable_options_update();
  options_save(nullptr);
  updateState();
}

void LocalOptionsDialog::on_resetButton_clicked() {
  while (!m_edits.isEmpty()) {
    auto opt = m_edits.takeFirst();
    opt->reset();
  }
  desired_settable_options_update();
  options_save(nullptr);
  updateState();
}

void LocalOptionsDialog::on_applyButton_clicked() {
  while (!m_edits.isEmpty()) {
    auto opt = m_edits.takeFirst();
    opt->apply();
  }
  desired_settable_options_update();
  options_save(nullptr);
  updateState();
}


