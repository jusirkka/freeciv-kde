#include "helpdialog.h"
#include "ui_helpdialog.h"
#include <KConfigGroup>
#include <KWindowConfig>
#include <KSharedConfig>
#include <QWindow>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QStackedLayout>
#include <QScrollArea>
#include <QProgressBar>
#include <QTextBrowser>
#include "sprite.h"
#include "canvas.h"
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include "logging.h"
#include "application.h"
#include <QTimer>
#include "conf_helpdialog.h"

#include "tilespec.h"
#include "government.h"
#include "specialist.h"
#include "client_main.h"
#include "movement.h"
#include "mapview_common.h"


static universal uid_decode(int v) {
  int kind = v / 10000;
  int value = v % 10000;
  return universal_by_number(static_cast<universals_n>(kind), value);
}

static int uid_encode(const universal& u) {
  int kind;
  int value;
  universal_extraction(&u, &kind, &value);
  return 10000 * kind + value;
}

static canvas *terrain_canvas(terrain *terrain,
                              const extra_type *resource = nullptr,
                              extra_cause cause = EC_COUNT);

using namespace KV;

HelpDialog::HelpDialog(QWidget *parent)
  : QDialog(parent)
  , m_ui(new Ui::HelpDialog)
  , m_model(new HelpModel(this))
  , m_browser(new QTextBrowser(this))
  , m_filter(new HelpFilter(this))
{
  m_ui->setupUi(this);

  m_ui->helpPanel->setLayout(new QVBoxLayout);
  m_panelWidget = m_browser;

  m_filter->setSourceModel(m_model);

  m_ui->forwardButton->setDisabled(true);
  m_ui->backButton->setDisabled(true);

  m_ui->helpTree->setModel(m_filter);
  if (m_model->isValid()) {
    m_ui->helpTree->setCurrentIndex(m_filter->index(0, 0));
    on_helpTree_clicked(m_filter->index(0, 0));
  }
  connect(this, &HelpDialog::anchorClicked, m_ui->helpTree, &QTreeView::clicked);
  connect(m_model, &HelpModel::modelAboutToBeReset, this, &HelpDialog::modelReset);


  connect(Application::instance(), &Application::popupHelpDialog,
          this, &HelpDialog::showMatchingTopics);

  setWindowTitle("Freeciv Manual");
  setWindowFlag(Qt::WindowStaysOnTopHint, false);
  readSettings();
}

HelpDialog::~HelpDialog()
{
  writeSettings();
  delete m_ui;
}

void HelpDialog::readSettings() {

  create(); // ensure there's a window created
  const KConfigGroup cnf(KSharedConfig::openConfig(), "HelpDialog");
  KWindowConfig::restoreWindowSize(windowHandle(), cnf);
  resize(windowHandle()->size());

  m_ui->mainSplitter->setSizes(Conf::HelpDialog::mainSplit());
  m_hSplitterSizes = Conf::HelpDialog::leftSplit();
  m_vSplitterSizes = Conf::HelpDialog::bottomSplit();

  m_ui->chaptersButton->setChecked(Conf::HelpDialog::searchChapters());
  on_chaptersButton_clicked();
  m_ui->headersButton->setChecked(Conf::HelpDialog::searchHeaders());
  on_headersButton_clicked();
}

void HelpDialog::writeSettings() const {
  if (m_ui->mainSplitter->sizes() != QList<int>{71, 22}) {
    Conf::HelpDialog::setMainSplit(m_ui->mainSplitter->sizes());
  } else {
    qCDebug(FC) << "A hack to work around a QSplitter bug: not writing main splitter sizes";
  }
  Conf::HelpDialog::setLeftSplit(m_hSplitterSizes);
  Conf::HelpDialog::setBottomSplit(m_vSplitterSizes);

  Conf::HelpDialog::setSearchChapters(m_ui->chaptersButton->isChecked());
  Conf::HelpDialog::setSearchHeaders(m_ui->headersButton->isChecked());

  Conf::HelpDialog::self()->save();

  KConfigGroup cnf(KSharedConfig::openConfig(), "HelpDialog");
  KWindowConfig::saveWindowSize(windowHandle(), cnf);
}


void HelpDialog::checkAndShow() {
  if (!m_model->isValid()) {
    m_model->reset();
    if (m_model->isValid()) {
      m_ui->helpTree->setCurrentIndex(m_filter->index(0, 0));
      on_helpTree_clicked(m_filter->index(0, 0));
    }
  }
  show();
}

void HelpDialog::modelReset() {
  m_ui->helpPanel->layout()->removeWidget(m_panelWidget);
  m_panelWidget->setParent(nullptr);

  qDeleteAll(m_leftCache);
  qDeleteAll(m_bottomCache);
  m_leftCache.clear();
  m_bottomCache.clear();

  m_history.clear();
  m_future.clear();

  m_ui->forwardButton->setDisabled(true);
  m_ui->backButton->setDisabled(true);
}

