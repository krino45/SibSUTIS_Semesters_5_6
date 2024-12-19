#include "edituserdialog.h"
#include "databasehandler.h"
#include <QMessageBox>

EditUserDialog::EditUserDialog(int userId, QWidget *parent)
    : QDialog(parent), userId(userId) {
    setupUI();
    loadUserData();
}

void EditUserDialog::setupUI() {
    setWindowTitle("Редактирование профиля");

    // Поле для аватара
    profilePictureLabel = new QLabel(this);
    profilePictureLabel->setFixedSize(100, 100);
    profilePictureLabel->setStyleSheet("border: 1px solid #ccc;");

    changeProfilePictureButton = new QPushButton("Изменить фото", this);
    connect(changeProfilePictureButton, &QPushButton::clicked, this, &EditUserDialog::selectProfilePicture);

    QVBoxLayout *profilePictureLayout = new QVBoxLayout();
    profilePictureLayout->addWidget(profilePictureLabel, 0, Qt::AlignCenter);
    profilePictureLayout->addWidget(changeProfilePictureButton, 0, Qt::AlignCenter);

    // Поля для редактирования
    nameEdit = new QLineEdit(this);
    emailEdit = new QLineEdit(this);
    aboutMeEdit = new QTextEdit(this);
    passwordEdit = new QLineEdit(this);
    newPasswordEdit = new QLineEdit(this);
    confirmPasswordEdit = new QLineEdit(this);

    passwordEdit->setEchoMode(QLineEdit::Password);
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);

    // Подписи
    QLabel *nameLabel = new QLabel("Имя:", this);
    QLabel *emailLabel = new QLabel("Email:", this);
    QLabel *aboutMeLabel = new QLabel("Обо мне:", this);
    QLabel *passwordLabel = new QLabel("Старый пароль:", this);
    QLabel *newPasswordLabel = new QLabel("Новый пароль:", this);
    QLabel *confirmPasswordLabel = new QLabel("Повторите пароль:", this);

    // Кнопки
    saveButton = new QPushButton("Сохранить", this);
    cancelButton = new QPushButton("Отмена", this);
    connect(saveButton, &QPushButton::clicked, this, &EditUserDialog::saveChanges);
    connect(cancelButton, &QPushButton::clicked, this, &EditUserDialog::reject);

    // Компоновка
    QGridLayout *formLayout = new QGridLayout();
    formLayout->addWidget(nameLabel, 0, 0);
    formLayout->addWidget(nameEdit, 0, 1);
    formLayout->addWidget(emailLabel, 1, 0);
    formLayout->addWidget(emailEdit, 1, 1);
    formLayout->addWidget(aboutMeLabel, 2, 0);
    formLayout->addWidget(aboutMeEdit, 2, 1);
    formLayout->addWidget(passwordLabel, 3, 0);
    formLayout->addWidget(passwordEdit, 3, 1);
    formLayout->addWidget(newPasswordLabel, 4, 0);
    formLayout->addWidget(newPasswordEdit, 4, 1);
    formLayout->addWidget(confirmPasswordLabel, 5, 0);
    formLayout->addWidget(confirmPasswordEdit, 5, 1);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(profilePictureLayout);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);
}

void EditUserDialog::loadUserData() {
    QVariantMap userData = DatabaseHandler::instance()->getUserInfo(userId);

    if (userData.isEmpty()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось загрузить данные пользователя.");
        reject();
        return;
    }

    nameEdit->setText(userData["name"].toString());
    emailEdit->setText(userData["email"].toString());
    aboutMeEdit->setText(userData["about_me"].toString());

    QByteArray photoData = userData["profile_picture"].toByteArray();
    if (!photoData.isEmpty()) {
        QPixmap pixmap;
        if (pixmap.loadFromData(photoData)) {
            profilePictureLabel->setPixmap(pixmap.scaled(profilePictureLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            profilePictureData = photoData;
        }
    }
}

void EditUserDialog::selectProfilePicture() {
    QString filePath = QFileDialog::getOpenFileName(this, "Выбрать изображение", QString(), "Изображения (*.png)");
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
}

bool EditUserDialog::validateInputs() {
    if (nameEdit->text().trimmed().isEmpty() || emailEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Имя и Email не могут быть пустыми.");
        return false;
    }

    if (!newPasswordEdit->text().isEmpty() || !confirmPasswordEdit->text().isEmpty()) {
        if (newPasswordEdit->text() != confirmPasswordEdit->text()) {
            QMessageBox::warning(this, "Ошибка", "Пароли не совпадают.");
            return false;
        }
    }

    return true;
}

void EditUserDialog::saveChanges() {
    if (!validateInputs()) return;

    QVariantMap updatedData;
    updatedData["name"] = nameEdit->text().trimmed();
    updatedData["email"] = emailEdit->text().trimmed();
    updatedData["about_me"] = aboutMeEdit->toPlainText().trimmed();

    if (!profilePictureData.isEmpty()) {
        updatedData["profile_picture"] = profilePictureData;
    }

    if (!passwordEdit->text().isEmpty()) {
        if (!DatabaseHandler::instance()->verifyUser(DatabaseHandler::instance()->getUserInfo(userId)["email"].toString(), passwordEdit->text())) {
            QMessageBox::warning(this, "Ошибка", "Пароль не подтвержден.");
            return;
        }
        updatedData["password"] = newPasswordEdit->text().isEmpty() ? passwordEdit->text() : newPasswordEdit->text();
    }

    if (DatabaseHandler::instance()->updateUser(userId, updatedData)) {
        QMessageBox::information(this, "Успех", "Данные пользователя успешно обновлены.");
        emit userUpdated();
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось обновить данные пользователя.");
    }
}
