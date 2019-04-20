#include "optionmodel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "application.h"
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QColorDialog>
#include <QPalette>
#include <QScrollArea>
#include "logging.h"
#include <QGroupBox>


#include "options.h"
#include "client_main.h"

using namespace KV;

OptionModel::OptionModel(const option_set *oset, QObject *parent)
  : KPageModel(parent)
  , m_optionSet(oset)
{
  connect(Application::instance(), &Application::updateOption,
          this, &OptionModel::updateOption);

  connect(Application::instance(), &Application::addOption,
          this, &OptionModel::addOption);

  connect(Application::instance(), &Application::delOption,
          this, &OptionModel::delOption);
}


LocalOptionModel::LocalOptionModel(QObject *parent)
  : OptionModel(client_optset, parent) {
  m_icons["Network"] = "preferences-system-network";
  m_icons["Sound"] = "preferences-desktop-sound";
  m_icons["Interface"] = "preferences-desktop";
  m_icons["Graphics"] = "applications-graphics";
  m_icons["Overview"] = "preferences-desktop";
  m_icons["Map Image"] = "image-x-generic";

  reset();
}

ServerOptionModel::ServerOptionModel(QObject *parent)
  : OptionModel(server_optset, parent) {
  m_icons["Geological"] = "games-config-tiles";
  m_icons["Internal"] = "games-config-custom";
  m_icons["Networking"] = "preferences-system-network";
  m_icons["Sociological"] = "games-config-custom";
  m_icons["Economic"] = "games-config-custom";
  m_icons["Scientific"] = "applications-science";
  m_icons["Military"] = "games-config-custom";

  reset();
}



void OptionModel::reset() {
  beginResetModel();

  qDeleteAll(m_categories);
  m_categories.clear();
  m_names.clear();


  for (auto name: m_icons.keys()) {
    addCategory(name);
  }

  options_iterate(m_optionSet, opt) {
    addOption(opt);
  } options_iterate_end;

  endResetModel();
  emit layoutChanged();
}

void OptionModel::addCategory(const QString &name) {
  auto page = new QWidget;
  auto lay = new QVBoxLayout;
  page->setLayout(lay);
  auto area = new QScrollArea;
  area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  area->setWidgetResizable(true);
  area->setWidget(page);
  m_categories[name] = area;
  m_names << name;
  // qCDebug(FC) << "category" << name;

  // keep this the last item
  lay->addItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

void OptionModel::checkOptions(const QModelIndex& idx) {
  auto page = idx.data(KPageModel::WidgetRole).value<QWidget*>();
  auto opts = page->findChildren<OptionWidget*>();
  for (auto opt: opts) {
    opt->setEnabled(opt->enablable());
    opt->reset();
  }
}


void OptionModel::updateOption(const void *d) {
  auto opt = static_cast<const option*>(d);
  if (option_optset(opt) != m_optionSet) return;
  auto w = static_cast<OptionWidget*>(option_get_gui_data(opt));
  if (w == nullptr) {
    qCWarning(FC) << "No handler for option"
                  << option_name(opt) << option_category_name(opt)
                  << "of type" << option_type_name(option_type(opt));
    return;
  }
  qCDebug(FC) << "updateOption" << w->description();
  w->updateIt();
}

void OptionModel::addOption(void *d) {
  delOption(d);
  auto opt = static_cast<option*>(d);
  if (option_optset(opt) != m_optionSet) return;
  QString category = option_category_name(opt);
  if (category == "Font") return; // we don't want no fricking fonts
  if (!m_categories.contains(category)) {
    beginInsertRows(QModelIndex(), m_names.size(), m_names.size());
    addCategory(category);
    endInsertRows();
  }
  auto page = m_categories[category];
  auto lay = qobject_cast<QVBoxLayout*>(page->widget()->layout());
  auto w = createOptionWidget(opt);
  if (w == nullptr) return; // unsupported option
  connect(w, &OptionWidget::edited, this, [this, w] (bool yes) {
    emit edited(w, yes);
  });
  connect(w, &OptionWidget::defaulted, this, [this, w] (bool yes) {
    emit defaulted(w, yes);
  });
  lay->insertWidget(lay->count() - 1, w);
}

void OptionModel::delOption(const void *d) {
  auto opt = static_cast<const option*>(d);
  if (option_optset(opt) != m_optionSet) return;
  auto w = static_cast<OptionWidget*>(option_get_gui_data(opt));
  if (w == nullptr) return;
  QString category = option_category_name(opt);
  if (!m_categories.contains(category)) {
    delete w;
    return;
  }
  auto page = m_categories[category];
  auto lay = qobject_cast<QVBoxLayout*>(page->widget()->layout());
  lay->removeWidget(w);
  // w->deleteLater(); crashes
}


OptionWidget* OptionModel::createOptionWidget(option *opt) {
  switch (option_type(opt)) {
  case OT_BOOLEAN:
    return new BooleanWidget(opt);
  case OT_INTEGER:
    return new IntegerWidget(opt);
  case OT_STRING:
    if (option_str_values(opt) == nullptr) {
      return new StringWidget(opt);
    }
    return new StringComboWidget(opt);
  case OT_ENUM:
    return new EnumWidget(opt);
  case OT_BITWISE:
    return new FlagsWidget(opt);
  case OT_COLOR:
    return new ColorWidget(opt);
  default:
    qCWarning(FC) << "Unsupported option" << option_description(opt)
                  << "of type" << option_type_name(option_type(opt));
    return nullptr;
  }
}

// OptionModel: ItemModel implementation

QModelIndex OptionModel::index(int row, int column, const QModelIndex &parent) const {
  if (parent.isValid()) return QModelIndex();
  return createIndex(row, column);
}

QModelIndex OptionModel::parent(const QModelIndex &/*child*/) const {
  return QModelIndex();
}

int OptionModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) return 0;
  return m_names.size();
}