void HelpDialog::on_chaptersButton_clicked() {
  m_ui->searchLine->setEnabled(m_ui->headersButton->isChecked() ||
                               m_ui->chaptersButton->isChecked());
  m_filter->setChapters(m_ui->chaptersButton->isChecked());
  on_searchLine_textEdited(m_ui->searchLine->text());
}

void HelpDialog::on_headersButton_clicked() {
  m_ui->searchLine->setEnabled(m_ui->headersButton->isChecked() ||
                               m_ui->chaptersButton->isChecked());
  m_filter->setHeaders(m_ui->headersButton->isChecked());
  on_searchLine_textEdited(m_ui->searchLine->text());
}

void HelpDialog::on_searchLine_textEdited(const QString &s) {
  if (s.length() < 3) {
    m_filter->setFilterRegExp(QRegExp());
  } else {
    m_filter->setFilterRegExp(QRegExp(s, Qt::CaseInsensitive, QRegExp::FixedString));
  }
}

void HelpDialog::on_helpTree_clicked(QModelIndex index) {

  if (m_currentPage.isValid()) {
    m_history.push(m_currentPage);
  }
  m_currentPage = index;
  m_future.clear();
  m_ui->backButton->setDisabled(m_history.isEmpty());
  m_ui->forwardButton->setDisabled(true);

  changePage(index);
}

void HelpDialog::on_forwardButton_clicked() {
  auto idx = m_future.pop();
  changePage(idx);
  m_history.push(m_currentPage);
  m_currentPage = idx;
  m_ui->forwardButton->setDisabled(m_future.isEmpty());
  m_ui->backButton->setEnabled(true);
}

void HelpDialog::on_backButton_clicked() {
  auto idx = m_history.pop();
  changePage(idx);
  m_future.push(m_currentPage);
  m_currentPage = idx;
  m_ui->forwardButton->setEnabled(true);
  m_ui->backButton->setDisabled(m_history.isEmpty());
}


void HelpDialog::changePage(QModelIndex index) {
  m_ui->title->setText(index.data(Qt::DisplayRole).toString());
  m_browser->setText(index.data(HelpModel::TextRole).toString());

  saveSplitterSizes();
  m_ui->helpPanel->layout()->removeWidget(m_panelWidget);
  m_panelWidget->setParent(nullptr);

  auto u = uid_decode(index.data(HelpModel::UidRole).toInt());

  m_panelWidget = m_browser;

  auto w = bottomPanel(u);
  if (w != nullptr) {
    auto splitter = new QSplitter(Qt::Vertical);
    splitter->setObjectName("vertical");
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(m_panelWidget);
    splitter->addWidget(w);
    splitter->setSizes(m_vSplitterSizes);
    m_panelWidget = splitter;
  }

  w = leftPanel(u);
  if (w != nullptr) {
    auto splitter = new QSplitter(Qt::Horizontal);
    splitter->setObjectName("horizontal");
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(w);
    splitter->addWidget(m_panelWidget);
    splitter->setSizes(m_hSplitterSizes);
    m_panelWidget = splitter;
  }

  m_ui->helpPanel->layout()->addWidget(m_panelWidget);
}

void HelpDialog::saveSplitterSizes() {
  auto splitters = findChildren<QSplitter*>();
  for (auto splitter: splitters) {
    if (splitter->objectName() == "horizontal") {
      m_hSplitterSizes = splitter->sizes();
    } else if (splitter->objectName() == "vertical") {
      m_vSplitterSizes = splitter->sizes();
    }
  }
}


QWidget* HelpDialog::leftPanel(const universal& u) {

  int cacheIndex = uid_encode(u);
  if (m_leftCache.contains(cacheIndex)) return m_leftCache[cacheIndex];

  QWidget* left = nullptr;

  if (u.kind == VUT_IMPROVEMENT) {
    left = buildingLeft(u.value.building);
  } else if (u.kind == VUT_ADVANCE) {
    left = techLeft(u.value.advance);
  } else if (u.kind == VUT_TERRAIN) {
    left = terrainLeft(u.value.terrain);
  } else if (u.kind == VUT_UTYPE) {
    left = unitLeft(u.value.utype);
  }

  if (left == nullptr) return left;

  auto area = new QScrollArea;
  area->setWidget(left);
  area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  area->setWidgetResizable(true);

  m_leftCache[cacheIndex] = area;

  return area;
}

