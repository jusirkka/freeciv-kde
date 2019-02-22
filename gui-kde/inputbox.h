#ifndef INPUTBOX_H
#define INPUTBOX_H

#include <QDialog>

namespace Ui {
class InputBox;
}

namespace KV {

class InputBox : public QDialog
{
    Q_OBJECT

public:
    InputBox(QWidget *parent,
             const QString& text,
             const QString& title,
             const QString& defInput);

    const QString& input() const {return mInput;}
    ~InputBox() override;

private slots:

    void on_lineEdit_textEdited(const QString& text);

private:

    Ui::InputBox* mUI;
    QString mInput;
};

}
#endif // INPUTBOX_H
