#ifndef EDITUSERDIALOG_H
#define EDITUSERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QByteArray>
#include <QPixmap>
#include <QFileDialog>
#include <QDebug>
#include <QBuffer>
#include <QTextEdit>


class EditUserDialog : public QDialog {
    Q_OBJECT

public:
    explicit EditUserDialog(int userId, QWidget *parent = nullptr);

signals:
    void userUpdated(); // Сигнал для оповещения об обновлении пользователя

private slots:
    void saveChanges();
    void selectProfilePicture();

private:
    void setupUI();            // Настройка пользовательского интерфейса
    void loadUserData();       // Загрузка данных пользователя из базы
    bool validateInputs();     // Проверка введенных данных перед сохранением

    int userId;

    QLabel *profilePictureLabel;
    QLineEdit *nameEdit;
    QLineEdit *emailEdit;
    QTextEdit *aboutMeEdit;
    QLineEdit *passwordEdit;
    QLineEdit *newPasswordEdit;
    QLineEdit *confirmPasswordEdit;
    QPushButton *changeProfilePictureButton;
    QPushButton *saveButton;
    QPushButton *cancelButton;

    QByteArray profilePictureData; // Хранение изображения профиля в памяти
};

#endif // EDITUSERDIALOG_H