QWidget* HelpDialog::bottomPanel(const universal& u) {

  int cacheIndex = uid_encode(u);
  if (m_bottomCache.contains(cacheIndex)) return m_bottomCache[cacheIndex];

  QWidget* bottom = nullptr;

  if (u.kind == VUT_TERRAIN) {
    bottom = terrainBottom(u.value.terrain);
  }

  if (bottom == nullptr) return bottom;

  auto area = new QScrollArea;
  area->setWidget(bottom);
  m_bottomCache[cacheIndex] = area;
  return area;
}

QWidget* HelpDialog::buildingLeft(impr_type* imp) {

  if (imp == nullptr) return nullptr;

  auto left = new QWidget;
  auto lay = new QVBoxLayout;
  auto margins = lay->contentsMargins();
  margins.setBottom(0);
  margins.setTop(0);
  lay->setContentsMargins(margins);
  left->setLayout(lay);

  auto spr = get_building_sprite(get_tileset(), imp);
  lay->addWidget(pixmapLabel(spr->pm));

  QStringList d{_("Cost:"), QString::number(imp->build_cost)};
  if (!is_great_wonder(imp)) {
    d << _("Upkeep:") << QString::number(imp->upkeep);
  }
  lay->addWidget(keyValueLabel(d));

  char buf[1024];
  requirement_vector_iterate(&imp->reqs, preq) {
    if (!preq->present) continue;
    lay->addWidget(makeLink(&preq->source, _("Requirement:")));
    break;
  } requirement_vector_iterate_end;

  requirement_vector_iterate(&imp->obsolete_by, pobs) {
    if (pobs->source.kind != VUT_ADVANCE) continue;
    lay->addWidget(makeLink(&pobs->source, _("Obsolete by:")));
    break;
  } requirement_vector_iterate_end;

  lay->addItem(new QSpacerItem(5, 5, QSizePolicy::Minimum, QSizePolicy::Expanding));
  return left;
}

QWidget* HelpDialog::techLeft(advance * adv) {

  if (adv == nullptr) return nullptr;

  auto left = new QWidget;
  auto lay = new QVBoxLayout;
  auto margins = lay->contentsMargins();
  margins.setBottom(0);
  margins.setTop(0);
  lay->setContentsMargins(margins);
  left->setLayout(lay);

  auto spr = get_tech_sprite(get_tileset(), advance_number(adv));
  lay->addWidget(pixmapLabel(spr->pm));

  governments_iterate(pgov) {
    requirement_vector_iterate(&pgov->reqs, preq) {
      if (preq->source.kind != VUT_ADVANCE || preq->source.value.advance == adv) continue;

      auto u = universal_by_number(VUT_GOVERNMENT, government_number(pgov));
      lay->addWidget(makeLink(&u, _("Allows")));
    } requirement_vector_iterate_end;
  } governments_iterate_end;

  improvement_iterate(pimp) {

    requirement_vector_iterate(&pimp->reqs, preq) {
      if (preq->source.kind != VUT_ADVANCE || preq->source.value.advance == adv) continue;

      auto u = universal_by_number(VUT_IMPROVEMENT, improvement_number(pimp));
      lay->addWidget(makeLink(&u, _("Allows")));
    } requirement_vector_iterate_end;

    requirement_vector_iterate(&pimp->obsolete_by, pobs) {
      if (pobs->source.kind != VUT_ADVANCE || pobs->source.value.advance == adv) continue;
      auto u = universal_by_number(VUT_IMPROVEMENT, improvement_number(pimp));
      lay->addWidget(makeLink(&u, _("Obsoletes")));
    } requirement_vector_iterate_end;

  } improvement_iterate_end;

  unit_type_iterate(put) {
    if (put->require_advance != adv) continue;
    auto u = universal_by_number(VUT_UTYPE, utype_number(put));
    lay->addWidget(makeLink(&u, _("Allows")));
  } unit_type_iterate_end;

  lay->addItem(new QSpacerItem(5, 5, QSizePolicy::Minimum, QSizePolicy::Expanding));


  return left;
}




