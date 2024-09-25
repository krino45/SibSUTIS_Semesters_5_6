#include "authorsdialog.h"
#include "ui_authorsdialog.h"

AuthorsDialog::AuthorsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthorsDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Авторы");
    this->setWindowIcon(QIcon("C:/Users/krina/OneDrive/Desktop/v.jpg"));
}

AuthorsDialog::~AuthorsDialog()
{
    delete ui;
}
