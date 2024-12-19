#include "userpage.h"
#include "databasehandler.h"
#include "reportdialog.h"
#include "edituserdialog.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QDebug>
#include <QMessageBox>
#include "editlistingdialog.h"
#include "listingcard.h"
#include "addreviewdialog.h"
#include "logindialog.h"


UserPage::UserPage(int requestingUserId, int requestedUserId, const QString &requestingUserRole, QWidget *parent)
    : QWidget(parent), requestingUserId(requestingUserId), requestedUserId(requestedUserId), requestingUserRole(requestingUserRole) {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    qDebug() << "UserPage: requestingUserId:" << requestingUserId << ", requestedUserId: " << requestedUserId;

    bool isOwner = (requestingUserId == requestedUserId);

    // Левое меню
    QVBoxLayout *menuLayout = new QVBoxLayout();
    QPushButton *adsButton;
    if (isOwner)
        adsButton = new QPushButton("Мои объявления", this);
    else {
        adsButton = new QPushButton("Объявления");
    }

    QPushButton *reviewsButton = new QPushButton("Отзывы", this);
    menuLayout->addWidget(adsButton);
    menuLayout->addWidget(reviewsButton);
    if (!isOwner && requestingUserRole != "admin")
    {
        QPushButton *reportButton = new QPushButton("Пожаловаться", this);
        menuLayout->addWidget(reportButton);

        connect(reportButton, &QPushButton::clicked, this, [this, requestedUserId]() {
            ReportDialog *dialog = new ReportDialog(requestedUserId, false, this);
            dialog->exec();
        });
    } else if (requestingUserRole == "admin") {
        QPushButton *deleteUserButton = new QPushButton("Удалить пользователя", this);
        menuLayout->addWidget(deleteUserButton);

        connect(deleteUserButton, &QPushButton::clicked, this, [this, requestedUserId]() {
            if (QMessageBox::question(this, "Удаление пользователя", "Вы действительно хотите удалить пользователя?") == QMessageBox::Yes) {
                if (DatabaseHandler::instance()->deleteUser(requestedUserId)) {
                    QMessageBox::information(this, "Успех", "Пользователь удален.");
                    emit goMain();
                } else {
                    QMessageBox::critical(this, "Ошибка", "Не получилось удалить пользователя.");
                }
            }
        });
    } else {
        QPushButton *logoutButton = new QPushButton(" Выход", this);
        logoutButton->setIcon(QIcon(":/images/logout.png"));
        menuLayout->addWidget(logoutButton);
        connect(logoutButton, &QPushButton::clicked, this, [this, adsButton]() {
            AppContext::instance()->clearCurrentUser();
            adsButton->setText("Объявления");
            this->setupAdsPage();
            this->setupUserInfo();
            this->setupReviewsPage();
        });

    }
    menuLayout->addStretch();
    mainLayout->addLayout(menuLayout, 1);

    // Основное содержимое
    stackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(stackedWidget, 3);

    // Страницы
    adsPage = new QWidget(this);
    reviewsPage = new QWidget(this);

    // Настройка страниц
    setupUserInfo();  // Отображаем информацию о пользователе
    setupAdsPage();
    setupReviewsPage();

    stackedWidget->addWidget(adsPage);     // Добавляем страницы в stackedWidget
    stackedWidget->addWidget(reviewsPage); // Добавляем страницы в stackedWidget

    connect(adsButton, &QPushButton::clicked, this, &UserPage::showAdsPage);
    connect(reviewsButton, &QPushButton::clicked, this, &UserPage::showReviewsPage);

    showAdsPage();
}