QWidget* HelpDialog::terrainLeft(terrain *terr) {

  if (terr == nullptr) return nullptr;

  auto left = new QWidget;
  auto lay = new QVBoxLayout;
  auto margins = lay->contentsMargins();
  margins.setBottom(0);
  margins.setTop(0);
  lay->setContentsMargins(margins);
  left->setLayout(lay);

  auto c = terrain_canvas(terr);
  lay->addWidget(pixmapLabel(c->map_pixmap));
  canvas_free(c);

  lay->addWidget(property(_("Food:"), terr->output[O_FOOD]));
  lay->addWidget(property(_("Production:"), terr->output[O_SHIELD]));
  lay->addWidget(property(_("Trade:"), terr->output[O_TRADE]));

  lay->addSpacing(2 * lay->spacing());

  lay->addWidget(property(_("Move cost:"), terr->movement_cost));
  lay->addWidget(property(_("Defense bonus:"), terr->defense_bonus));

  lay->addSpacing(2 * lay->spacing());

  auto src = universal_by_number(VUT_TERRAIN, terrain_number(terr));

  bool irrigate = terr->irrigation_result != terr
      && terr->irrigation_result != T_NONE
      && terr->irrigation_time != 0
      && univs_have_action_enabler(ACTION_IRRIGATE_TF, nullptr, &src);

  bool mine = terr->mining_result != terr
      && terr->mining_result != T_NONE
      && terr->mining_time != 0
      && univs_have_action_enabler(ACTION_MINE_TF, NULL, &src);

  bool transform = terr->transform_result != T_NONE
      && terr->transform_time != 0
      && univs_have_action_enabler(ACTION_TRANSFORM_TERRAIN, NULL, &src);

  if (irrigate) {
    auto tgt = universal_by_number(VUT_TERRAIN, terrain_number(terr->irrigation_result));
    auto footer = QString(PL_("%1 turn", "%1 turns", terr->irrigation_time)).arg(terr->irrigation_time);
    lay->addWidget(makeLink(&tgt, _("Irrigation Result:"), footer));
  }

  if (mine) {
    auto tgt = universal_by_number(VUT_TERRAIN, terrain_number(terr->mining_result));
    auto footer = QString(PL_("%1 turn", "%1 turns", terr->mining_time)).arg(terr->mining_time);
    lay->addWidget(makeLink(&tgt, _("Mining Result:"), footer));
  }

  if (transform) {
    auto tgt = universal_by_number(VUT_TERRAIN, terrain_number(terr->transform_result));
    auto footer = QString(PL_("%1 turn", "%1 turns", terr->transform_time)).arg(terr->transform_time);
    lay->addWidget(makeLink(&tgt, _("Transform Result:"), footer));
  }

  irrigate = terr->irrigation_result == terr
      && terr->irrigation_time != 0
      && univs_have_action_enabler(ACTION_IRRIGATE, NULL, &src);

  mine = terr->mining_result == terr
      && terr->mining_time != 0
      && univs_have_action_enabler(ACTION_MINE, NULL, &src);

  if (irrigate) {
    extra_type_by_cause_iterate(activity_to_extra_cause(ACTIVITY_IRRIGATE), pextra) {
      if (pextra->buildable
          && universal_fulfills_requirements(false, &(pextra->reqs), &src)) {
        auto tgt = universal_by_number(VUT_EXTRA, extra_number(pextra));
        lay->addWidget(makeLink(&tgt,
                                _("Build as irrigation"),
                                helptext_extra_for_terrain_str(pextra,
                                                               terr,
                                                               ACTIVITY_IRRIGATE)));
      }
    } extra_type_by_cause_iterate_end;
  }

  if (mine) {
    extra_type_by_cause_iterate(activity_to_extra_cause(ACTIVITY_MINE), pextra) {
      if (pextra->buildable
          && universal_fulfills_requirements(false, &(pextra->reqs), &src)) {
        auto tgt = universal_by_number(VUT_EXTRA, extra_number(pextra));
        lay->addWidget(makeLink(&tgt,
                                _("Build as mine"),
                                helptext_extra_for_terrain_str(pextra,
                                                               terr,
                                                               ACTIVITY_MINE)));
      }
    } extra_type_by_cause_iterate_end;
  }

  if (terr->road_time != 0) {
    extra_type_by_cause_iterate(activity_to_extra_cause(ACTIVITY_GEN_ROAD), pextra) {
      if (pextra->buildable
          && universal_fulfills_requirements(false, &(pextra->reqs), &src)) {
        auto tgt = universal_by_number(VUT_EXTRA, extra_number(pextra));
        lay->addWidget(makeLink(&tgt,
                                _("Build as road"),
                                helptext_extra_for_terrain_str(pextra,
                                                               terr,
                                                               ACTIVITY_GEN_ROAD)));
      }
    } extra_type_by_cause_iterate_end;
  }

  if (terr->base_time != 0) {
    extra_type_by_cause_iterate(activity_to_extra_cause(ACTIVITY_BASE), pextra) {
      if (pextra->buildable
          && universal_fulfills_requirements(false, &(pextra->reqs), &src)) {
        auto tgt = universal_by_number(VUT_EXTRA, extra_number(pextra));
        lay->addWidget(makeLink(&tgt,
                                _("Build as base"),
                                helptext_extra_for_terrain_str(pextra,
                                                               terr,
                                                               ACTIVITY_BASE)));
      }
    } extra_type_by_cause_iterate_end;
  }

  lay->addItem(new QSpacerItem(5, 5, QSizePolicy::Minimum, QSizePolicy::Expanding));


  return left;
}


