#include "ui_cityview.h"
#include "cityview.h"
#include <QMenu>
#include "application.h"

#include "cityrepdata.h"
#include "city.h"
#include "client_main.h"

using namespace KV;

CityModel::CityModel(QObject *parent)
  : QAbstractTableModel(parent)
{
  reset();
  connect(Application::instance(), &Application::updateCity,
          this, &CityModel::updateCity);
  connect(Application::instance(), &Application::updateCityReport,
          this, &CityModel::reset);
}

int CityModel::rowCount(const QModelIndex &/*index*/) const {
  return m_cities.count();
}

int CityModel::columnCount(const QModelIndex &/*parent*/) const {
  return NUM_CREPORT_COLS;
}

QVariant CityModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) return QVariant();

  int row = index.row();
  int col = index.column();

  if (role == Qt::UserRole) {
    return QVariant::fromValue(reinterpret_cast<void*>(m_cities[row]));
  }

  if (role == Qt::DisplayRole) {
    city_report_spec* spec = &city_report_specs[col];
    return QString(spec->func(m_cities[row], spec->data)).trimmed();
  }

  return QVariant();
}

QVariant CityModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Horizontal && section < NUM_CREPORT_COLS) {
    city_report_spec *spec = &city_report_specs[section];

    if (role == Qt::DisplayRole) {
      return QString("%1\n%2")
          .arg(spec->title1 ? spec->title1 : "")
          .arg(spec->title2 ? spec->title2 : "")
          .trimmed();
    }

    if (role == Qt::ToolTipRole) {
      return QString(spec->explanation);
    }

    if (role == Qt::UserRole) {
      return spec->show;
    }

  }
  return QVariant();

}

void CityModel::reset() {
  beginResetModel();
  m_cities.clear();
  if (client_has_player()) {
    city_list_iterate(client_player()->cities, pcity) {
      m_cities.append(pcity);
    } city_list_iterate_end;
  } else {
    cities_iterate(pcity) {
      m_cities.append(pcity);
    } cities_iterate_end;
  }
  endResetModel();
}

void CityModel::updateCity(city* c) {
  auto idx = toIndex(c);
  if (idx.isValid()) {
    int row = idx.row();
    emit dataChanged(index(row, 0), index(row, columnCount() - 1));
  }
}

QModelIndex CityModel::toIndex(city* c, int col) const {
  int row = m_cities.indexOf(c);
  if (row >= 0) {
    return createIndex(row, col);
  }
  return QModelIndex();
}


CityFilterModel::CityFilterModel(QObject* parent)
  : QSortFilterProxyModel(parent)
{}

bool CityFilterModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
  auto d1 = sourceModel()->data(left).toString();
  auto d2 = sourceModel()->data(right).toString();
  int cmp = cityrepfield_compare(d1.toUtf8(), d2.toUtf8());

  return cmp >= 0;
}

bool CityFilterModel::filterAcceptsRow(int row, const QModelIndex &/*parent*/) const {
  if (m_filters.isEmpty()) return true;
  auto idx = sourceModel()->index(row, 0);
  auto d = sourceModel()->data(idx, Qt::UserRole);
  auto c = reinterpret_cast<city*>(d.value<void*>());
  // intersection of all filters
  for (auto f: m_filters) {
    if (!f->apply(c)) return false;
  }
  return true;
}

CityFilterModel::~CityFilterModel() {
  qDeleteAll(m_filters);
}
void CityFilterModel::addFilter(CityFilter* filter) {
  if (m_filters.contains(filter->name())) {
    delete m_filters[filter->name()];
  }
  m_filters[filter->name()] = filter;
}

void CityFilterModel::removeFilter(const QString& name) {
  if (!m_filters.contains(name)) return;
  delete m_filters[name];
  m_filters.remove(name);
}


CityDelegate::CityDelegate(QObject *parent)
  : QItemDelegate(parent)
{
  QFontMetrics fm(QApplication::font());
  m_height = fm.height() + 8;
}

void CityDelegate::paint(QPainter* painter,
                         const QStyleOptionViewItem &option,
                         const QModelIndex&index) const
{
  QFont font;
  city_report_spec *spec = &city_report_specs[index.column()];
  QStyleOptionViewItem opt = QItemDelegate::setOptions(index, option);

  QString txt = spec->tagname;
  if (txt == "cityname") {
    font.setCapitalization(QFont::SmallCaps);
    font.setBold(true);
    opt.font = font;
  } else if (txt == "hstate_verbose") {
    font.setItalic(true);
    opt.font = font;
  } else if (txt == "prodplus") {
    QPalette palette;
    auto prod = index.data().toString();
    if (prod.toInt() < 0) {
      font.setBold(true);
      palette.setColor(QPalette::Text, QColor(255, 0, 0));
      opt.font = font;
      opt.palette = palette;
    }
  }

  QItemDelegate::paint(painter, opt, index);
}

