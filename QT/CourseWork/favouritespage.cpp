#include "favouritespage.h"
#include "databasehandler.h"
#include "listingcard.h"
#include "appcontext.h"
#include "logindialog.h"

#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QMessageBox>
#include <QDebug>
#include <QDate>

FavouritesPage::FavouritesPage(QWidget *parent)
    : QWidget(parent){

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("Избранные объявления", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    favoritesListWidget = new QListWidget(this);
    favoritesListWidget->setStyleSheet(R"(QListWidget::item:hover {
                                          background: none;
                                      }
                                      QListWidget::item:selected {
                                          background-color: #f0f0f0;
                                      })");
    mainLayout->addWidget(favoritesListWidget);

    updateFavorites();
}

void FavouritesPage::updateFavorites() {
    favoritesListWidget->clear();  // Очистить текущие элементы

    // Получаем список избранных объявлений
    QList<int> favoriteIds = AppContext::instance()->getUserFavorites();

    if (favoriteIds.isEmpty()) {
        favoritesListWidget->addItem("Нет избранных объявлений.");
        return;
    }

    for (int adId : favoriteIds) {
        QVariantMap ad = DatabaseHandler::instance()->getAdById(adId);

        ListingCard *listingCard = new ListingCard(adId, this);
        listingCard->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);

        QHBoxLayout *adLayout = new QHBoxLayout(listingCard);

        // Фото объявления
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

        // Информация о товаре
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

        // Кнопка для удаления из избранного
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
                    this->updateFavorites();
                }
            });
        }

        QPushButton *msgButton = new QPushButton("Написать продавцу", listingCard);

        if (AppContext::instance()->isLoggedIn()){
            connect(msgButton, &QPushButton::clicked, this, [this, ad, adId]() {
                emit this->goToDialog(adId, AppContext::instance()->getCurrentUser()["id"].toInt(), ad["user"].toInt()); // Сигнал для начала чата с продавцом
            });
        } else {
            connect(msgButton, &QPushButton::clicked, this, [this](){
                LoginDialog *login = new LoginDialog(this);
                if (login->exec())
                {
                    updateFavorites();
                }
            });
        }

        buttonLayout->addWidget(favoriteButton);
        buttonLayout->addWidget(msgButton);
        adLayout->addLayout(buttonLayout);

        // Добавляем карточку объявления в список
        QListWidgetItem *item = new QListWidgetItem(favoritesListWidget);
        item->setSizeHint(listingCard->sizeHint());
        favoritesListWidget->setItemWidget(item, listingCard);

        // Обработчик клика по карточке объявления
        connect(listingCard, &ListingCard::clicked, this, [this, adId]() {
            qDebug() << "Going over to product page id: " << adId;
            emit this->goToProductPage(adId);
        });
    }
}
