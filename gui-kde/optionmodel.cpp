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


#include "options.h"
#include "client_main.h"

using namespace KV;

OptionModel::OptionModel(const option_set *oset, QObject *parent)
  : KPageModel(parent)
  , m_optionSet(oset)
{
  options_iterate(m_optionSet, opt) {
    QString category = option_category_name(opt);
    if (category == "Font") continue; // we don't want no fricking fonts
    if (!m_categories.contains(category)) {
      auto page = new QWidget;
      page->setLayout(new QVBoxLayout);
      auto area = new QScrollArea;
      area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      area->setWidgetResizable(true);
      area->setWidget(page);
      m_categories[category] = area;
      m_names << category;
    }
    auto page = m_categories[category];
    auto lay = qobject_cast<QVBoxLayout*>(page->widget()->layout());
    auto w = createOptionWidget(opt);
    if (w == nullptr) continue; // unsupported option
    connect(w, &OptionWidget::edited, this, [this, w] (bool yes) {
      emit edited(w, yes);
    });
    connect(w, &OptionWidget::defaulted, this, [this, w] (bool yes) {
      emit defaulted(w, yes);
    });
    lay->addWidget(w);
  } options_iterate_end;

  // add a spacer to the bottom of the page layout
  PageIterator it(m_categories);
  while (it.hasNext()) {
    it.next();
    auto lay = qobject_cast<QVBoxLayout*>(it.value()->widget()->layout());
    lay->addItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));
  }

  m_icons["Network"] = "preferences-system-network";
  m_icons["Sound"] = "preferences-desktop-sound";
  m_icons["Interface"] = "preferences-desktop";
  m_icons["Graphics"] = "applications-graphics";
  m_icons["Overview"] = "preferences-desktop";
  m_icons["Map Image"] = "image-x-generic";

  connect(Application::instance(), &Application::updateOption,
          this, &OptionModel::updateOption);
}

void OptionModel::checkOptions(const QModelIndex& idx) {
  auto page = idx.data(KPageModel::WidgetRole).value<QWidget*>();
  auto opts = page->findChildren<OptionWidget*>();
  for (auto opt: opts) {
    opt->setEnabled(opt->enablable());
  }
}

void OptionModel::updateOption(const void *d) {
  auto opt = static_cast<const option*>(d);
  if (option_optset(opt) != m_optionSet) return;
  auto w = static_cast<OptionWidget*>(option_get_gui_data(opt));
  if (w == nullptr) {
    qCWarning(FC) << "No handler for option" << option_description(opt)
                  << "of type" << option_type_name(option_type(opt));
    return;
  }
  qCDebug(FC) << "updateOption" << option_name(opt);
  w->reset();
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
  bool found = false;
  auto widgets = page->findChildren<OptionWidget*>();
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
    f.setBold(edits == false && canDefault());
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
  if (canChange()) {
    qCDebug(FC) << "reset" << option_name(m_option);
    doReset();
  }
  emit edited(false);
  emit defaulted(!defaultable());
}

void OptionWidget::apply() {
  if (canChange()) {
    qCDebug(FC) << "apply" << option_name(m_option);
    doApply();
  }
  emit edited(false);
  emit defaulted(!defaultable());
}

void OptionWidget::defaultIt() {
  doDefault();
  reset();
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
  reset();
  m_lay->addWidget(m_box);
  connect(m_box, &QCheckBox::clicked, this, [this] (bool on) {
    emit edited(on != option_bool_get(m_option));
  });
}

bool BooleanWidget::canChange() const {
  return option_bool_get(m_option) != m_box->isChecked();
}

void BooleanWidget::doReset() {
  m_box->setChecked(option_bool_get(m_option));
}

void BooleanWidget::doApply() {
  option_bool_set(m_option, m_box->isChecked());
}

bool BooleanWidget::canDefault() const {
  return option_bool_get(m_option) != option_bool_def(m_option);
}

void BooleanWidget::doDefault() {
  option_bool_set(m_option, option_bool_def(m_option));
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
  reset();
  m_lay->addWidget(m_box);
  connect(m_box, QOverload<int>::of(&QSpinBox::valueChanged), this, [this] (int v) {
    emit edited(v != option_int_get(m_option));
  });
}

bool IntegerWidget::canChange() const {
  return option_int_get(m_option) != m_box->value();
}

void IntegerWidget::doReset() {
  m_box->blockSignals(true);
  m_box->setValue(option_int_get(m_option));
  m_box->blockSignals(false);
}

void IntegerWidget::doApply() {
  option_int_set(m_option, m_box->value());
}

bool IntegerWidget::canDefault() const {
  return option_int_get(m_option) != option_int_def(m_option);
}

void IntegerWidget::doDefault() {
  option_int_set(m_option, option_int_def(m_option));
}


// StringWidget implementation