int OptionModel::columnCount(const QModelIndex &parent) const {
  if (parent.isValid()) return 0;
  return 1;
}

QVariant OptionModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) return QVariant();
  if (role == Qt::DisplayRole) return m_names[index.row()];
  if (role == KPageModel::HeaderRole) return m_names[index.row()];
  if (role == KPageModel::HeaderVisibleRole) return true;

  if (role == Qt::DecorationRole && m_icons.contains(m_names[index.row()])) {
    return QIcon::fromTheme(m_icons[m_names[index.row()]]);
  }

  if (role == KPageModel::WidgetRole) {
    return QVariant::fromValue(m_categories[m_names[index.row()]]);
  }
  return QVariant();
}

// Categories filter implementation

CategoriesFilter::CategoriesFilter(QObject *parent)
  : QSortFilterProxyModel(parent) {}

bool CategoriesFilter::filterAcceptsRow(int row, const QModelIndex &parent) const {
  auto model = qobject_cast<KPageModel*>(sourceModel());
  if (model == nullptr) return false;
  auto page = model->data(model->index(row, 0, parent), KPageModel::WidgetRole).value<QWidget*>();

  auto re = filterRegularExpression();
  auto widgets = page->findChildren<OptionWidget*>();
  bool found = widgets.isEmpty();
  for (auto opt: widgets) {
    found = opt->highlight(re) || found;
  }
  return found;
}


// OptionWidget implementation

OptionWidget::OptionWidget(option *opt, QWidget *parent)
  : QWidget(parent)
  , m_lay(new QHBoxLayout)
  , m_label(new QLabel)
  , m_description(option_description(opt))
  , m_option(opt)
{
  option_set_gui_data(m_option, this);

  setWhatsThis(option_help_text(m_option));

  m_label->setText(m_description);

  m_lay->addWidget(m_label);
  m_lay->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
  setLayout(m_lay);

  connect(this, &OptionWidget::edited, this, [this] (bool edits) {
    auto f = m_label->font();
    f.setItalic(edits);
    f.setBold(edits == false && defaultable());
    m_label->setFont(f);
  });
}

const QString& OptionWidget::description() const {
  return m_description;
}

bool OptionWidget::highlight(const QRegularExpression &re) {
  if (!re.isValid() || re.pattern().isEmpty()) {
    m_label->setText(m_description);
    return true;
  }
  if (m_description.contains(re)) {
    QString res;
    QRegularExpressionMatchIterator it = re.globalMatch(m_description);
    int start = 0;
    while (it.hasNext()) {
      QRegularExpressionMatch match = it.next();
      res += m_description.mid(start, match.capturedStart());
      res += QString("<span style=background-color:#ffff00;>%1</span>").arg(match.captured());
      start = match.capturedEnd();
    }
    res += m_description.mid(start);
    m_label->setText(res);
    return true;
  }
  m_label->setText(m_description);
  return false;
}

