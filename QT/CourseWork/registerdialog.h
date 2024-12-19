#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include "databasehandler.h"

class RegisterDialog : public QDialog {
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);

private slots:
    void onRegisterButtonClicked();

private:
    QLineEdit *nameLineEdit;
    QLineEdit *emailLineEdit;
    QLineEdit *passwordLineEdit;
    QLabel *profilePictureLabel;

    QPushButton *ProfilePictureButton;
    QByteArray profilePictureData;

    QPushButton *registerButton;
};

#endif // REGISTERDIALOG_H