QWidget* HelpDialog::unitLeft(unit_type *ut) {

  if (ut == nullptr) return nullptr;

  auto left = new QWidget;
  auto lay = new QVBoxLayout;
  auto margins = lay->contentsMargins();
  margins.setBottom(0);
  margins.setTop(0);
  lay->setContentsMargins(margins);
  left->setLayout(lay);

  auto c = canvas_create(
        tileset_full_tile_width(get_tileset()),
        tileset_full_tile_height(get_tileset())
        );
  c->map_pixmap.fill(Qt::transparent);
  put_unittype(ut, c, 1.0f, 0, 0);
  lay->addWidget(pixmapLabel(c->map_pixmap));
  canvas_free(c);


  lay->addWidget(property(_("Attack:"), ut->attack_strength));
  lay->addWidget(property(_("Defense:"), ut->defense_strength));
  lay->addWidget(property(_("Moves:"), move_points_text(ut->move_rate, true)));

  lay->addSpacing(2 * lay->spacing());

  lay->addWidget(property(_("Hitpoints:"), ut->hp));
  lay->addWidget(property(_("Cost:"), utype_build_shield_cost_base(ut)));
  lay->addWidget(property(_("Firepower:"), ut->firepower));
  lay->addWidget(property(_("Basic upkeep:"), helptext_unit_upkeep_str(ut)));

  lay->addSpacing(2 * lay->spacing());

  // Tech requirement
  auto adv = ut->require_advance;
  if (adv != nullptr && adv != advance_by_number(0)) {
    auto u = universal_by_number(VUT_ADVANCE, advance_number(adv));
    lay->addWidget(makeLink(&u, _("Requires")));
  }

  // Obsolescence
  auto obs = ut->obsoleted_by;
  if (obs != nullptr) {
    adv = obs->require_advance;
    if (adv != nullptr && adv != advance_by_number(0)) {
      auto u = universal_by_number(VUT_ADVANCE, advance_number(adv));
      lay->addWidget(makeLink(&u, _("Obsoleted by")));
    } else {
      auto u = universal_by_number(VUT_UTYPE, utype_number(obs));
      lay->addWidget(makeLink(&u, _("Obsoleted by")));
    }
  }

  lay->addItem(new QSpacerItem(5, 5, QSizePolicy::Minimum, QSizePolicy::Expanding));

  return left;
}


QWidget* HelpDialog::terrainBottom(terrain *terr) {

  if (terr == nullptr) return nullptr;

  auto bottom = new QWidget;
  auto lay = new QHBoxLayout;
  bottom->setLayout(lay);

  extra_type** resource = terr->resources;

  while (*resource != nullptr) {
    auto w = terrainExtraBottom(terr, *resource);
    lay->addWidget(w);
    resource++;
  }

  if (*terr->resources == nullptr) {
    delete bottom;
    return nullptr;
  }

  return bottom;
}

QWidget* HelpDialog::makeLink(const universal *u, const QString &header, const QString& footer) {
  int type;
  int value;
  universal_extraction(u, &type, &value);
  char buf[1024];
  universal_name_translation(u, buf, sizeof(buf));
  auto link = QString("<a href=%1,%2>%3</a>").arg(type).arg(value).arg(buf);;
  QString text;
  if (footer.isEmpty()) {
    text = QString("<b>%1</b> %2").arg(header).arg(link);
  } else {
    text = QString("<b>%1</b> %2 %3").arg(header).arg(link).arg(footer);
  }
  auto label = new QLabel(text);
  label->setWordWrap(true);
  label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
  label->setTextFormat(Qt::RichText);
  connect(label, &QLabel::linkActivated, this, &HelpDialog::interpretLink);
  return label;
}

QWidget* HelpDialog::pixmapLabel(const QPixmap& pm) {

  auto label = new QLabel;
  label->setPixmap(pm);
  label->setAlignment(Qt::AlignHCenter);

  auto effect = new QGraphicsDropShadowEffect(label);
  effect->setBlurRadius(3);
  effect->setOffset(0, 2);
  label->setGraphicsEffect(effect);

  return label;
}

void HelpDialog::interpretLink(const QString &link) {
  auto parts = link.split(",");
  auto kind = static_cast<universals_n>(parts[0].toInt());
  auto value = parts[1].toInt();
  auto u = universal_by_number(kind, value);
  QModelIndex idx = m_model->findByUniversal(u);
  if (idx.isValid()) {
    emit anchorClicked(m_filter->mapFromSource(idx));
  }
}

