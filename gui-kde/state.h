#ifndef STATE_H
#define STATE_H

#include <QState>
#include <QAction>
#include <QVector>
#include "pages_g.h"

class QWidget;
class QDialog;

namespace KV {

class MainWindow;
class MapWidget;
class NetworkDialog;
class StartDialog;

namespace State {

class Base: public QState
{
  Q_OBJECT
public:

  Base(MainWindow* parent, enum client_pages page)
    : QState()
    , m_parent(parent)
    , m_page(page) {}

  enum client_pages id() const {return m_page;}

protected:

  MainWindow* m_parent;
  enum client_pages m_page;

};

class Intro: public Base {
  Q_OBJECT
public:
  Intro(MainWindow* parent);
  ~Intro() override;
protected:
  void onEntry(QEvent *event) override;
signals:
  void playing();
private:
  QWidget* createIntroWidget();
};

class Network: public Base {
  Q_OBJECT
public:
  Network(MainWindow* parent);
  ~Network() override;
protected:
  void onEntry(QEvent *event) override;
  void onExit(QEvent *event) override;

signals:
  void accepted();
  void rejected();

private slots:

  void connectToServer();

private:
  KV::NetworkDialog* m_networkDialog;
};

class Game: public Base {
  Q_OBJECT
public:
  Game(MainWindow* parent);
  ~Game() override;
protected:
  void onEntry(QEvent *event) override;



private:
  MapWidget* createMapWidget();
};


class Start: public Base {
  Q_OBJECT
public:
  Start(MainWindow* parent);
  ~Start() override;
protected:
  void onEntry(QEvent *event) override;

private slots:

  void disconnectFromServer();
  void playerReady();

signals:
  void accepted();
  void rejected();

private:
  StartDialog* m_startDialog;
};

}}
#endif // STATE_H
