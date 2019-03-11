#ifndef BUILDABLES_H
#define BUILDABLES_H

#include <QTableView>

struct city;
struct universal;

namespace KV {

class Buildables: public QTableView
{
  Q_OBJECT
public:
  Buildables(city* c, quint64 flags, QWidget* parent = nullptr);

  static const quint64 ShowUnits = 1;
  static const quint64 Future = 2;

signals:

  void selected(universal*);

};

}

#endif // BUILDABLES_H