void HelpDialog::showMatchingTopics(const QString &topic, help_page_type section) {
  checkAndShow();
  IndexStack indices = m_model->findByTopic(topic, section);
  if (indices.isEmpty()) return;
  auto idx = indices.pop();
  emit anchorClicked(m_filter->mapFromSource(idx));
  while (!indices.isEmpty()) {
    idx = indices.pop();
    QTimer::singleShot(1000, this, [=] () {
      emit anchorClicked(m_filter->mapFromSource(idx));
    });
  }
}


QWidget* HelpDialog::keyValueLabel(const QStringList &data) {
  int cnt = data.count() / 2;
  auto templ = QString("<b>%1</b> %2\n");
  QString text = "";
  for (int i = 0; i < cnt; i++) {
    text += templ.arg(data[2 * i]).arg(data[2 * i + 1]);
  }
  auto label = new QLabel(text);
  label->setWordWrap(true);
  return label;
}


QWidget* HelpDialog::property(const QString &key, const QString &value) {
  // qCDebug(FC) << "property" << key << value;
  auto lay = new QHBoxLayout;
  auto margins = lay->contentsMargins();
  margins.setBottom(0);
  margins.setTop(0);
  lay->setContentsMargins(margins);
  lay->addWidget(new QLabel(key));
  lay->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
  auto vLabel = new QLabel(value);
  vLabel->setWordWrap(true);
  lay->addWidget(vLabel);

  auto prop = new QWidget;
  prop->setLayout(lay);
  return prop;
}

QWidget* HelpDialog::property(const QString &key, int value) {
  return property(key, QString::number(value));
}

QWidget* HelpDialog::terrainExtraBottom(terrain *terr, const extra_type *resource) {

  auto bottom = new QWidget;
  auto lay = new QHBoxLayout;
  auto margins = lay->contentsMargins();
  margins.setBottom(0);
  margins.setTop(0);
  lay->setContentsMargins(margins);
  bottom->setLayout(lay);

  auto title = QString("<b>%1</b><br/>F/P/T = %2/%3/%4")
      .arg(extra_name_translation(resource))
      .arg(terr->output[O_FOOD]  + resource->data.resource->output[O_FOOD])
      .arg(terr->output[O_SHIELD]  + resource->data.resource->output[O_SHIELD])
      .arg(terr->output[O_TRADE]  + resource->data.resource->output[O_TRADE]);

  auto label = new QLabel(title);
  label->setWordWrap(true);
  lay->addWidget(label);

  label = new QLabel;
  auto effect = new QGraphicsDropShadowEffect;
  effect->setBlurRadius(3);
  effect->setOffset(0, 2);
  label->setGraphicsEffect(effect);

  auto c = terrain_canvas(terr, resource);
  label->setPixmap(c->map_pixmap);
  canvas_free(c);

  lay->addWidget(label);

  return bottom;
}

HelpFilter::HelpFilter(QObject* parent)
  : QSortFilterProxyModel(parent) {}

void HelpFilter::setChapters(bool on) {
  m_chapters = on;
}

void HelpFilter::setHeaders(bool on) {
  m_headers = on;
}


bool HelpFilter::filterAcceptsRow(int row, const QModelIndex &parent) const {
  if (!m_headers && !m_chapters) return true;

  QObject* p = sourceModel();
  if (parent.isValid()) {
    p = static_cast<QObject*>(parent.internalPointer());
  }

  // accept if node or its children contains the search string

  bool accept = false;
  auto re = filterRegExp();
  auto model = qobject_cast<HelpModel*>(sourceModel());
  auto node = qobject_cast<HelpNode*>(p->children()[row]);
  auto myIndex = model->index(row, 0, parent);
  if (m_headers) {
    accept = node->title.contains(re);
    if (!accept) {
      HelpModel::IndexStack res;
      model->findAnything(res, myIndex, true, [re] (HelpNode* n) {
        return n->title.contains(re);
      });
      accept = !res.isEmpty();
    }
  }
  if (m_chapters && !accept) {
    accept = node->text.contains(re);
    if (!accept) {
      HelpModel::IndexStack res;
      auto model = qobject_cast<HelpModel*>(sourceModel());
      model->findAnything(res, myIndex, true, [re] (HelpNode* n) {
        return n->text.contains(re);
      });
      accept = !res.isEmpty();
    }
  }
  return accept;
}


HelpNode::HelpNode(QObject *parent)
  : QObject(parent) {}


HelpModel::HelpModel(QObject *parent)
  : QAbstractItemModel(parent) {}

void HelpModel::reset() {

  beginResetModel();

  qDeleteAll(children());

  boot_help_texts();

  // Fill in help tree
  QHash<int, HelpNode*> items;

  help_items_iterate(h) {
    const char* s;
    for (s = h->topic; *s == ' '; s++) {} // noop
    int depth = s - h->topic;
    QObject* parent = this;
    if (depth > 0) {
      parent = items.value(depth - 1);
    }
    auto node = new HelpNode(parent);
    node->type = h->type;
    node->title = QString(h->topic).trimmed();
    node->u = universal_by_topic(h->type, s);
    node->text = chapter(node, h->text);
    node->pix = pixmap(node);

    items.insert(depth, node);
  } help_items_iterate_end;

  endResetModel();
}

