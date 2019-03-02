#ifndef STARTDIALOG_H
#define STARTDIALOG_H

#include <QDialog>

class QPoint;

namespace Ui {
class StartDialog;
}

namespace KV {
class StartDialog : public QDialog
{
  Q_OBJECT

public:
  explicit StartDialog(QWidget* parent);
  ~StartDialog();

private slots:

  void popupTreeMenu(const QPoint& p);
  void populateTree();

private:

  void updateButtons();

private:
  Ui::StartDialog* m_UI;
};
}

#endif // STARTDIALOG_H