QSize CityDelegate::sizeHint(const QStyleOptionViewItem& option,
                             const QModelIndex& index) const
{
  QSize s = QItemDelegate::sizeHint(option, index);
  s.setHeight(m_height);
  return s;
}


CityView::CityView(QWidget* parent)
  : QDialog(parent)
  , m_ui(new Ui::CityView)
{
  m_ui->setupUi(this);
  m_filter = new CityFilterModel(this);
  m_cities = new CityModel(this);

  for (int col = 0; col < m_cities->columnCount(); col++) {
    bool show = m_cities->headerData(col, Qt::Horizontal, Qt::UserRole).toBool();
    m_ui->cityView->setColumnHidden(col, !show);
  }

  m_filter->setSourceModel(m_cities);
  m_ui->cityView->setModel(m_filter);
  m_ui->cityView->setItemDelegate(new CityDelegate);
  m_filter->setDynamicSortFilter(true);
  m_ui->cityView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
  m_ui->cityView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_ui->cityView->horizontalHeader(),
          &QTableView::customContextMenuRequested,
          this,
          &CityView::popupHeaderMenu);
  connect(m_ui->cityView,
          &QTableView::doubleClicked,
          this,
          &CityView::gotoCity);
  connect(m_ui->cityView,
          &QTableView::customContextMenuRequested,
          this,
          &CityView::popupListMenu);

  connect(m_filter,
          &CityFilterModel::layoutChanged,
          this,
          &CityView::orderingChanged);
}

bool CityView::hasPrev(city* c) const {
  int numRows = m_filter->rowCount();
  if (numRows < 2) return false;
  auto curr = m_filter->mapFromSource(m_cities->toIndex(c));
  if (!curr.isValid()) return false;
  int row = (curr.row() - 1 + numRows) % numRows;
  return m_filter->index(row, 0).isValid();
}

bool CityView::hasNext(city* c) const {
  int numRows = m_filter->rowCount();
  if (numRows < 2) return false;
  auto curr = m_filter->mapFromSource(m_cities->toIndex(c));
  if (!curr.isValid()) return false;
  int row = (curr.row() + 1 + numRows) % numRows;
  return m_filter->index(row, 0).isValid();
}

city* CityView::next(city* c) const {
  int numRows = m_filter->rowCount();
  auto curr = m_filter->mapFromSource(m_cities->toIndex(c));
  int row = (curr.row() + 1 + numRows) % numRows;
  auto next = m_filter->mapToSource(m_filter->index(row, 0));
  auto data = m_cities->data(next, Qt::UserRole);
  return reinterpret_cast<city*>(data.value<void*>());
}

city* CityView::prev(city* c) const {
  int numRows = m_filter->rowCount();
  auto curr = m_filter->mapFromSource(m_cities->toIndex(c));
  int row = (curr.row() - 1 + numRows) % numRows;
  auto prev = m_filter->mapToSource(m_filter->index(row, 0));
  auto data = m_cities->data(prev, Qt::UserRole);
  return reinterpret_cast<city*>(data.value<void*>());
}

void CityView::on_filterButton_clicked() {
  // TODO
}

void CityView::popupHeaderMenu(const QPoint& /*pos*/) {

  QMenu menu;
  menu.setTitle(_("Column visibility"));

  QVector<QAction*> actions;
  int numCols = m_cities->columnCount();
  for (int i = 0; i < numCols; i++) {
    QString name = m_cities->headerData(i, Qt::Horizontal, Qt::ToolTipRole).toString();
    auto a = menu.addAction(name);
    a->setCheckable(true);
    a->setChecked(!m_ui->cityView->isColumnHidden(i));
    actions.append(a);
  }

  auto selected = menu.exec(QCursor::pos());
  if (selected) {
    int col = actions.indexOf(selected);
    m_ui->cityView->setColumnHidden(col, !selected->isChecked());
    auto spec = &city_report_specs[col];
    spec->show = selected->isChecked();
  }
}

void CityView::popupListMenu(const QPoint& pos) {
  // TODO
}

void CityView::gotoCity(const QModelIndex& cityIndex) {
  auto selected = m_filter->mapToSource(cityIndex);
  auto data = m_cities->data(selected, Qt::UserRole);
  auto c = reinterpret_cast<city*>(data.value<void*>());
  emit manageCity(c);
}