QModelIndex HelpModel::index(int row, int column, const QModelIndex &parent) const {
  const QObject* p = this;
  if (parent.isValid()) {
    p = static_cast<const QObject*>(parent.internalPointer());
  }
  if (row >= p->children().count()) return QModelIndex();
  return createIndex(row, column, p->children()[row]);
}

QModelIndex HelpModel::parent(const QModelIndex &index) const {
  if (!index.isValid()) return QModelIndex();
  auto kid = static_cast<QObject*>(index.internalPointer());
  auto p = kid->parent();
  if (p == this) return QModelIndex();
  int parentRow = 0;
  for (auto c: p->parent()->children()) {
    if (c == p) {
      return createIndex(parentRow, 0, p);
    }
    parentRow += 1;
  }
  return QModelIndex();
}

int HelpModel::columnCount(const QModelIndex &parent) const {
  if (!parent.isValid()) return 1;
  auto p = static_cast<const QObject*>(parent.internalPointer());
  if (p->children().isEmpty()) return 0;
  return 1;
}

int HelpModel::rowCount(const QModelIndex &parent) const {
  const QObject* p = this;
  if (parent.isValid()) {
    p = static_cast<const QObject*>(parent.internalPointer());
  }
  return p->children().count();
}

QVariant HelpModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) return QVariant();

  auto node = static_cast<HelpNode*>(index.internalPointer());

  if (role == Qt::DisplayRole) return node->title;
  if (role == Qt::DecorationRole) return node->pix;
  if (role == TextRole) return node->text;
  if (role == TypeRole) return node->type;
  if (role == UidRole) return uid_encode(node->u);

  return QVariant();
}

bool HelpModel::isValid() const {
  return children().count() > 0 && client_state() >= C_S_RUNNING;
}

QModelIndex HelpModel::findByUniversal(const universal &u,
                                       const QModelIndex &parent) const {
  IndexStack results;
  findAnything(results, parent, true, [u] (HelpNode* node) {
    return are_universals_equal(&u, &(node->u));
  });
  if (results.isEmpty()) return QModelIndex();
  return results.pop();
}

HelpModel::IndexStack HelpModel::findByTopic(const QString &title,
                                             help_page_type section,
                                             const QModelIndex &parent) const {
  IndexStack results;

  if (section == HELP_ANY) {
    findAnything(results, parent, false, [title] (HelpNode* node) {
      return title == node->title;
    });
  } else {
    findAnything(results, parent, true, [title, section] (HelpNode* node) {
      return (title == node->title) && (section == node->type);
    });
  }
  return results;
}

void HelpModel::findAnything(IndexStack& results,
                             const QModelIndex &parent,
                             bool stopAtFirst,
                             MatchFunc matchFunc) const {
  int rows = rowCount(parent);
  for (int row = 0; row < rows; row++) {
    auto idx = index(row, 0, parent);
    auto node = static_cast<HelpNode*>(idx.internalPointer());
    if (matchFunc(node)) {
      results.push(idx);
      if (stopAtFirst) return;
    }
    findAnything(results, idx, stopAtFirst, matchFunc);
  }
}