void OptionWidget::reset() {
  doReset();
  emit edited(false);
  emit defaulted(!defaultable());
}

void OptionWidget::apply() {
  if (canApply()) {
    doApply();
  }
  emit edited(false);
  emit defaulted(!defaultable());
}

void OptionWidget::defaultIt() {  
  doDefault();
  doReset();
  option_reset(m_option);
  emit edited(false);
  emit defaulted(true);
}

void OptionWidget::updateIt() {
  doUpdate();
  doReset();
  emit edited(false);
  emit defaulted(!defaultable());
}

bool OptionWidget::defaultable() const {
  return enablable() && canDefault();
}

bool OptionWidget::enablable() const {
  return option_is_changeable(m_option) && canEnable();
}


bool OptionWidget::canEnable() const {
  return !inBlacklist(option_name(m_option));
}

bool OptionWidget::inBlacklist(const QString &name) {
  static const int banAllStates = 100;
  static QMap<QString, int> blackstate{
    {"mapimg_layer[MAPIMG_LAYER_AREA]", C_S_PREPARING},
    {"mapimg_layer[MAPIMG_LAYER_CITIES]", C_S_PREPARING},
    {"mapimg_layer[MAPIMG_LAYER_TERRAIN]", C_S_PREPARING},
    {"mapimg_layer[MAPIMG_LAYER_BORDERS]", C_S_PREPARING},
    {"mapimg_layer[MAPIMG_LAYER_UNITS]", C_S_PREPARING},
    {"mapimg_layer[MAPIMG_LAYER_FOGOFWAR]", C_S_PREPARING},
    {"mapimg_zoom", C_S_PREPARING},
    {"ai_manual_turn_done", C_S_PREPARING},
    {"gui_qt_fullscreen", banAllStates},
    {"gui_qt_show_titlebar", banAllStates},
    {"gui_qt_sidebar_left", banAllStates},
  };
  return blackstate.contains(name) && client_state() < blackstate[name];
}

// BooleanWidget implementation

BooleanWidget::BooleanWidget(option *opt, QWidget *parent)
  : OptionWidget(opt, parent)
  , m_box(new QCheckBox)
{
  doUpdate();
  m_lay->addWidget(m_box);
  connect(m_box, &QCheckBox::clicked, this, [this] (bool on) {
    emit edited(on != m_proxy);
  });
}

void BooleanWidget::doReset() {
  m_box->setChecked(m_proxy);
}

void BooleanWidget::doApply() {
  m_proxy = m_box->isChecked();
  option_bool_set(m_option, m_box->isChecked());
}

void BooleanWidget::doUpdate() {
  m_proxy = option_bool_get(m_option);
}

void BooleanWidget::doDefault() {
  m_proxy = option_bool_def(m_option);
}

bool BooleanWidget::canApply() const {
  return m_proxy != m_box->isChecked();
}

bool BooleanWidget::canDefault() const {
  return m_proxy != option_bool_def(m_option);
}

// IntegerWidget implementation

IntegerWidget::IntegerWidget(option *opt, QWidget *parent)
  : OptionWidget(opt, parent)
  , m_box(new QSpinBox)
{
  int min = option_int_min(m_option);
  int max = option_int_max(m_option);
  m_box->setMinimum(min);
  m_box->setMaximum(max);
  m_box->setSingleStep(qMax((max - min) / 50, 1));
  doUpdate();
  m_lay->addWidget(m_box);
  connect(m_box, QOverload<int>::of(&QSpinBox::valueChanged), this, [this] (int v) {
    emit edited(v != m_proxy);
  });
}

void IntegerWidget::doReset() {
  m_box->blockSignals(true);
  m_box->setValue(m_proxy);
  m_box->blockSignals(false);
}

void IntegerWidget::doApply() {
  m_proxy = m_box->value();
  option_int_set(m_option, m_box->value());
}

void IntegerWidget::doUpdate() {
  m_proxy = option_int_get(m_option);
}

void IntegerWidget::doDefault() {
  m_proxy = option_int_def(m_option);
}

