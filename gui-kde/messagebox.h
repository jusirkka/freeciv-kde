#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QMessageBox>

class QKeyEvent;

namespace KV {

class MessageBox: public QMessageBox
{
    Q_OBJECT

public:
    MessageBox(QWidget* parent, const QString& text, const QString& title);

protected:
    void keyPressEvent(QKeyEvent *event);
    MessageBox(QWidget* parent);
    void setTextTitle(const QString& text, const QString& title);

};

class StandardMessageBox: public MessageBox
{
    Q_OBJECT

public:
    StandardMessageBox(QWidget* parent, const QString& text, const QString& title);

};

}

#endif // MESSAGEBOX_H
