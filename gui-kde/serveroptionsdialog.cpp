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
#include <QDirIterator>
#include <QMenu>
#include "inputbox.h"
#include "messagebox.h"

#include "options.h"
#include "chat.h"
#include "chatline_common.h"

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
  on_copyButton_clicked();
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
  bool preset = m_ui->copyButton->isEnabled();

  auto view = m_ui->pageView->findChild<QAbstractItemView*>();
  if (view != nullptr) {
    view->setEnabled(!editing);
  }
  m_ui->defaultsButton->setEnabled(!editing && canDefault);
  m_ui->resetButton->setEnabled(editing);
  m_ui->applyButton->setEnabled(editing);
  m_ui->closeButton->setEnabled(!editing);

  m_ui->pageView->setEnabled(!preset);

  m_ui->openButton->setEnabled(!editing);
  m_ui->saveButton->setEnabled(!editing && !preset);
  m_ui->deleteButton->setEnabled(true);
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

void ServerOptionsDialog::on_copyButton_clicked() {
  m_ui->copyButton->setEnabled(false);
  m_ui->nameLabel->setText("Untitled");
  updateState();
}

static void serverCommand(const QString& s) {
  auto cmd = QString("%1%2").arg(SERVER_COMMAND_PREFIX).arg(s);
  send_chat(cmd.toUtf8());
}

void ServerOptionsDialog::on_actionOpen_triggered() {
  QMenu menu;
  QDirIterator it(
        freeciv_storage_dir(),
        QStringList() << "*.serv",
        QDir::Files | QDir::Readable,
        QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
  while (it.hasNext()) {
    auto path = it.next();
    auto a = new QAction(it.fileInfo().completeBaseName());
    a->setData(path);
    menu.addAction(a);
  }

  auto a = menu.exec(QCursor::pos());
  if (a == nullptr) return;

  auto path = a->data().toString();
  // qCDebug(FC) << "/read" << path;
  serverCommand(QString("read %1").arg(path));

  m_ui->copyButton->setEnabled(true);
  m_ui->nameLabel->setText(a->text());
  updateState();
}

void ServerOptionsDialog::on_actionSave_triggered() {
  KV::InputBox ask(this,
                   QString(
                     _("What should we name the new server configuration file?\n"
                       "Subdirectories are created if needed and prefix appended.\n"
                       "E.g. path/to/example becomes example.serv in %1/path/to."))
                   .arg(freeciv_storage_dir()),
                   _("Save current server configuration"),
                   "server_config");

  if (ask.exec() != QDialog::Accepted) return;
  auto path = ask.input().trimmed();
  if (path.isEmpty()) return;
  path += ".serv";

  QDir storage(freeciv_storage_dir());

  // qCDebug(FC) << "/write" << storage.absoluteFilePath(path);
  storage.mkpath(QFileInfo(path).path());
  serverCommand(QString("write %1").arg(storage.absoluteFilePath(path)));

  m_ui->copyButton->setEnabled(true);
  m_ui->nameLabel->setText(QFileInfo(path).completeBaseName());
  updateState();
}

void ServerOptionsDialog::on_actionDelete_triggered() {
  QMenu menu;
  QDirIterator it(
        freeciv_storage_dir(),
        QStringList() << "*.serv",
        QDir::Files | QDir::Writable,
        QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
  while (it.hasNext()) {
    auto path = it.next();
    auto a = new QAction(it.fileInfo().completeBaseName());
    a->setData(path);
    menu.addAction(a);
  }

  auto a = menu.exec(QCursor::pos());
  if (a == nullptr) return;

  auto path = a->data().toString();
  // qCDebug(FC) << path;

  StandardMessageBox ask(this,
                         QString("Do you really want delete %1?")
                         .arg(path),
                         "Delete server config");
  if (ask.exec() == QMessageBox::Ok) {
    QFile(path).remove();
  }
}
