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
  void addButton(int id, int actor, int target, const act_prob& prob);
  void setTitleAndHeader(const QString& title, const QString& header);

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

signals:

public slots:

private:

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

private slots:

  void reset();

private:

  int m_actor;
  int m_targetCity;
  int m_targetUnit;
  int m_targetTile;
  int m_targetExtra;
  ActionDialog* m_dialog = nullptr;

};

} // namespace KV