StringWidget::StringWidget(option *opt, QWidget *parent)
  : OptionWidget(opt, parent)
  , m_line(new QLineEdit)
{
  reset();
  m_lay->addWidget(m_line);
  connect(m_line, &QLineEdit::editingFinished, this, [this] () {
    emit edited(m_line->text() != option_str_get(m_option));
  });
}

bool StringWidget::canChange() const {
  return option_str_get(m_option) != m_line->text();
}

void StringWidget::doReset() {
  m_line->blockSignals(true);
  m_line->setText(option_str_get(m_option));
  m_line->blockSignals(false);
}


void StringWidget::doApply() {
  option_str_set(m_option, m_line->text().toUtf8());
}

bool StringWidget::canDefault() const {
  auto def = QString(option_str_def(m_option));
  if (def.isEmpty()) return false;
  return def != option_str_get(m_option);
}

void StringWidget::doDefault() {
  option_str_set(m_option, option_str_def(m_option));
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
  reset();
  m_lay->addWidget(m_box);
  connect(m_box, &QComboBox::currentTextChanged, this, [this] (const QString& text) {
    emit edited(text != option_str_get(m_option));
  });
}

bool StringComboWidget::canChange() const {
  return option_str_get(m_option) != m_box->currentText();
}

void StringComboWidget::doReset() {
  m_box->blockSignals(true);
  m_box->setCurrentText(option_str_get(m_option));
  m_box->blockSignals(false);
}

void StringComboWidget::doApply() {
  option_str_set(m_option, m_box->currentText().toUtf8());
}

bool StringComboWidget::canDefault() const {
  auto def = QString(option_str_def(m_option));
  if (def.isEmpty()) return false;
  return def != option_str_get(m_option);
}

void StringComboWidget::doDefault() {
  option_str_set(m_option, option_str_def(m_option));
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
  reset();
  m_lay->addWidget(m_box);
  connect(m_box, &QComboBox::currentTextChanged, this, [this] (const QString& text) {
    emit edited(text != option_enum_get_str(m_option));
  });
}

bool EnumWidget::canChange() const {
  return option_enum_get_str(m_option) != m_box->currentText();
}


void EnumWidget::doReset() {
  m_box->blockSignals(true);
  m_box->setCurrentText(option_enum_get_str(m_option));
  m_box->blockSignals(false);
}

void EnumWidget::doApply() {
  option_enum_set_str(m_option, m_box->currentText().toUtf8());
}

bool EnumWidget::canDefault() const {
  return QString(option_enum_get_str(m_option)) != option_enum_def_str(m_option);
}

void EnumWidget::doDefault() {
  option_enum_set_str(m_option, option_enum_def_str(m_option));
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

  reset();
  m_lay->addWidget(m_fgButton);
  m_lay->addWidget(m_bgButton);
}


bool ColorWidget::canChange() const {
  ft_color c = option_color_get(m_option);
  QColor cbg, cfg;
  cbg.setNamedColor(c.background);
  cfg.setNamedColor(c.foreground);
  return cfg != m_fgButton->palette().color(QPalette::Active, QPalette::Button) ||
      cbg != m_bgButton->palette().color(QPalette::Active, QPalette::Button);
}


void ColorWidget::doReset() {
  auto colors = option_color_get(m_option);
  QColor c;
  c.setNamedColor(colors.foreground);
  setColor(m_fgButton, c);

  c.setNamedColor(colors.background);
  setColor(m_bgButton, c);
}


void ColorWidget::doApply() {
  ft_color c;
  c.foreground = m_fgButton->palette().color(QPalette::Active, QPalette::Button).name().toUtf8();
  c.background = m_bgButton->palette().color(QPalette::Active, QPalette::Button).name().toUtf8();
  option_color_set(m_option, c);
}

bool ColorWidget::canDefault() const {
  ft_color d = option_color_def(m_option);
  ft_color c = option_color_get(m_option);
  QColor dbg, dfg, cbg, cfg;
  dbg.setNamedColor(d.background);
  dfg.setNamedColor(d.foreground);
  cbg.setNamedColor(c.background);
  cfg.setNamedColor(c.foreground);
  return dbg != cbg && dfg != cfg;
}

void ColorWidget::doDefault() {
  option_color_set(m_option, option_color_def(m_option));
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

  auto colors = option_color_get(m_option);
  c.setNamedColor(colors.foreground);
  if (c != m_fgButton->palette().color(QPalette::Active, QPalette::Button)) {
    emit edited(true);
    emit defaulted(!canDefault());
    return;
  }
  c.setNamedColor(colors.background);
  if (c != m_bgButton->palette().color(QPalette::Active, QPalette::Button)) {
    emit edited(true);
    emit defaulted(!canDefault());
    return;
  }

  emit edited(false);
  emit defaulted(!canDefault());
}
