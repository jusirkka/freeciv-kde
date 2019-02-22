#include <QKeyEvent>
#include "messagebox.h"

using namespace KV;

MessageBox::MessageBox(QWidget *parent, const QString& text, const QString& title)
    : QMessageBox(parent)
{
    setWindowTitle(title);
    setText(text);
}

MessageBox::MessageBox(QWidget *parent)
    : QMessageBox(parent)
{}


void MessageBox::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
        destroy();
        event->accept();
    }
    QWidget::keyPressEvent(event);
}

void MessageBox::setTextTitle(const QString &text, const QString &title) {
    setWindowTitle(title);
    setText(text);
}

StandardMessageBox::StandardMessageBox(QWidget *parent,
                                       const QString& text,
                                       const QString& title)
    : MessageBox(parent, text, title)
{
    setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    setDefaultButton(QMessageBox::Ok);
}