bool IntegerWidget::canApply() const {
  return m_proxy != m_box->value();
}

bool IntegerWidget::canDefault() const {
  return m_proxy != option_int_def(m_option);
}


// StringWidget implementation

StringWidget::StringWidget(option *opt, QWidget *parent)
  : OptionWidget(opt, parent)
  , m_line(new QLineEdit)
{
  doUpdate();
  m_lay->addWidget(m_line);
  connect(m_line, &QLineEdit::editingFinished, this, [this] () {
    emit edited(m_line->text() != m_proxy);
  });
}

void StringWidget::doReset() {
  m_line->blockSignals(true);
  m_line->setText(m_proxy);
  m_line->blockSignals(false);
}

void StringWidget::doApply() {
  m_proxy = m_line->text();
  option_str_set(m_option, m_line->text().toUtf8());
}

void StringWidget::doUpdate() {
  m_proxy = option_str_get(m_option);
}

void StringWidget::doDefault() {
  m_proxy = option_str_def(m_option);
}

bool StringWidget::canApply() const {
  return  m_proxy != m_line->text();
}

bool StringWidget::canDefault() const {
  auto def = QString(option_str_def(m_option));
  if (def.isEmpty()) return false;
  return def != m_proxy;
}

// StringComboWidget implementation

StringComboWidget::StringComboWidget(option *opt, QWidget *parent)
  : OptionWidget(opt, parent)
  , m_box(new QComboBox)
{
  auto values = option_str_values(opt);
  strvec_iterate(values, value) {
    m_box->addItem(value);
  } strvec_iterate_end;
  doUpdate();
  m_lay->addWidget(m_box);
  connect(m_box, &QComboBox::currentTextChanged, this, [this] (const QString& text) {
    emit edited(text != m_proxy);
  });
}

void StringComboWidget::doReset() {
  m_box->blockSignals(true);
  m_box->setCurrentText(m_proxy);
  m_box->blockSignals(false);
}

void StringComboWidget::doApply() {
  m_proxy = m_box->currentText();
  option_str_set(m_option, m_box->currentText().toUtf8());
}

void StringComboWidget::doUpdate() {
  m_proxy = option_str_get(m_option);
}

void StringComboWidget::doDefault() {
  m_proxy = option_str_def(m_option);
}

bool StringComboWidget::canApply() const {
  return m_proxy != m_box->currentText();
}

bool StringComboWidget::canDefault() const {
  auto def = QString(option_str_def(m_option));
  if (def.isEmpty()) return false;
  return def != m_proxy;
}


// EnumWidget implementation

EnumWidget::EnumWidget(option *opt, QWidget *parent)
  : OptionWidget(opt, parent)
  , m_box(new QComboBox)
{
  auto values = option_enum_values(m_option);
  strvec_iterate(values, value) {
    m_box->addItem(value);
  } strvec_iterate_end;
  doUpdate();
  m_lay->addWidget(m_box);
  connect(m_box, &QComboBox::currentTextChanged, this, [this] (const QString& text) {
    emit edited(text != m_proxy);
  });
}

void EnumWidget::doReset() {
  m_box->blockSignals(true);
  m_box->setCurrentText(m_proxy);
  m_box->blockSignals(false);
}

void EnumWidget::doApply() {
  m_proxy = m_box->currentText();
  option_enum_set_str(m_option, m_box->currentText().toUtf8());
}

void EnumWidget::doUpdate() {
  m_proxy = option_enum_get_str(m_option);
}

void EnumWidget::doDefault() {
  m_proxy = option_enum_def_str(m_option);
}

bool EnumWidget::canApply() const {
  return m_proxy != m_box->currentText();
}


bool EnumWidget::canDefault() const {
  return m_proxy != option_enum_def_str(m_option);
}


// FlagsWidget implementation

FlagsWidget::FlagsWidget(option *opt, QWidget *parent)
  : OptionWidget(opt, parent)
{
  doUpdate();
  auto group = new QGroupBox;
  auto lay = new QVBoxLayout;
  group->setLayout(lay);
  auto vs = option_bitwise_values(m_option);
  for (int m = 0; m < strvec_size(vs); m++) {
    auto box = new QCheckBox(strvec_get(vs, m));
    m_boxes.append(box);
    lay->addWidget(box);
    connect(box, &QCheckBox::clicked, this, [this] () {
      emit edited(canApply());
    });
  }
  m_lay->addWidget(group);
}

