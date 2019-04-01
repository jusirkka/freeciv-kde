#include "messageconfigdialog.h"
#include "ui_messageconfigdialog.h"
#include <QApplication>
#include <QSortFilterProxyModel>
#include "logging.h"
#include <QModelIndex>
#include "proxytablemodel.h"
#include <KConfigGroup>
#include <KWindowConfig>
#include <KSharedConfig>
#include <QWindow>

using namespace KV;

MessageConfigDialog::MessageConfigDialog(int flag, const QString& title, QWidget *parent)
  : QDialog(parent)
  , m_ui(new Ui::MessageConfigDialog)
  , m_flag(flag)
{
  m_ui->setupUi(this);

  setWindowTitle(QString("%1: %2").arg(qAppName()).arg(title));
  setAttribute(Qt::WA_DeleteOnClose);


  m_model = new MessageConfigModel(flag, this);
  auto filter = new QSortFilterProxyModel(this);
  filter->setSourceModel(m_model);
  auto table = new ProxyTableModel(3, this);
  table->setSourceModel(filter);
  m_ui->eventsTable->setModel(table);

  filter->setFilterCaseSensitivity(Qt::CaseInsensitive);
  filter->sort(0);
  m_ui->eventsTable->resizeColumnsToContents();

  connect(m_ui->filterEdit, &QLineEdit::textChanged,
          filter, &QSortFilterProxyModel::setFilterFixedString);

  create(); // ensure there's a window created
  const KConfigGroup cnf(KSharedConfig::openConfig(), "MessageConfigDialog");
  KWindowConfig::restoreWindowSize(windowHandle(), cnf);
  resize(windowHandle()->size());
}

MessageConfigDialog::~MessageConfigDialog() {
  // qCDebug(FC) << "MessageConfigDialog::~MessageConfigDialog";
  KConfigGroup cnf(KSharedConfig::openConfig(), "MessageConfigDialog");
  KWindowConfig::saveWindowSize(windowHandle(), cnf);
}


void MessageConfigDialog::saveConfig() {
  for (event_type e = event_type_begin(); e != event_type_end(); e = event_type_next(e)) {
    int idx = static_cast<int>(e);
    auto d = m_model->data(m_model->index(idx, 0), Qt::CheckStateRole);
    if (!d.isValid()) continue;
    Qt::CheckState state = static_cast<Qt::CheckState>(d.toInt());
    if (state == Qt::Checked) {
      messages_where[e] |= m_flag;
    } else {
      messages_where[e] &= ~ m_flag;
    }
  }
  close();
}

// MessageConfigModel implementation

MessageConfigModel::MessageConfigModel(int flag, QObject *parent)
  : QAbstractListModel(parent)
  , m_checked(m_rowCount) {

  for (event_type e = event_type_begin(); e != event_type_end(); e = event_type_next(e)) {
    int idx = static_cast<int>(e);
    m_checked[idx] = messages_where[idx] & flag;
  }
}

int MessageConfigModel::rowCount(const QModelIndex &/*parent*/) const {
  return m_rowCount;
}

QVariant MessageConfigModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) return QVariant();
  int idx = index.row();
  if (idx >= m_checked.size()) return QVariant();
  if (role == Qt::DisplayRole) {
    return QString(get_event_message_text(static_cast<event_type>(idx)));
  }
  if (role == Qt::CheckStateRole) {
    return m_checked[idx] ? Qt::Checked : Qt::Unchecked;
  }
  return QVariant();
}

bool MessageConfigModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (!index.isValid()) return false;
  if (role != Qt::CheckStateRole) return false;
  int idx = index.row();
  if (idx >= m_checked.size()) return false;
  auto state = static_cast<Qt::CheckState>(value.toInt());
  m_checked[idx] = state == Qt::Checked;
  emit dataChanged(index, index);
  return true;
}


Qt::ItemFlags MessageConfigModel::flags(const QModelIndex &index) const {
  Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);
  if (!index.isValid()) return defaultFlags;
  return defaultFlags | Qt::ItemIsUserCheckable;
}
