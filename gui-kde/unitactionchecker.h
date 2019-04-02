#pragma once

#include <functional>
#include <QAction>


class QAction;
struct unit_list;

namespace KV {

class UnitActionChecker {
public:
  virtual void check(unit_list* us, QAction* a) = 0;
  virtual ~UnitActionChecker() = default;
};

using BoolFunc = std::function<bool (unit_list*)>;

class BoolChecker: public UnitActionChecker {
public:
  BoolChecker(BoolFunc func)
    : m_func(func) {}

  void check(unit_list *us, QAction *a) override {
    a->setEnabled(m_func(us));
  }

private:
  BoolFunc m_func;
};

class BuildCityChecker: public UnitActionChecker {
  void check(unit_list *us, QAction *a) override;
};

class AutoWorkerChecker: public UnitActionChecker {
  void check(unit_list *us, QAction *a) override;
};

class RoadChecker: public UnitActionChecker {
  void check(unit_list *us, QAction *a) override;
};

class IrrigateChecker: public UnitActionChecker {
  void check(unit_list *us, QAction *a) override;
};
class MineChecker: public UnitActionChecker {
  void check(unit_list *us, QAction *a) override;
};
class ConnectRoadChecker: public UnitActionChecker {
  void check(unit_list *us, QAction *a) override;
};
class ConnectRailChecker: public UnitActionChecker {
  void check(unit_list *us, QAction *a) override;
};
class ConnectIrrigationChecker: public UnitActionChecker {
  void check(unit_list *us, QAction *a) override;
};
class TransformChecker: public UnitActionChecker {
  void check(unit_list *us, QAction *a) override;
};

}