void UserPage::setupUserInfo() {
    // Создаем вертикальную компоновку для информации о пользователе
    QVBoxLayout *infoLayout = new QVBoxLayout();
    infoLayout->setAlignment(Qt::AlignTop);

    // Фото профиля
    QLabel *profilePictureLabel = new QLabel(this);
    profilePictureLabel->setFixedSize(100, 100);

    QVariantMap userInfo = DatabaseHandler::instance()->getUserInfo(requestedUserId);
    QByteArray profilePictureData = userInfo["profile_picture"].toByteArray();

    QPixmap pixmap;
    if (!profilePictureData.isEmpty() && pixmap.loadFromData(profilePictureData)) {
        profilePictureLabel->setPixmap(pixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        pixmap.load(":/images/placeholder.png"); // Путь к изображению-заглушке
        profilePictureLabel->setPixmap(pixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    infoLayout->addWidget(profilePictureLabel, 0, Qt::AlignCenter);

    // Имя пользователя
    QLabel *nameLabel = new QLabel(userInfo["name"].toString(), this);
    nameLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin-top: 10px;");
    nameLabel->setAlignment(Qt::AlignCenter);
    infoLayout->addWidget(nameLabel);

    // Рейтинг пользователя
    QLabel *ratingLabel = new QLabel(QString("Оценка: ⭐ %1").arg(userInfo["reviews"].toDouble(), 0, 'f', 1), this);
    ratingLabel->setAlignment(Qt::AlignCenter);
    infoLayout->addWidget(ratingLabel);

    // Поле "О пользователе"
    QLabel *aboutMeLabel = new QLabel(userInfo["about_me"].toString(), this);
    aboutMeLabel->setWordWrap(true);
    aboutMeLabel->setAlignment(Qt::AlignCenter);
    aboutMeLabel->setStyleSheet("font-size: 14px; color: #666; margin-top: 10px;");
    infoLayout->addWidget(aboutMeLabel);

    QVBoxLayout *menuLayout = static_cast<QVBoxLayout *>(this->layout()->itemAt(0)->layout());
    menuLayout->insertLayout(0, infoLayout);


    // Добавляем кнопку "Редактировать данные" для владельца
    if (requestingUserId == requestedUserId) {
        QPushButton *editProfileButton = new QPushButton("Редактировать профиль", this);
        editProfileButton->setStyleSheet("margin-top: 10px;");
        editProfileButton->setMinimumSize(210,40);
        infoLayout->addWidget(editProfileButton, Qt::AlignCenter);

        connect(editProfileButton, &QPushButton::clicked, this, [this]() {
            EditUserDialog *dialog = new EditUserDialog(requestedUserId, this);
            if (dialog->exec() == QDialog::Accepted) {
                QMessageBox::information(this, "Успех", "Ваши данные были обновлены.");
                emit plshelpme();
            }
        });
    }
}



void UserPage::setupAdsPage() {
    QVBoxLayout *adsLayout = new QVBoxLayout(adsPage);

    bool isOwner = requestingUserId == requestedUserId;
    QLabel *titleLabel;
    if (isOwner)
        titleLabel = new QLabel("Мои объявления", adsPage);
    else {
        titleLabel = new QLabel("Объявления", adsPage);
    }
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    adsLayout->addWidget(titleLabel);

    adsListWidget = new QListWidget(adsPage);
    adsLayout->addWidget(adsListWidget);

    adsListWidget->setStyleSheet(R"(QListWidget::item:hover {
                                        background: none;
                                    }
                                    QListWidget::item:selected {
                                        background-color: #f0f0f0;
                                    })");



    updateAds();
}

void UserPage::setupReviewsPage() {
    QVBoxLayout *reviewsLayout = new QVBoxLayout(reviewsPage);

    QLabel *titleLabel = new QLabel("Отзывы", reviewsPage);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    reviewsLayout->addWidget(titleLabel);

    if (requestingUserId != requestedUserId && AppContext::instance()->isLoggedIn()) {
        QPushButton *addReviewButton = new QPushButton("Добавить отзыв", reviewsPage);
        reviewsLayout->addWidget(addReviewButton);

        connect(addReviewButton, &QPushButton::clicked, this, [this]() {
            AddReviewDialog dialog(requestingUserId, requestedUserId, this);
            if (dialog.exec() == QDialog::Accepted) {
                updateReviews();
            }
        });
    }
    else if (!AppContext::instance()->isLoggedIn())
    {
        QLabel *label = new QLabel("Чтобы оставить отзыв, войдите в систему");
        reviewsLayout->addWidget(label);
    }

    reviewsListWidget = new QListWidget(reviewsPage);
    reviewsLayout->addWidget(reviewsListWidget);

    updateReviews();
}

void UserPage::showAdsPage() {
    stackedWidget->setCurrentWidget(adsPage);
}

void UserPage::showReviewsPage() {
    stackedWidget->setCurrentWidget(reviewsPage);
}

void UserPage::updateAds() {
    adsListWidget->clear(); // Очищаем текущие элементы
    QList<QVariantMap> adsList = DatabaseHandler::instance()->getUserAds(requestedUserId);

    bool isOwner = (requestingUserId == requestedUserId);

    for (const QVariantMap &ad : adsList) {
        if (!ad.contains("id") || !ad.contains("name")) continue; // Проверяем, что данные объявления корректны

        int adId = qvariant_cast<int>(ad["id"]);
        ListingCard *listingCard = new ListingCard(adId, this); // Создаем карточку
        listingCard->setFrameStyle(QFrame::StyledPanel | QFrame::Plain); // Устанавливаем стиль рамки для отображения

        // Настраиваем содержимое карточки
        QHBoxLayout *adLayout = new QHBoxLayout(listingCard);

        // Фото
        QLabel *image = new QLabel(listingCard);
        QByteArray photoData = ad["photo"].toByteArray();
        if (!photoData.isEmpty()) {
            QPixmap pixmap;
            pixmap.loadFromData(photoData);
            image->setPixmap(pixmap.scaled(100, 100, Qt::KeepAspectRatio));
        } else {
            image->setText("Нет фото");
            image->setAlignment(Qt::AlignCenter);
        }
        adLayout->addWidget(image);

        // Информация
        QVBoxLayout *infoLayout = new QVBoxLayout();
        QLabel *nameLabel = new QLabel(ad["name"].toString(), listingCard);
        QLabel *detailsLabel = new QLabel(
            QString("%1 | %2 | %3 руб.")
                .arg(ad.value("city").toString())
                .arg(ad.value("date").toDate().toString("dd.MM.yyyy"))
                .arg(ad.value("price").toDouble(), 0, 'f', 2),
            listingCard);
        infoLayout->addWidget(nameLabel);
        infoLayout->addWidget(detailsLabel);
        adLayout->addLayout(infoLayout);

        // Кнопки для владельца
        if (isOwner) {
            QVBoxLayout *buttonLayout = new QVBoxLayout();
            QPushButton *editButton = new QPushButton("Редактировать", listingCard);
            QPushButton *toggleButton = new QPushButton(ad.value("active").toBool() ? "Снять с продажи" : "Выставить на продажу", listingCard);

            connect(editButton, &QPushButton::clicked, this, [this, adId]() {
                EditListingDialog editListingDialog(adId);
                if (editListingDialog.exec() == QDialog::Accepted) {
                    qDebug() << "Объявление изменено!";
                    this->updateAds();
                }});
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
            if (requestingUserRole != "admin") {
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
                connect(favoriteButton, &QPushButton::clicked, this, [](){
                    LoginDialog *login = new LoginDialog();
                    login->exec();
                });
            }

            QPushButton *msgButton = new QPushButton("Написать продавцу", listingCard);
            connect(msgButton, &QPushButton::clicked, this, [this, adId]() {
                emit this->goToDialog(adId); // Сигнал для начала чата с продавцом
            });
            buttonLayout->addWidget(favoriteButton);
            buttonLayout->addWidget(msgButton);
            adLayout->addLayout(buttonLayout);
            } else { // admin
                qDebug() << "Не загружаем кнопки ибо смотрит админ";
            }
        }

        // Привязка карточки к QListWidget через QListWidgetItem
        QListWidgetItem *item = new QListWidgetItem(adsListWidget);
        item->setSizeHint(listingCard->sizeHint());
        adsListWidget->setItemWidget(item, listingCard);

        // Обработчик клика на карточке
        connect(listingCard, &ListingCard::clicked, this, [this, adId]() {
            qDebug() << "Going over to product page id: " << adId;
            emit this->goToProductPage(adId); // Сигнал для перехода на страницу продукта
        });
    }
}


// Изменяем updateReviews для отображения отзывов
void UserPage::updateReviews() {
    reviewsListWidget->clear();

    QList<QVariantMap> reviewsList = DatabaseHandler::instance()->getUserReviews(requestedUserId);

    for (const QVariantMap &review : reviewsList) {
        QWidget *reviewWidget = new QWidget();
        QHBoxLayout *mainLayout = new QHBoxLayout(reviewWidget);

        // PFP
        QLabel *pfp = new QLabel(reviewWidget);
        QByteArray photoData = review["reviewer_pfp"].toByteArray();
        QPixmap pixmap;
        if (!photoData.isEmpty() && pixmap.loadFromData(photoData)) {
            pfp->setPixmap(pixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            pixmap.load(":/images/placeholder.png");
            pfp->setPixmap(pixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        pfp->setFixedSize(100, 100);
        mainLayout->addWidget(pfp);

        // Текстовая часть
        QVBoxLayout *textLayout = new QVBoxLayout();

        // Верхняя строка
        QHBoxLayout *topLayout = new QHBoxLayout();
        QLabel *nameLabel = new QLabel(review["reviewer_name"].toString(), reviewWidget);
        QLabel *ratingLabel = new QLabel(QString("Оценка: ⭐ %1").arg(review["rating"].toInt()), reviewWidget);
        topLayout->addWidget(nameLabel);
        topLayout->addWidget(ratingLabel);

        // Удаление отзыва
        if (review["reviewer"].toInt() == requestingUserId || requestingUserRole == "admin") {
            QPushButton *deleteButton = new QPushButton("Удалить", reviewWidget);
            connect(deleteButton, &QPushButton::clicked, this, [this, review]() {
                if (QMessageBox::question(this, "Удаление", "Вы уверены, что хотите удалить этот отзыв?") == QMessageBox::Yes) {
                    if (DatabaseHandler::instance()->deleteReview(review["id"].toInt())) {
                        updateReviews();
                    } else {
                        QMessageBox::warning(this, "Ошибка", "Не удалось удалить отзыв.");
                    }
                }
            });
            topLayout->addWidget(deleteButton);
        }

        textLayout->addLayout(topLayout);

        // Текст отзыва
        QLabel *reviewText = new QLabel(review["review_text"].toString(), reviewWidget);
        reviewText->setWordWrap(true);
        textLayout->addWidget(reviewText);

        mainLayout->addLayout(textLayout);

        QListWidgetItem *item = new QListWidgetItem(reviewsListWidget);
        item->setSizeHint(reviewWidget->sizeHint());
        reviewsListWidget->setItemWidget(item, reviewWidget);
    }
}
