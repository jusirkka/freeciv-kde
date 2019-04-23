#pragma once

#include <QObject>
#include <QDialog>
#include <QMap>

struct unit;
struct city;
struct tile;
struct act_prob;
struct extra_type;

class QPushButton;
class QVBoxLayout;
class QLabel;

namespace KV {

class ActionDialog: public QDialog {
  Q_OBJECT
public:

  ActionDialog(const QString& title, const QString& header, QWidget* parent = nullptr);
  void addButton(int id, int actor, int target, int extra, const act_prob& prob);
  void setTitleAndHeader(const QString& title, const QString& header);

signals:

  void inProgress(int actor);
  void stealTech(int id, int actor, int target);

private:

  QLabel* m_header;
  QVBoxLayout* m_buttonLayout;

  QMap<int, int> m_actions;
};

class ActionSelector : public QObject
{
  Q_OBJECT
public:

  static void Popup(unit *actor,
                    city *t_city,
                    unit *t_unit,
                    tile *t_tile,
                    extra_type *t_extra,
                    const act_prob *act_probs);

  static void Refresh(unit *actor,
                      city *t_city,
                      unit *t_unit,
                      tile *t_tile,
                      extra_type *t_extra,
                      const act_prob *act_probs);

  static void Close();

  static int ActorId();
  static int TargetCityId();
  static int TargetUnitId();
  static int TargetTileId();
  static int TargetExtraId();

  static void Finalize(int actor);

private:

  static QMap<int, int> m_targetedActionMap;

  static ActionSelector* instance();
  ActionSelector(QObject *parent = nullptr);
  ActionSelector(const ActionSelector&);
  ActionSelector& operator=(const ActionSelector&);

  void createDialog(unit *actor,
                    city *t_city,
                    unit *t_unit,
                    tile *t_tile,
                    extra_type *t_extra,
                    const act_prob *act_probs);

  void refreshDialog(unit *actor,
                     city *t_city,
                     unit *t_unit,
                     tile *t_tile,
                     extra_type *t_extra,
                     const act_prob *act_probs);

  void finalAct(int actor);

private slots:

  void reset();
  void buildStealTechDialog(int id, int actor, int target);

private:

  using ActorVector = QVector<int>;

  int m_actor;
  int m_targetCity;
  int m_targetUnit;
  int m_targetTile;
  int m_targetExtra;
  ActionDialog* m_dialog = nullptr;
  ActorVector m_inProgressActors;

};

class ActionExec: public QObject {
  Q_OBJECT
public:
  static ActionExec* Create(int id, QObject* parent = nullptr);
  ActionExec(int id, QObject* parent = nullptr);
  virtual void func(int actor, int target, int extra) const = 0;
signals:
  void inProgress(int actor) const;
  void stealTech(int id, int actor, int target) const;
protected:
  int m_actionId;
};

class RequestDoAction: public ActionExec {
  Q_OBJECT
public:
  RequestDoAction(int id, QObject* parent = nullptr);
  void func(int actor, int target, int extra) const override;
};

class RequestDoActionWithExtra: public ActionExec {
  Q_OBJECT
public:
  RequestDoActionWithExtra(int id, QObject* parent = nullptr);
  void func(int actor, int target, int extra) const override;
};

class RequestActionDetails: public ActionExec {
  Q_OBJECT
public:
  RequestActionDetails(int id, QObject* parent = nullptr);
  void func(int actor, int target, int extra) const override;
};

class SuggestCityName: public ActionExec {
  Q_OBJECT
public:
  SuggestCityName(int id, QObject* parent = nullptr);
  void func(int actor, int target, int extra) const override;
};

class BuildStealTechDialog: public ActionExec {
  Q_OBJECT
public:
  BuildStealTechDialog(int id, QObject* parent = nullptr);
  void func(int actor, int target, int extra) const override;
};

} // namespace KV

