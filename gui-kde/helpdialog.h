#pragma once

#include <QDialog>
#include <QMap>
#include <QPixmap>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QStack>
#include <functional>

#include "fc_types.h"
#include "helpdata.h"

namespace Ui {
class HelpDialog;
}

class QTreeWidgetItem;
class QLabel;
class QTextBrowser;
class QSplitter;

struct terrain;
struct impr_type;
struct advance;
struct unit_type;
struct help_item;
struct extra_type;
struct universal;

namespace KV {

class HelpModel;
class HelpNode;
class HelpFilter;

class HelpDialog : public QDialog
{
  Q_OBJECT

public:
  explicit HelpDialog(QWidget *parent = nullptr);
  ~HelpDialog();

  void checkAndShow();

private  slots:

  void on_helpTree_clicked(QModelIndex);
  void on_chaptersButton_clicked();
  void on_headersButton_clicked();
  void on_searchLine_textEdited(const QString& s);
  void on_backButton_clicked();
  void on_forwardButton_clicked();

  void modelReset();
  void interpretLink(const QString&);
  void showMatchingTopics(const QString& title, help_page_type section);

signals:

  void anchorClicked(const QModelIndex&);

private:

  void changePage(QModelIndex);
  void saveSplitterSizes();

  QWidget* leftPanel(const universal&);
  QWidget* bottomPanel(const universal&);

  QWidget* terrainBottom(terrain*);
  QWidget* terrainLeft(terrain*);
  QWidget* buildingLeft(impr_type*);
  QWidget* techLeft(advance*);
  QWidget* unitLeft(unit_type*);

  QWidget* pixmapLabel(const QPixmap&);
  QWidget* keyValueLabel(const QStringList& data);
  QWidget* makeLink(const universal*, const QString& header, const QString& footer = QString());
  QWidget* property(const QString& key, int value);
  QWidget* property(const QString& key, const QString& value);
  QWidget* terrainExtraBottom(terrain*, const extra_type* resource);

  void readSettings();
  void writeSettings() const;

private:

  using WidgetCache = QMap<int, QWidget*>;
  using IndexStack = QStack<QModelIndex>;

  Ui::HelpDialog *m_ui;
  HelpModel* m_model;

  QTextBrowser* m_browser;
  QWidget* m_panelWidget;
  QList<int> m_hSplitterSizes;
  QList<int> m_vSplitterSizes;
  WidgetCache m_leftCache;
  WidgetCache m_bottomCache;
  HelpFilter* m_filter;
  IndexStack m_history;
  IndexStack m_future;
  QModelIndex m_currentPage;
};

class HelpFilter: public QSortFilterProxyModel {
  Q_OBJECT
public:
  HelpFilter(QObject* parent = nullptr);
  void setChapters(bool on);
  void setHeaders(bool on);

protected:

  bool filterAcceptsRow(int row, const QModelIndex &parent) const override;

private:

  bool m_headers = false;
  bool m_chapters = false;

};


class HelpModel: public QAbstractItemModel {

  Q_OBJECT

public:

  static const int TextRole = Qt::UserRole;
  static const int TypeRole = Qt::UserRole + 1;
  static const int UidRole = Qt::UserRole + 2;

  using IndexStack = QStack<QModelIndex>;
  using MatchFunc = std::function<bool (HelpNode*)>;

  HelpModel(QObject* parent = nullptr);

  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &index) const override;

  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  int rowCount(const QModelIndex &parent) const override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;


  QModelIndex findByUniversal(const universal& u,
                              const QModelIndex& parent = QModelIndex()) const;
  IndexStack findByTopic(const QString& title, help_page_type section,
                         const QModelIndex& parent = QModelIndex()) const;
  void findAnything(IndexStack& results, const QModelIndex& parent, bool stopAtFirst,
                    MatchFunc matchFunc) const;

  bool isValid() const;

public slots:

  void reset();

private:

  universal universal_by_topic(help_page_type, const char*);
  QPixmap pixmap(HelpNode*);
  QString chapter(HelpNode*, const char* source);

};

class HelpNode: public QObject {
  Q_OBJECT
public:
  HelpNode(QObject* parent = nullptr);

  QString title;
  QString text;
  universal u;
  help_page_type type;
  QPixmap pix;
};

}
