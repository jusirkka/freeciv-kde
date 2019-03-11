#ifndef GOVMENU_H
#define GOVMENU_H

#include <QMenu>

class government;

namespace KV {

class GovMenu : public QMenu
{
  Q_OBJECT
public:

  explicit GovMenu(QWidget *parent = nullptr);


public slots:

  void updateGov();

private:

  void popupRevolutionDialog(government *g = nullptr) const;
  QAction* findAction(int i) const;

};

}

#endif // GOVMENU_H
