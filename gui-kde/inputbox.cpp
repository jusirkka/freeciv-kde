#include "inputbox.h"
#include "ui_inputbox.h"

using namespace KV;

InputBox::InputBox(QWidget *parent,
                   const QString &text,
                   const QString &title,
                   const QString &defInput)
    : QDialog(parent)
    , mUI(new Ui::InputBox)
    , mInput(defInput)
{
    mUI->setupUi(this);
    setWindowTitle(title);
    mUI->label->setText(text);
    mUI->lineEdit->setText(mInput);
    mUI->lineEdit->selectAll();
    setResult(QDialog::Rejected);
}

InputBox::~InputBox() {
    delete mUI;
}

void InputBox::on_lineEdit_textEdited(const QString& text) {
    mInput = text;
}

