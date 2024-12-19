// LoginDialog.cpp
#include "logindialog.h"
#include "registerdialog.h"
#include "appcontext.h"
#include <QMessageBox>
#include <QDebug>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent), emailLineEdit(new QLineEdit(this)),
      passwordLineEdit(new QLineEdit(this)),
      loginButton(new QPushButton("Войти", this)) {

    setWindowTitle("Вход");
    setFixedSize(400, 350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("Вход", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; margin-bottom: 15px;");
    mainLayout->addWidget(titleLabel);

    emailLineEdit->setPlaceholderText("Почта");
    passwordLineEdit->setPlaceholderText("Пароль");
    passwordLineEdit->setEchoMode(QLineEdit::Password);

    loginButton->setStyleSheet("background-color: #DC49B0; font-size: 24px; color: white; padding: 10px; border-radius: 8px;");

    QPushButton *registerLabel = new QPushButton("Нет аккаунта? Зарегистрироваться", this);
    registerLabel->setStyleSheet("QPushButton {font-size: 18px; text-decoration: underline; border: none; color: #DC49B0;}"
                                 "QPushButton:hover {color: #e91e63;}");

    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginButtonClicked);
    connect(registerLabel, &QPushButton::clicked, this, &LoginDialog::onRegisterLinkClicked);

    mainLayout->addWidget(emailLineEdit);
    mainLayout->addWidget(passwordLineEdit);
    mainLayout->addWidget(loginButton);
    mainLayout->addWidget(registerLabel, 0, Qt::AlignCenter);

    setLayout(mainLayout);
}

void LoginDialog::onRegisterLinkClicked() {
    // Здесь вызывается диалог регистрации
    qDebug() << "Открыть окно регистрации";
    // Например:
    RegisterDialog *registrationDialog = new RegisterDialog(this);
    if (registrationDialog->exec() == QDialog::Accepted) {
        QMessageBox::information(this, "Успешно", "Регистрация завершена.");
    }
    delete registrationDialog;
}

QVariantMap LoginDialog::getUserDetails() const {
    return userDetails;
}

void LoginDialog::onLoginButtonClicked()
{
    QString email = emailLineEdit->text().trimmed();
    QString password = passwordLineEdit->text().trimmed();

    if (email.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все поля.");
        return;
    }

    if (!DatabaseHandler::instance()->verifyUser(email, password, userDetails)) {
        QMessageBox::critical(this, "Ошибка", "Неверная почта или пароль.");
        return;
    }


    qDebug() << "LoginDialog userDetails " << userDetails["id"] << " " << userDetails["name"] << " " << userDetails["role"];

    {
        QVariantMap userDetails2 = userDetails;
        AppContext::instance()->setCurrentUser(userDetails2);
    }

    qDebug() << "LoginDialog AppContext " << AppContext::instance()->getCurrentUser()["id"] << " " << AppContext::instance()->getCurrentUser()["name"] << " " << AppContext::instance()->getCurrentUser()["role"];
    if (!userDetails.contains("id") || userDetails["id"].isNull()) {
        qDebug() << "userDetails['id'] is missing or null!";
        return;
    }
    int userId = userDetails["id"].toInt();
    QList<QVariantMap> favorites;
    favorites = DatabaseHandler::instance()->getUserFavourites(userId);

    QList<int> favoriteIds;
    for (const QVariantMap &ad : favorites) {
        favoriteIds.append(ad["id"].toInt());
    }
    AppContext::instance()->setUserFavorites(favoriteIds);
    qDebug() << "favIDs: " << favoriteIds;

    accept();
}
