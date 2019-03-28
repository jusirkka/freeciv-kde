#include "sciencedialog.h"
#include "ui_sciencedialog.h"
#include "sprite.h"
#include "application.h"
#include <QApplication>


#include"research.h"
#include "client_main.h"
#include "text.h"
#include "tilespec.h"
#include "researchtreewidget.h"

using namespace KV;

ScienceDialog::ScienceDialog(QWidget *parent)
  : QDialog(parent)
  , m_ui(new Ui::ScienceDialog)
  , m_researchTree(new ResearchTreeWidget(this))
{
  m_ui->setupUi(this);

  m_ui->scienceTreeArea->setAutoFillBackground(true);
  m_ui->scienceTreeArea->setWidget(m_researchTree);

  updateReport();

  connect(Application::instance(), &Application::updateScienceReport,
          this, &ScienceDialog::updateReport);

  setWindowTitle(qAppName() + ": Research");
}

ScienceDialog::~ScienceDialog()
{
  delete m_ui;
}

void ScienceDialog::updateReport() {
  static const int techLimit = 10;

  if (!client_has_player()) return;

  m_ui->progressLabel->setText(science_dialog_text());

  auto res = research_get(client_player());

  m_ui->goalCombo->blockSignals(true);
  m_ui->goalCombo->clear();

  QMap<QString, int> items; // for sorting
  advance_index_iterate(A_FIRST, i) {
    if (research_invention_reachable(res, i)
        && res->inventions[i].state != TECH_KNOWN
        && (res->tech_goal == i
            || res->inventions[i].num_required_techs <= techLimit)) {
      items[advance_name_translation(advance_by_number(i))] = i;
    }
  } advance_index_iterate_end;

  QMapIterator<QString, int> it(items);
  while (it.hasNext()) {
    it.next();
    auto pix = get_tech_sprite(get_tileset(), it.value())->pm;
    m_ui->goalCombo->addItem(QIcon(pix), it.key(), it.value());
    if (res->tech_goal == it.value()) {
      m_ui->goalCombo->setCurrentIndex(m_ui->goalCombo->count() - 1);
    }
  }

  if (res->researching == A_UNSET) {
    m_ui->goalCombo->insertItem(0, Q_("?tech:None"), A_UNSET);
    m_ui->goalCombo->setCurrentIndex(0);
  }

  m_ui->goalCombo->blockSignals(false);
  m_ui->goalCombo->setDisabled(client_is_observer());

  m_ui->researchingCombo->blockSignals(true);
  m_ui->researchingCombo->clear();

  items.clear();
  advance_index_iterate(A_FIRST, i) {
    if (res->inventions[i].state == TECH_PREREQS_KNOWN) {
      items[advance_name_translation(advance_by_number(i))] = i;
    }
  } advance_index_iterate_end;

  it = items;
  while (it.hasNext()) {
    it.next();
    auto pix = get_tech_sprite(get_tileset(), it.value())->pm;
    m_ui->researchingCombo->addItem(QIcon(pix), it.key(), it.value());
    if (res->researching == it.value()) {
      m_ui->researchingCombo->setCurrentIndex(m_ui->researchingCombo->count() - 1);
    }
  }

  if (res->researching == A_UNSET || is_future_tech(res->researching)) {
    m_ui->researchingCombo->insertItem(0, Q_("?tech:None"), A_UNSET);
    m_ui->researchingCombo->setCurrentIndex(0);
  }

  m_ui->researchingCombo->blockSignals(false);
  m_ui->researchingCombo->setDisabled(client_is_observer());

  m_ui->progressBar->setFormat(get_science_target_text(nullptr));
  m_ui->progressBar->setMinimum(0);
  m_ui->progressBar->setMaximum(res->researching != A_UNSET ? res->client.researching_cost : -1);
  m_ui->progressBar->setValue(res->bulbs_researched);

  m_ui->goalLabel->setText(get_science_goal_text(res->tech_goal));

  m_researchTree->updateTree();

}

void ScienceDialog::on_goalCombo_currentIndexChanged(int idx) {

  if (!can_client_issue_orders()) return;

  int id = m_ui->goalCombo->itemData(idx).toInt();
  dsend_packet_player_tech_goal(&client.conn, id);
}

void ScienceDialog::on_researchingCombo_currentIndexChanged(int idx) {
  if (!can_client_issue_orders()) return;

  int id = m_ui->researchingCombo->itemData(idx).toInt();
  dsend_packet_player_research(&client.conn, id);
}