universal HelpModel::universal_by_topic(help_page_type type, const char *s) {

  universal u = universal_by_number(VUT_NONE, 0);

#define COND21(hkey, vkey, func) } else if (type == HELP_ ## hkey) {\
  auto rec = func ## _by_translated_name(s); \
  if (rec) {\
    auto value = func ## _number(rec);\
    u = universal_by_number(VUT_ ## vkey, value); \
  }\

#define COND22(hkey, vkey, func1, func2) } else if (type == HELP_ ## hkey) {\
  auto rec = func1(s); \
  if (rec) {\
    auto value = func2(rec);\
    u = universal_by_number(VUT_ ## vkey, value); \
  }\

  if (type == HELP_ANY) {
  COND22(UNIT, UTYPE, unit_type_by_translated_name, utype_number)
  COND21(IMPROVEMENT, IMPROVEMENT, improvement)
  COND21(WONDER, IMPROVEMENT, improvement)
  COND21(TECH, ADVANCE, advance)
  COND21(TERRAIN, TERRAIN, terrain)
  COND22(EXTRA, EXTRA, extra_type_by_translated_name, extra_number)
  COND21(GOODS, GOOD, goods)
  COND21(SPECIALIST, SPECIALIST, specialist)
  COND21(GOVERNMENT, GOVERNMENT, government)
  }

#undef COND11
#undef COND12
#undef COND21
#undef COND22

  return u;
}


QString HelpModel::chapter(HelpNode* node, const char* text) {

  char buffer[8192];
  QString ch = text;

  if (node->u.kind == VUT_EXTRA) {
    helptext_extra(buffer, sizeof(buffer), client_player(), text, node->u.value.extra);
    ch = buffer;
  } else if (node->u.kind == VUT_GOOD) {
    helptext_goods(buffer, sizeof(buffer), client_player(), text, node->u.value.good);
    ch = buffer;
  } else if (node->u.kind == VUT_GOVERNMENT) {
    helptext_government(buffer, sizeof(buffer), client_player(), text, node->u.value.govern);
    ch = buffer;
  } else if (node->u.kind == VUT_IMPROVEMENT) {
    helptext_building(buffer, sizeof(buffer), client_player(), text, node->u.value.building);
    ch = buffer;
  } else if (node->u.kind == VUT_NATION) {
    helptext_nation(buffer, sizeof(buffer), node->u.value.nation, text);
    ch = buffer;
  } else if (node->u.kind == VUT_SPECIALIST) {
    helptext_specialist(buffer, sizeof(buffer), client_player(),
                        text, node->u.value.specialist);
      ch = buffer;
  } else if (node->u.kind == VUT_ADVANCE) {
    helptext_advance(buffer, sizeof(buffer), client_player(),
                     text, advance_number(node->u.value.advance));
    ch = buffer;
  } else if (node->u.kind == VUT_TERRAIN) {
    helptext_terrain(buffer, sizeof(buffer), client_player(),
                     text, node->u.value.terrain);
    ch = buffer;
  } else if (node->u.kind == VUT_UTYPE) {
    helptext_unit(buffer, sizeof(buffer), client_player(),
                  text, node->u.value.utype);
    ch = buffer;
  }

  return ch;
}


QPixmap HelpModel::pixmap(HelpNode* node) {


  QPixmap pix;

  if (node->u.kind == VUT_EXTRA) {
    drawn_sprite sprites[80];
    fill_basic_extra_sprite_array(get_tileset(), sprites, node->u.value.extra);
    pix = sprites[0].sprite->pm;
  } else if (node->u.kind == VUT_GOVERNMENT) {
    pix = get_government_sprite(get_tileset(), node->u.value.govern)->pm;
  } else if (node->u.kind == VUT_IMPROVEMENT) {
    pix = get_building_sprite(get_tileset(), node->u.value.building)->pm;
  } else if (node->u.kind == VUT_NATION) {
    pix = get_nation_flag_sprite(get_tileset(), node->u.value.nation)->pm;
  } else if (node->u.kind == VUT_ADVANCE) {
    pix = get_tech_sprite(get_tileset(), advance_number(node->u.value.advance))->pm;
  } else if (node->u.kind == VUT_TERRAIN) {
    auto c = terrain_canvas(node->u.value.terrain);
    if (c) {
      pix = c->map_pixmap;
      delete c;
    }
  } else if (node->u.kind == VUT_UTYPE) {
    pix = get_unittype_sprite(get_tileset(), node->u.value.utype, direction8_invalid())->pm;
  }

  if (pix.isNull()) return pix;
  return pix.scaledToHeight(QApplication::fontMetrics().height() + 4);
}

static canvas *terrain_canvas(terrain *terrain,
                              const extra_type *resource,
                              extra_cause cause)
{
  struct canvas *canvas;
  struct drawn_sprite sprs[80];
  int canvas_y, count, i, width, height;
  struct extra_type *pextra;

  width = tileset_full_tile_width(get_tileset());
  height = tileset_full_tile_height(get_tileset());
  canvas_y = height - tileset_tile_height(get_tileset());

  canvas = canvas_create(width, height);
  canvas->map_pixmap.fill(Qt::transparent);
  for (i = 0; i < 3; ++i) {
    count = fill_basic_terrain_layer_sprite_array(get_tileset(), sprs,
                                                  i, terrain);
    put_drawn_sprites(canvas, 1.0f, 0, canvas_y, count, sprs, false);
  }

  pextra = NULL;
  if (cause != EC_COUNT) {
    extra_type_by_cause_iterate(cause, e) {
      pextra = e;
      break;
    } extra_type_by_cause_iterate_end;

    count = fill_basic_extra_sprite_array(get_tileset(), sprs, pextra);
    put_drawn_sprites(canvas, 1.0f, 0, canvas_y, count, sprs, false);
  }

  if (resource != NULL) {
    count = fill_basic_extra_sprite_array(get_tileset(), sprs, resource);
    put_drawn_sprites(canvas, 1.0f, 0, canvas_y, count, sprs, false);
  }

  return canvas;
}
