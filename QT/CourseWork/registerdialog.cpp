#include "registerdialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QBuffer>

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent), nameLineEdit(new QLineEdit(this)),
      emailLineEdit(new QLineEdit(this)),
      passwordLineEdit(new QLineEdit(this)),
      registerButton(new QPushButton("Зарегистрироваться", this))
{
    setWindowTitle("Регистрация");
    setFixedSize(400, 550);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("Регистрация", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; margin-bottom: 15px;");
    layout->addWidget(titleLabel);

    nameLineEdit->setPlaceholderText("Имя");
    emailLineEdit->setPlaceholderText("Почта");
    passwordLineEdit->setPlaceholderText("Пароль");
    profilePictureLabel = new QLabel(this);
    profilePictureLabel->setFixedSize(100, 100);
    profilePictureLabel->setAlignment(Qt::AlignCenter); // не работает :(
    profilePictureLabel->setStyleSheet("border: 1px solid #ccc;");

    ProfilePictureButton = new QPushButton("Фото профиля", this);
    connect(ProfilePictureButton, &QPushButton::clicked, this, [this](){
            QString filePath = QFileDialog::getOpenFileName(this, "Выбрать изображение", QString(), "Изображения (*.png *.jpg *.jpeg)");
            if (filePath.isEmpty()) return;

            QPixmap pixmap(filePath);
            if (pixmap.isNull()) {
                QMessageBox::warning(this, "Ошибка", "Не удалось загрузить изображение.");
                return;
            }

            profilePictureLabel->setPixmap(pixmap.scaled(profilePictureLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

            // Преобразование изображения в QByteArray
            QByteArray byteArray;
            QBuffer buffer(&byteArray);
            buffer.open(QIODevice::WriteOnly);
            pixmap.save(&buffer, "PNG");
            profilePictureData = byteArray;
    });

    passwordLineEdit->setEchoMode(QLineEdit::Password);

    registerButton->setStyleSheet("background-color: #DC49B0; font-size: 24px; color: white; padding: 10px; border-radius: 8px;");

    layout->addWidget(nameLineEdit);
    layout->addWidget(emailLineEdit);
    layout->addWidget(passwordLineEdit);
    layout->addWidget(profilePictureLabel);
    layout->addWidget(ProfilePictureButton);
    layout->addWidget(registerButton);

    connect(registerButton, &QPushButton::clicked, this, &RegisterDialog::onRegisterButtonClicked);

    setLayout(layout);
}

void RegisterDialog::onRegisterButtonClicked()
{
    QString name = nameLineEdit->text().trimmed();
    QString email = emailLineEdit->text().trimmed();
    QString password = passwordLineEdit->text().trimmed();

    if (name.isEmpty() || email.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Все поля должны быть заполнены.");
        return;
    }

    if (!DatabaseHandler::instance()->registerUser(email, password, name, profilePictureData, nullptr, "user")) {
        QMessageBox::critical(this, "Ошибка", "Регистрация не удалась. Возможно, почта уже используется.");
        return;
    }

    QMessageBox::information(this, "Успех", "Регистрация выполнена успешно.");
    accept();
}
