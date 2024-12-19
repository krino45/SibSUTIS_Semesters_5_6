#include "productpage.h"
#include "databasehandler.h"
#include "reportdialog.h"
#include "appcontext.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QPushButton>
#include <QPixmap>
#include <QByteArray>
#include <QScrollArea>
#include <QMessageBox>
#include "logindialog.h"
#include "editlistingdialog.h"

ProductPage::ProductPage(int productId, QWidget *parent) : QWidget(parent) {
    setUpProductPage(productId);
}

void ProductPage::setUpProductPage(int productId){

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Загрузка данных товара
    QVariantMap ad = DatabaseHandler::instance()->getAdById(productId);
    QVariantMap user = DatabaseHandler::instance()->getUserInfo(qvariant_cast<int>(ad["user"]));

    // Основной контейнер с разделением 2:1
    QWidget *contentWidget = new QWidget(this);
    QHBoxLayout *contentLayout = new QHBoxLayout(contentWidget);
    contentLayout->setSpacing(30); // Отступ между левым и правым блоками

    // Левая часть (2/3) — информация о товаре
    QWidget *leftWidget = new QWidget(this);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setSpacing(15);

    QLabel *nameLabel = new QLabel(ad["name"].toString(), this);
    nameLabel->setStyleSheet("font-size: 40px; font-weight: bold;");
    QLabel *imageLabel = new QLabel(this);
    QByteArray photoData = ad["photo"].toByteArray();
    if (!photoData.isEmpty()) {
        QPixmap pixmap;
        pixmap.loadFromData(photoData);
        imageLabel->setPixmap(pixmap.scaled(1200, 700, Qt::KeepAspectRatio));
    } else {
        imageLabel->setText("Нет изображения");
        imageLabel->setAlignment(Qt::AlignCenter);
        imageLabel->setStyleSheet("border: 1px solid #A75E91; background-color: #f9f9f9; color: #A75E91;");
        imageLabel->setFixedSize(400, 300);
    }
    QLabel *locationLabel = new QLabel("Местоположение: " + ad["geolocation"].toString(), this);
    locationLabel->setStyleSheet("font-size: 18px; color: #555555;");
    QLabel *categoryLabel = new QLabel("Категория: " + DatabaseHandler::instance()->getCategoryNameFromId(qvariant_cast<int>(ad["category"])), this);
    categoryLabel->setStyleSheet("font-size: 18px; color: #555555;");
    QLabel *descriptionLabel = new QLabel(ad["description"].toString(), this);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setStyleSheet("font-size: 18px; color: #333333;");

    leftLayout->addWidget(nameLabel);
    leftLayout->addWidget(imageLabel);
    leftLayout->addWidget(locationLabel);
    leftLayout->addWidget(categoryLabel);
    leftLayout->addWidget(descriptionLabel);

    // Правая часть (1/3) — цена, кнопки, информация о продавце
    QWidget *rightWidget = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setSpacing(15);

    QLabel *priceLabel = new QLabel(QString::number(ad["price"].toDouble()) + " руб.", this);
    priceLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #DC49B0;");

    rightLayout->addWidget(priceLabel);

    bool isOwner = (qvariant_cast<int>(AppContext::instance()->getCurrentUser()["id"]) == qvariant_cast<int>(ad["user"]));

    /*
        // Кнопки для владельца
        if (isOwner) {
            QVBoxLayout *buttonLayout = new QVBoxLayout();
            QPushButton *editButton = new QPushButton("Редактировать", listingCard);
            QPushButton *toggleButton = new QPushButton(ad.value("active").toBool() ? "Снять с продажи" : "Выставить на продажу", listingCard);

            connect(editButton, &QPushButton::clicked, this, [this, adId]() { editListing(adId); });
            bool active = ad["active"].toBool();
            connect(toggleButton, &QPushButton::clicked, this, [this, adId, active]() {
                if (active) {
                    DatabaseHandler::instance()->deactivateAd(adId);
                    QMessageBox::information(this, "Успех", "Объявление снято с публикации.");
                } else {
                    DatabaseHandler::instance()->activateAd(adId);
                    QMessageBox::information(this, "Успех", "Объявление выставлено на публикацию.");
                }
                this->updateAds();
            });

            buttonLayout->addWidget(editButton);
            buttonLayout->addWidget(toggleButton);
            adLayout->addLayout(buttonLayout);
        } else {
            QVBoxLayout *buttonLayout = new QVBoxLayout();
            QPushButton *favoriteButton = new QPushButton(this);
            bool isFavorite = AppContext::instance()->isFavorite(adId);
            favoriteButton->setText(isFavorite ? "Удалить из избранного" : "Добавить в избранное");
            if (AppContext::instance()->isLoggedIn()){
                connect(favoriteButton, &QPushButton::clicked, this, [adId, favoriteButton]() {
                    if (AppContext::instance()->isFavorite(adId)) {
                        if (DatabaseHandler::instance()->removeFavourite(qvariant_cast<int>(AppContext::instance()->getCurrentUser()["id"]), adId)) {
                            AppContext::instance()->removeFromFavorites(adId);
                            favoriteButton->setText("Добавить в избранное");
                        }
                    } else {
                        if (DatabaseHandler::instance()->addFavourite(qvariant_cast<int>(AppContext::instance()->getCurrentUser()["id"]), adId)) {
                            AppContext::instance()->addToFavorites(adId);
                            favoriteButton->setText("Удалить из избранного");
                        }
                    }
                });
            } else {
                connect(favoriteButton, &QPushButton::clicked, this, [this](){
                    LoginDialog *login = new LoginDialog(this);
                    if (login->exec())
                    {
                        this->setupAdsPage();
                    }
                });
            }

            QPushButton *msgButton = new QPushButton("Написать продавцу", listingCard);
            buttonLayout->addWidget(favoriteButton);
            buttonLayout->addWidget(msgButton);
            adLayout->addLayout(buttonLayout);
        }

    */

    if (isOwner) {
        QPushButton *editButton = new QPushButton("Редактировать", this);
        QPushButton *toggleButton = new QPushButton(ad.value("active").toBool() ? "Снять с продажи" : "Выставить на продажу", this);

        connect(editButton, &QPushButton::clicked, this, [this, productId]() {
            EditListingDialog editListingDialog(productId);
            if (editListingDialog.exec() == QDialog::Accepted) {
                qDebug() << "Объявление изменено!";
                this->setUpProductPage(productId);
            }});
        bool active = ad["active"].toBool();
        connect(toggleButton, &QPushButton::clicked, this, [this, productId, active]() {
            if (active) {
                DatabaseHandler::instance()->deactivateAd(productId);
                QMessageBox::information(this, "Успех", "Объявление снято с публикации.");
            } else {
                DatabaseHandler::instance()->activateAd(productId);
                QMessageBox::information(this, "Успех", "Объявление выставлено на публикацию.");
            }
            this->setUpProductPage(productId);
        });

        rightLayout->addWidget(editButton);
        rightLayout->addWidget(toggleButton);
    } else {
        if (!AppContext::instance()->isAdmin()) {
            QPushButton *favoriteButton = new QPushButton("", this);
            bool isFavorite = AppContext::instance()->isFavorite(productId);
            favoriteButton->setText(isFavorite ? " Удалить из избранного" : " Добавить в избранное");

            favoriteButton->setStyleSheet(isFavorite ? "background-color: #454545; color: white; border-radius: 8px; padding: 12px; text-align: left;" : "background-color: #DC49B0; color: white; border-radius: 8px; padding: 12px; text-align: left;");
            favoriteButton->setIcon(isFavorite ? QIcon(":/images/greyed_heart.png") : QIcon(":/images/heart_2.png"));
            if (AppContext::instance()->isLoggedIn()) {
                    connect(favoriteButton, &QPushButton::clicked, this, [productId, favoriteButton]() {
                    if (AppContext::instance()->isFavorite(productId)) {
                        if (DatabaseHandler::instance()->removeFavourite(qvariant_cast<int>(AppContext::instance()->getCurrentUser()["id"]), productId)) {
                            AppContext::instance()->removeFromFavorites(productId);
                            favoriteButton->setText(" Добавить в избранное");
                            favoriteButton->setStyleSheet("background-color: #DC49B0; color: white; border-radius: 8px; padding: 12px; text-align: left;");
                            favoriteButton->setIcon(QIcon(":/images/heart_2.png"));
                        }
                    } else {
                        if (DatabaseHandler::instance()->addFavourite(qvariant_cast<int>(AppContext::instance()->getCurrentUser()["id"]), productId)) {
                            AppContext::instance()->addToFavorites(productId);
                            favoriteButton->setText(" Удалить из избранного");
                            favoriteButton->setStyleSheet("background-color: #959595; color: white; border-radius: 8px; padding: 12px; text-align: left;");
                            favoriteButton->setIcon(QIcon(":/images/greyed_heart.png"));

                        }
                    }
                });
            } else {
                connect(favoriteButton, &QPushButton::clicked, this, [this, productId]() {
                    LoginDialog *login = new LoginDialog(this);
                    if(login->exec())
                    {
                        this->setUpProductPage(productId);
                    }
                });
            }

            QPushButton *messageButton = new QPushButton("Написать продавцу", this);
            messageButton->setStyleSheet("background-color: #A75E91; color: white; border-radius: 8px; padding: 12px;");

            if (AppContext::instance()->isLoggedIn()){
                connect(messageButton, &QPushButton::clicked, this, [this, ad, productId]() {
                    emit this->goToDialog(productId, AppContext::instance()->getCurrentUser()["id"].toInt(), ad["user"].toInt()); // Сигнал для начала чата с продавцом
                });
            } else {
                connect(messageButton, &QPushButton::clicked, this, [this, productId](){
                    LoginDialog *login = new LoginDialog(this);
                    if (login->exec())
                    {
                        this->setUpProductPage(productId);
                    }
                });
            }

            QPushButton *reportButton = new QPushButton("", this);
            reportButton->setText(" Пожаловаться");
            reportButton->setStyleSheet("background-color: white; color: black; border-radius: 8px; padding: 12px; text-align: left;");
            reportButton->setIcon(QIcon(":/images/warning.png"));
            connect(reportButton, &QPushButton::clicked, this, [this, productId]() {
                ReportDialog *dialog = new ReportDialog(productId, true, this);
                dialog->exec();
            });

            rightLayout->addWidget(favoriteButton);
            rightLayout->addWidget(messageButton);
            rightLayout->addWidget(reportButton);
        } else {
            QPushButton *deleteButton = new QPushButton("Удалить объявление", this);
            deleteButton->setStyleSheet("background-color: #F55; color: white; border-radius: 8px; padding: 12px;");
            connect(deleteButton, &QPushButton::clicked, this, [this, productId]() {
                if(DatabaseHandler::instance()->deleteAd(productId)) {
                    QMessageBox::information(this, "Успех", "Объявление удалено.");
                    emit goMain();
                }
                else {
                    QMessageBox::critical(this, "Ошибка", "Объявление не получилось удалить.");
                }
            });
            rightLayout->addWidget(deleteButton);
        }
    }

    // Информация о продавце
    QHBoxLayout *sellerLayout = new QHBoxLayout();
    QPushButton *profileIcon = new QPushButton(this);
    profileIcon->setMinimumSize(200, 200);

    // Получаем бинарные данные изображения профиля из базы данных
    QByteArray profilePictureData = user["profile_picture"].toByteArray(); // Получаем бинарные данные

    if (!profilePictureData.isEmpty()) {
        QPixmap profilePicture;
        profilePicture.loadFromData(profilePictureData);

        // Устанавливаем QPixmap как иконку для кнопки
        profileIcon->setIcon(QIcon(profilePicture));
        profileIcon->setIconSize(profileIcon->size());

        // Убираем стандартные рамки
        profileIcon->setStyleSheet("border: none; margin-right: 10px;");

    } else {
        // Если данных нет, показываем изображение-заполнитель
        profileIcon->setStyleSheet("border-image: url(:/images/placeholder.png); border: none; margin-right: 10px;");
    }

    //profileIcon->setPixmap(profilePixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    //profileIcon->setFixedSize(200, 200);
    profileIcon->setCursor(Qt::PointingHandCursor); // Указываем, что можно кликать

    QVBoxLayout *sellerDetailsLayout = new QVBoxLayout();
    QLabel *usernameLabel = new QLabel(user["name"].toString(), this);
    usernameLabel->setStyleSheet("font-size: 20px; font-weight: bold;");

    QLabel *ratingLabel = new QLabel(QString::number(user["reviews"].toDouble(), 'f', 1) + " ★", this);
    ratingLabel->setStyleSheet("font-size: 18px; color: #997700;");

    sellerDetailsLayout->addWidget(usernameLabel);
    sellerDetailsLayout->addWidget(ratingLabel);

    sellerLayout->addWidget(profileIcon);
    sellerLayout->addLayout(sellerDetailsLayout);

    rightLayout->addLayout(sellerLayout);
    rightLayout->addStretch(1);

    // Добавляем в основной лэйаут
    contentLayout->addWidget(leftWidget, 2); // Левая часть (2/3)
    contentLayout->addWidget(rightWidget, 1); // Правая часть (1/3)
    contentWidget->setLayout(contentLayout);

    // Добавляем виджет в QScrollArea
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(contentWidget);

    mainLayout->addWidget(scrollArea);

    // Подключаем сигнал к действию
    connect(profileIcon, &QPushButton::clicked, this, [this, user]() {
        int userId = qvariant_cast<int>(user["id"]);
        qDebug() << "link activated, pressed pfp, uid = " << userId;
        emit goToUserPage(userId);  // Переход на страницу пользователя
    });
}