unsigned FlagsWidget::value() const {
  unsigned v = 0;
  for (int m = 0; m < m_boxes.size(); m++) {
    if (m_boxes[m]->isChecked()) {
      v |= 1 << m;
    }
  }
  return v;
}

void FlagsWidget::doReset() {
  for (int m = 0; m < m_boxes.size(); m++) {
    m_boxes[m]->setChecked((m_proxy & (1 << m)) != 0);
  }
}

void FlagsWidget::doApply() {
  m_proxy = value();
  option_bitwise_set(m_option, value());
}

void FlagsWidget::doUpdate() {
  m_proxy = option_bitwise_get(m_option);
}

void FlagsWidget::doDefault() {
  m_proxy = option_bitwise_def(m_option);
}

bool FlagsWidget::canApply() const {
  return m_proxy != value();
}

bool FlagsWidget::canDefault() const {
  return m_proxy != option_bitwise_def(m_option);
}


// ColorWidget implementation

ColorWidget::ColorWidget(option *opt, QWidget *parent)
  : OptionWidget(opt, parent)
  , m_fgButton(new QPushButton)
  , m_bgButton(new QPushButton)
{
  connect(m_fgButton, &QPushButton::clicked, this, &ColorWidget::buttonClicked);
  connect(m_bgButton, &QPushButton::clicked, this, &ColorWidget::buttonClicked);
  m_fgButton->setToolTip("Foreground Color");
  m_bgButton->setToolTip("Background Color");

  doUpdate();
  m_lay->addWidget(m_fgButton);
  m_lay->addWidget(m_bgButton);
}

void ColorWidget::doReset() {
  setColor(m_fgButton, m_fgProxy);
  setColor(m_bgButton, m_bgProxy);
}


void ColorWidget::doApply() {
  m_fgProxy = m_fgButton->palette().color(QPalette::Active, QPalette::Button);
  m_bgProxy = m_bgButton->palette().color(QPalette::Active, QPalette::Button);

  ft_color c;
  c.foreground = m_fgProxy.name().toUtf8();
  c.background = m_bgProxy.name().toUtf8();
  option_color_set(m_option, c);
}

void ColorWidget::doUpdate() {
  auto colors = option_color_get(m_option);
  m_fgProxy.setNamedColor(colors.foreground);
  m_bgProxy.setNamedColor(colors.background);
}

void ColorWidget::doDefault() {
  auto colors = option_color_def(m_option);
  m_fgProxy.setNamedColor(colors.foreground);
  m_bgProxy.setNamedColor(colors.background);
}

bool ColorWidget::canApply() const {
  return m_fgProxy != m_fgButton->palette().color(QPalette::Active, QPalette::Button) ||
      m_bgProxy != m_bgButton->palette().color(QPalette::Active, QPalette::Button);
}



bool ColorWidget::canDefault() const {
  ft_color d = option_color_def(m_option);
  QColor dbg, dfg;
  dbg.setNamedColor(d.background);
  dfg.setNamedColor(d.foreground);
  return dbg != m_bgProxy || dfg != m_fgProxy;
}



void ColorWidget::setColor(QPushButton *but, const QColor &c) {
  but->setStyleSheet(QString("QPushButton:default {background: %1;}").arg(c.name()));
  QPalette pal = but->palette();
  pal.setColor(QPalette::Active, QPalette::Button, c);
  pal.setColor(QPalette::Inactive, QPalette::Button, c);
  but->setPalette(pal);
}

void ColorWidget::buttonClicked() {
  auto but = qobject_cast<QPushButton*>(sender());
  if (but == nullptr) return;
  auto c = QColorDialog::getColor(but->palette().color(QPalette::Active, QPalette::Button), this);
  setColor(but, c);

  if (m_fgProxy != m_fgButton->palette().color(QPalette::Active, QPalette::Button)) {
    emit edited(true);
    emit defaulted(!defaultable());
    return;
  }
  if (m_bgProxy != m_bgButton->palette().color(QPalette::Active, QPalette::Button)) {
    emit edited(true);
    emit defaulted(!defaultable());
    return;
  }

  emit edited(false);
  emit defaulted(!defaultable());
}
