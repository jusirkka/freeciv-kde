#include "filedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <KFileWidget>
#include <QUrl>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include "logging.h"
#include <QDateTime>
#include <KUrlComboBox>
#include <QLineEdit>

#include "registry_ini.h"

using namespace KV;

FileDialog::FileDialog(const QStringList &dirs,
                       KFileWidget::OperationMode mode,
                       QWidget* parent)
  : QDialog(parent)
{
  setWindowTitle("Load Freeciv Game/Scenario");
  auto lay = new QVBoxLayout;
  setLayout(lay);
  m_main = new KFileWidget(QUrl(), this);
  m_main->setOperationMode(mode);
  m_main->setFilter("*.sav *.sav.*|Saved freeciv games & scenarios");
  if (mode == KFileWidget::Opening) {
    m_main->setMode(KFile::ExistingOnly | KFile::LocalOnly | KFile::File);
    m_main->setPreviewWidget(new SavedGamePreview);
  } else if (mode == KFileWidget::Saving) {
    m_main->setMode(KFile::LocalOnly | KFile::File);
    m_main->setConfirmOverwrite(true);
    m_main->locationEdit()->clear();
  }

  if (!dirs.isEmpty()) {
    m_main->setUrl(QUrl::fromLocalFile(QFileInfo(dirs.first()).canonicalFilePath()));
  }

  if (dirs.size() > 1) {
    auto left = new QVBoxLayout;
    left->addWidget(new QLabel("Game Folders"));
    auto view = new QListWidget;
    left->addWidget(view);
    auto upper = new QHBoxLayout;
    upper->addLayout(left);
    upper->addWidget(m_main);
    upper->setStretch(0, 1);
    upper->setStretch(1, 6);
    lay->addLayout(upper);
    for (auto dir: dirs) {
      auto item = new QListWidgetItem(QIcon::fromTheme("document-open-folder"),
                                      QFileInfo(dir).canonicalFilePath());
      view->addItem(item);
    }
    view->setTextElideMode(Qt::ElideLeft);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(view, &QListWidget::itemClicked, this, [this] (QListWidgetItem* item) {
      m_main->setUrl(QUrl::fromLocalFile(item->text()));
    });
  } else {
    lay->addWidget(m_main);
  }

  auto buttons = new QHBoxLayout;
  buttons->addItem(new QSpacerItem(5, 5, QSizePolicy::Expanding, QSizePolicy::Minimum));

  QPushButton* ok;
  if (mode == KFileWidget::Opening) {
    ok = new QPushButton(QIcon::fromTheme("document-open"), "Open");
  } else if (mode == KFileWidget::Saving) {
    ok = new QPushButton(QIcon::fromTheme("document-save"), "Save");
  } else {
    ok = new QPushButton(QIcon::fromTheme("dialog-ok"), "Ok");
  }

  connect(ok, &QPushButton::clicked, m_main, &KFileWidget::slotOk);
  connect(ok, &QPushButton::clicked, m_main, &KFileWidget::accept);
  connect(ok, &QPushButton::clicked, this, &FileDialog::accept);
  buttons->addWidget(ok);
  auto cancel = new QPushButton(QIcon::fromTheme("dialog-cancel"), "Cancel");
  buttons->addWidget(cancel);
  connect(cancel, &QPushButton::clicked, this, &FileDialog::reject);

  lay->addLayout(buttons);
}

QString FileDialog::selectedFile() const {
  return m_main->selectedFile();
}


SavedGamePreview::SavedGamePreview(QWidget *parent)
  : KPreviewWidgetBase(parent)
  , m_label(new QLabel)
{
  m_label->setWordWrap(true);
  auto lay = new QVBoxLayout;
  setLayout(lay);
  lay->addWidget(m_label);
  lay->addItem(new QSpacerItem(5, 5, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

void SavedGamePreview::showPreview(const QUrl &url) {
  QString f = url.toLocalFile();
  if (f.isEmpty()) return;

  auto section = secfile_load_section(f.toUtf8(), "scenario", true);
  if (section == nullptr) return;
  bool scenario = secfile_lookup_bool_default(section, false, "scenario.is_scenario");
  if (scenario) {
    showScenario(f);
  } else {
    showSaved(f);
  }
}

void SavedGamePreview::showSaved(const QString &f) {

  auto line = QString("<b>%1</b>: %2<br/>");
  QString info = line.arg("Last Modified").arg(QFileInfo(f).lastModified().toString("ddd MMMM d hh:mm"));

  auto section = secfile_load_section(f.toUtf8(), "game", true);
  if (section != nullptr) {
    int turn = secfile_lookup_int_default(section, -1, "game.turn");
    info += line.arg("Turn").arg(turn);
  }

  section = secfile_load_section(f.toUtf8(), "players", true);
  if (section != nullptr) {
    int nplayers = secfile_lookup_int_default(section, -1, "players.nplayers");
    // off by one bug in some files
    for (int i = 0; i < nplayers + 1; i++) {
      auto player = QString("player%1").arg(i);
      section = secfile_load_section(f.toUtf8(), player.toUtf8(), true);
      if (section != nullptr) {
        auto name = secfile_lookup_str_default(section, "N/A",
                                               "player%d.name", i);
        auto ai = secfile_lookup_bool_default(section, true,
                                              "player%d.unassigned_user", i);
        info += line.arg(name).arg(ai ? "AI" : "Human");
      }
    }
  }

  m_label->setText(info);
}

void SavedGamePreview::showScenario(const QString &f) {
  auto line = QString("<b>%1</b>: %2<br/>");
  QString info;

  auto section = secfile_load_section(f.toUtf8(), "scenario", true);
  if (section == nullptr) return;
  auto name = secfile_lookup_str_default(section, "N/A", "scenario.name");
  info += line.arg("Name").arg(name);
  auto descr = secfile_lookup_str_default(section, "N/A", "scenario.description");
  info += line.arg("Description").arg(descr);
  auto auts = secfile_lookup_str_default(section, "N/A", "scenario.authors");
  info += line.arg("Authors").arg(auts);
  m_label->setText(info);
}

void SavedGamePreview::clearPreview() {
  m_label->clear();
}
