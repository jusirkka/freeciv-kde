#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QGraphicsView>
#include <QWidget>
#include <QCursor>
#include <QMap>
#include <QVector>

#include "tilespec.h"

namespace KV {

class UnitInfo;
class GameInfo;
class MinimapView;
class EndTurnRect;
class MainWindow;
class UnitSelector;

class MapView: public QWidget
{
  Q_OBJECT

public:
  MapView(MainWindow* parent);

  qreal zoomLevel() const {return m_zoom;}
  void zoomIn();
  void zoomOut();

public slots:

  void popupUnitSelector(tile* t);

protected:

  void paintEvent(QPaintEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void focusOutEvent(QFocusEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

private slots:

  void animate();
  void flushDirty();
  void dirtyAll();
  void dirtyRect(const QRect& r);
  void updateCursor(cursor_type ct);
  void createLine();

  void handleSelectPress(const QPoint& p);
  void handleSelectRelease(const QPoint& p);
  void handleQuickBuy(const QPoint& p);
  void handleProductionSelect(const QPoint& p);
  void handleAdjustWorkers(const QPoint& p);
  void handleAppendFocus(const QPoint& p);
  void handleScroll(const QPoint& p);
  void handleCopyProduction(const QPoint& p);
  void handlePasteProduction(const QPoint& p);
  void handleHideWorkers(const QPoint& p);
  void handleShowLink(const QPoint& p);
  void handleTileInfoPopup(const QPoint& p);
  void handleTileInfoPopdown(const QPoint& p);
  void handleWakeupCentries(const QPoint& p);

private:

  static const int m_maxCountDirties = 20;

  int m_cursor;
  int m_cursorFrame;
  QMap<int, QVector<QCursor>> m_cursors;
  QVector<QRect> m_dirties;
  int m_dirtyCount;
  GameInfo* m_gameInfo;
  UnitInfo* m_unitInfo;
  MinimapView* m_minimap;
  EndTurnRect* m_endTurn;
  qreal m_zoom;
  MainWindow* m_mainWindow;
  bool m_storedAutocenter;
  UnitSelector* m_unitSelector;

};


}

#endif // MAPVIEW_H
