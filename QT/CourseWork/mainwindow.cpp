#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTimer>
#include <QStackedLayout>
#include <QDateTime>

#include "flowlayout.h"

// Конструктор главного окна
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      mainPage(nullptr),
      searchPage(nullptr),
      userPage(nullptr),
      favouritesPage(nullptr),
      dialogsPage(nullptr),
      dialogPage(nullptr),
      productPage(nullptr),
      adminPage(nullptr),
      ui(new Ui::MainWindow) {

    ui->setupUi(this);
    setWindowTitle("Давай");
    resize(1600, 1000);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    HeaderBar *topbar = new HeaderBar(this);
    connect(topbar, &HeaderBar::goToMainPage, this, &MainWindow::showMainPage);
    connect(topbar, &HeaderBar::goToUserPage, this, &MainWindow::showUserPage);
    connect(topbar, &HeaderBar::goToFavouritesPage, this, &MainWindow::showFavouritesPage);
    connect(topbar, &HeaderBar::goToMessagesPage, this, &MainWindow::showMessagesPage);
    connect(topbar, &HeaderBar::listingsUpdated, this, &MainWindow::refreshListings);
    connect(topbar, &HeaderBar::goBack, this, &MainWindow::goBack);
    connect(topbar, &HeaderBar::searchInit, this, &MainWindow::showSearchPage_text);
    connect(topbar, &HeaderBar::goToAdminPanel, this, &MainWindow::showAdminPanel);

    topbar->setStyleSheet("margin: 0px;");

    mainLayout->addWidget(topbar);

    stackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(stackedWidget);
    setCentralWidget(centralWidget);

    loadingOverlay = new QWidget(this);
    loadingOverlay->setStyleSheet("background-color: rgba(0, 0, 0, 0.33);");
    loadingOverlay->setGeometry(this->rect());
    loadingOverlay->setVisible(false);


    opacityEffect = new QGraphicsOpacityEffect(loadingOverlay);
    loadingOverlay->setGraphicsEffect(opacityEffect);

    fadeAnimation = new QPropertyAnimation(opacityEffect, "opacity");
    fadeAnimation->setDuration(300);

    connect(AppContext::instance(), &AppContext::userLoggedOut, this, &MainWindow::showMainPage);

    showMainPage();
}

void MainWindow::resizeEvent(QResizeEvent*)
{
    loadingOverlay->setGeometry(this->rect());
}

void MainWindow::goBack() {
    if (!AppContext::instance()->hasPreviousPage()) return;

    qDebug() << "popped " << AppContext::instance()->popPreviousPage(); // Удаляем текущую страницу

    if (!AppContext::instance()->hasPreviousPage()) {
        showMainPage();
    } else {
        QString previousPage = AppContext::instance()->popPreviousPage();
        if (previousPage == "main") {
            showMainPage();
        } else if (previousPage == "user") {
            showUserPage();
        } else if (previousPage == "favourites") {
            showFavouritesPage();
        } else if (previousPage == "messages") {
            showMessagesPage();
        } else if (previousPage == "admin") {
            showAdminPanel();
        } else if (previousPage.startsWith("product:")) {
            int productId = previousPage.mid(QString("product:").length()).toInt();
            showProductPage(productId);
        } else if (previousPage.startsWith("other_user:")) {
            int uid = previousPage.mid(QString("other_user:").length()).toInt();
            showOtherUserPage(uid);
        } else if (previousPage.startsWith("search_text:")) {
            QString param = previousPage.mid(QString("search_text:").length());
            showSearchPage_text(param);
        } else if (previousPage.startsWith("search_category:")) {
            QString param = previousPage.mid(QString("search_category:").length());
            showSearchPage_category(param);
        } else if (previousPage.startsWith("dialog:")) {
            QStringList params = previousPage.mid(QString("dialog:").length()).split(":");
            if (params.size() == 3) {
                int productId = params[0].toInt();
                int user1 = params[1].toInt();
                int user2 = params[2].toInt();
                showDialogPage(productId, user1, user2);
            }
        }
    }
}

void MainWindow::showLoadingOverlay() {
    loadingOverlay->setVisible(true);
    fadeAnimation->stop(); // Останавливаем текущую анимацию, если идет
    fadeAnimation->setStartValue(0.0);
    fadeAnimation->setEndValue(1.0);
    fadeAnimation->start();
    QCoreApplication::processEvents(); // Обновляем GUI немедленно
}

void MainWindow::hideLoadingOverlay() {
    fadeAnimation->stop();
    fadeAnimation->setStartValue(1.0);
    fadeAnimation->setEndValue(0.0);
    connect(fadeAnimation, &QPropertyAnimation::finished, [this]() {
        loadingOverlay->setVisible(false);
        fadeAnimation->disconnect(); // Отключаем, чтобы избежать множественных вызовов
    });
    fadeAnimation->start();
}


// создание главной страницы
void MainWindow::showMainPage() {
    AppContext::instance()->pushPreviousPage("main");
    showLoadingOverlay();
    QTimer::singleShot(150, this, [this]() { // Задержка перед созданием страницы
        if (mainPage) {
            stackedWidget->removeWidget(mainPage);
            mainPage->setParent(nullptr);
            delete mainPage;
        }
        createMainPage();
        stackedWidget->addWidget(mainPage);
        stackedWidget->setCurrentWidget(mainPage);
        hideLoadingOverlay();
    });
}

void MainWindow::showSearchPage_text(QString search_param){
    AppContext::instance()->pushPreviousPage(QString("search_text:%1").arg(search_param));
    showLoadingOverlay();
    if (searchPage) {
        stackedWidget->removeWidget(searchPage);
        searchPage->setParent(nullptr);
        delete searchPage;
    }
    createSearchPage(search_param, "");
    stackedWidget->addWidget(searchPage);
    stackedWidget->setCurrentWidget(searchPage);
    hideLoadingOverlay();
}

void MainWindow::showSearchPage_category(QString search_param){
    AppContext::instance()->pushPreviousPage(QString("search_category:%1").arg(search_param));
    showLoadingOverlay();
    if (searchPage) {
        stackedWidget->removeWidget(searchPage);
        searchPage->setParent(nullptr);
        delete searchPage;
    }
    createSearchPage("", search_param);
    stackedWidget->addWidget(searchPage);
    stackedWidget->setCurrentWidget(searchPage);
    hideLoadingOverlay();
}

void MainWindow::createSearchPage(QString text_param, QString category_param)
{
    qDebug() << "Создаем страницу поиска по параметрам текста и категории соответственно: " << text_param << " и " <<category_param;

    if (text_param.isEmpty() && category_param.isEmpty())
    {
        searchPage = new SearchPage(this);
    }
    else {
        searchPage = new SearchPage(text_param, category_param, this);
    }
    connect(searchPage, &SearchPage::goToProductPage, this, &MainWindow::showProductPage);
    connect(searchPage, &SearchPage::goToDialog, this, &MainWindow::showDialogPage);
    connect(searchPage, &SearchPage::plshelpme, this, [this](){
        QString jank = AppContext::instance()->popPreviousPage();
        AppContext::instance()->pushPreviousPage(jank);
        AppContext::instance()->pushPreviousPage(jank);
        AppContext::instance()->pushPreviousPage(jank);
        goBack();
        goBack();
    });
}

void MainWindow::showDialogPage(int productId, int user1, int user2) {
    int uid = AppContext::instance()->getCurrentUser()["id"].toInt();
    if (uid != user1 && uid != user2) {
        QMessageBox::warning(this, "Ошибка", "Невозможно просмтаривать чужие переписки!");
        qDebug() << "err " << productId << user1 << user2;
        return;
    }

    AppContext::instance()->pushPreviousPage(QString("dialog:%1:%2:%3").arg(productId).arg(user1).arg(user2));
    showLoadingOverlay();
    if (dialogPage) {
        stackedWidget->removeWidget(dialogPage);
        dialogPage->setParent(nullptr);
        delete dialogPage;
    }
    qDebug() << "Показываем / создаем диалоговую страницу между пользователями " << user1
             << " и " << user2 << " по объявлению " << productId;
    int dialogId = qvariant_cast<int>(DatabaseHandler::instance()->createOrGetDialog(productId, user1, user2));
    dialogPage = new DialogPage(dialogId);
    stackedWidget->addWidget(dialogPage);
    stackedWidget->setCurrentWidget(dialogPage);
    hideLoadingOverlay();
}

// создание страницы пользователя
void MainWindow::showUserPage() {
    AppContext::instance()->pushPreviousPage("user");
    showLoadingOverlay();
    if (userPage) {
        stackedWidget->removeWidget(userPage);
        userPage->setParent(nullptr);
        delete userPage;
    }
    createUserPage();
    stackedWidget->addWidget(userPage);
    stackedWidget->setCurrentWidget(userPage);
    hideLoadingOverlay();
    connect(userPage, &UserPage::plshelpme, this, [this](){
        AppContext::instance()->pushPreviousPage("user");
        goBack();
    });
    connect(userPage, &UserPage::goMain, this, &MainWindow::showMainPage);
}

// создание страницы иного пользователя
void MainWindow::showOtherUserPage(int uid) {
    AppContext::instance()->pushPreviousPage(QString("other_user:%1").arg(uid));
    qDebug() << "showOtherUserPage uid: " << uid;
    showLoadingOverlay();
    if (userPage) {
        stackedWidget->removeWidget(userPage);
        userPage->setParent(nullptr);
        delete userPage;
    }
    createOtherUserPage(uid);
    stackedWidget->addWidget(userPage);
    stackedWidget->setCurrentWidget(userPage);
    hideLoadingOverlay();
    connect(userPage, &UserPage::goMain, this, &MainWindow::showMainPage);
}

void MainWindow::showProductPage(int productId) {
    AppContext::instance()->pushPreviousPage(QString("product:%1").arg(productId));
    showLoadingOverlay();
        if (productPage) {
            stackedWidget->removeWidget(productPage);
            productPage->setParent(nullptr);
            delete productPage;
        }
        productPage = new ProductPage(productId, this);
        connect(productPage, &ProductPage::goToUserPage, this, &MainWindow::showOtherUserPage);
        connect(productPage, &ProductPage::goToDialog, this, &MainWindow::showDialogPage);
        connect(productPage, &ProductPage::goMain, this, &MainWindow::showMainPage);
        stackedWidget->addWidget(productPage);
        stackedWidget->setCurrentWidget(productPage);
        hideLoadingOverlay();
}

// Пересоздание страницы избранного
void MainWindow::showFavouritesPage() {
    AppContext::instance()->pushPreviousPage("favourites");
    showLoadingOverlay();
    if (favouritesPage) {
        stackedWidget->removeWidget(favouritesPage);
        favouritesPage->setParent(nullptr);
        delete favouritesPage;
    }
    createFavouritesPage();
    stackedWidget->addWidget(favouritesPage);
    stackedWidget->setCurrentWidget(favouritesPage);
    hideLoadingOverlay();
}

// Пересоздание страницы сообщений
void MainWindow::showMessagesPage() {
    AppContext::instance()->pushPreviousPage("messages");
    showLoadingOverlay();
    if (dialogsPage) {
        stackedWidget->removeWidget(dialogsPage);
        dialogsPage->setParent(nullptr);
        delete dialogsPage;
    }
    createMessagesPage();
    stackedWidget->addWidget(dialogsPage);
    stackedWidget->setCurrentWidget(dialogsPage);
    hideLoadingOverlay();
}

// Создание главной страницы
void MainWindow::createMainPage() {
    mainPage = new QWidget(this);

    QVBoxLayout *mainPageLayout = new QVBoxLayout(mainPage);
    mainPageLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("border: none; margin: 0px;");

    QWidget *scrollWidget = new QWidget(this); // Контейнер для прокручиваемого содержимого
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollWidget);
    scrollWidget->setStyleSheet("border: none;");

    if (!AppContext::instance()->isLoggedIn())
    {
        QLabel *hint = new QLabel("Для того чтобы создать объявление, войдите в систему");
        hint->setStyleSheet("font-size: 24px; margin: 20px; font-weight: 100;");
        scrollLayout->addWidget(hint);
        connect(AppContext::instance(), &AppContext::userLoggedIn, hint, [hint]()
        {
            if (hint->isHidden()) hint->show();
            else hint->hide();
        });
    }
    QLabel *categoriesLabel = new QLabel("Категории", this);
    categoriesLabel->setStyleSheet("font-size: 52px; font-weight: light; margin: 20px 0;");
    scrollLayout->addWidget(categoriesLabel);

    QHBoxLayout *categories = setupCategories();
    scrollLayout->addLayout(categories);

    QLabel *newListingsLabel = new QLabel("Новое", this);
    newListingsLabel->setStyleSheet("font-size: 52px; font-weight: light; margin: 20px 0;");
    scrollLayout->addWidget(newListingsLabel);

    QWidget *listingsWidget = new QWidget(this);
    listingsWidget->setLayout(new FlowLayout(15, 15, 15)); // Используется FlowLayout вместо QScrollArea

    scrollLayout->addWidget(listingsWidget); // Добавляем виджет с объявлениями

    scrollWidget->setLayout(scrollLayout);
    scrollArea->setWidget(scrollWidget);
    mainPageLayout->addWidget(scrollArea);

    listingsArea = listingsWidget; // Привязываем listingsArea к listingsWidget
    refreshListings();
}

// Подготовка категорий

QHBoxLayout* MainWindow::setupCategories() {
    QHBoxLayout *categoriesLayout = new QHBoxLayout();

    // Запрос категорий из базы данных
    QList<QVariantMap> categories = DatabaseHandler::instance()->getMainCategories();

    for (const QVariantMap &category : categories) {
        QString categoryName = category["category"].toString();
        QByteArray imageData = category["photo"].toByteArray();

        QVBoxLayout *categoryLayout = new QVBoxLayout();
        QPushButton *categoryButton = new QPushButton(this);

        // Устанавливаем изображение кнопки
        QPixmap pixmap;
        if (!imageData.isEmpty()) {
            pixmap.loadFromData(imageData);
        } else {
            pixmap.load(":/images/placeholder.png");
        }
        QIcon icon(pixmap);
        categoryButton->setIcon(icon);
        categoryButton->setIconSize(QSize(150, 150));
        categoryButton->setStyleSheet("border: none; font-size: 32px;");

        QLabel *categoryLabel = new QLabel(categoryName, this);
        categoryLabel->setAlignment(Qt::AlignCenter);

        categoryLayout->addWidget(categoryButton);
        categoryLayout->addWidget(categoryLabel);
        categoriesLayout->addLayout(categoryLayout);

        // Обработчик клика по кнопке
        connect(categoryButton, &QPushButton::clicked, this, [this, categoryName]() {
            showSearchPage_category(categoryName);
        });
    }

    return categoriesLayout;
}


// Создание страницы пользователя
void MainWindow::createUserPage() {
    qDebug() << "Before UserPage AppContext " << AppContext::instance()->getCurrentUser()["id"] << " " << AppContext::instance()->getCurrentUser()["name"] << " " << AppContext::instance()->getCurrentUser()["role"];

    int uid = qvariant_cast<int>(AppContext::instance()->getCurrentUser()["id"]);
    userPage = new UserPage(uid, uid, "user", this);
    connect(userPage, &UserPage::goToProductPage, this, &MainWindow::showProductPage);
}

// Создание страницы пользователя
void MainWindow::createOtherUserPage(int other_uid) {
    qDebug() << "Before UserPage AppContext " << AppContext::instance()->getCurrentUser()["id"] << " " << AppContext::instance()->getCurrentUser()["name"] << " " << AppContext::instance()->getCurrentUser()["role"];

    int uid = qvariant_cast<int>(AppContext::instance()->getCurrentUser()["id"]);
    userPage = new UserPage(uid, other_uid, AppContext::instance()->getCurrentUser()["role"].toString(), this);
    connect(userPage, &UserPage::goToProductPage, this, &MainWindow::showProductPage);
    connect(AppContext::instance(), &AppContext::userLoggedIn, userPage, [this]()
    {
            QString jank = AppContext::instance()->popPreviousPage();
            if (jank == "main")
            {
                AppContext::instance()->pushPreviousPage(jank);
            } else {
                AppContext::instance()->pushPreviousPage(jank);
                AppContext::instance()->pushPreviousPage(jank);
                goBack();
            }
    });

}

// Создание страницы избранного
void MainWindow::createFavouritesPage() {
    qDebug() << "visiting favourites page of user";
    favouritesPage = new FavouritesPage(this);
    connect(favouritesPage, &FavouritesPage::goToProductPage, this, &MainWindow::showProductPage);
    connect(favouritesPage, &FavouritesPage::goToDialog, this, &MainWindow::showDialogPage);
}

// Создание страницы сообщений
void MainWindow::createMessagesPage() {
    dialogsPage = new DialogsPage(this);
    connect(dialogsPage, &DialogsPage::goToPostPage, this, &MainWindow::showProductPage);
    connect(dialogsPage, &DialogsPage::goToDialogPage, this, &MainWindow::showDialogPage);
    connect(dialogsPage, &DialogsPage::goToUserPage, this, &MainWindow::showUserPage);
}


void MainWindow::refreshListings() {
    QList<QVariantMap> adsList = DatabaseHandler::instance()->getLatestAds(36);

    QLayout *listingsLayout = listingsArea->layout();
    if (listingsLayout != nullptr) {
        QLayoutItem *child;
        while ((child = listingsLayout->takeAt(0)) != nullptr) {
            delete child->widget();  // Удаляем виджеты
            delete child;            // Удаляем элементы макета
        }
        delete listingsLayout;
    }

    FlowLayout *newListingsLayout = new FlowLayout(20, 20, 20); // Уменьшено расстояние
    listingsArea->setLayout(newListingsLayout);

    for (const QVariantMap &ad : adsList) {
        int productId = ad["id"].toInt(); // Получаем ID товара
        ListingCard *listingCard = new ListingCard(productId, this);
        listingCard->setFrameShape(QFrame::StyledPanel);
        listingCard->setStyleSheet(
            "background-color: #ffffff;"
            "border: 1px solid #ddd;"
            "border-radius: 15px;"    // Округлые углы
            "padding: 0px;"
        );
        listingCard->setCursor(Qt::PointingHandCursor);

        QVBoxLayout *cardLayout = new QVBoxLayout(listingCard);
        cardLayout->setSpacing(10);
        cardLayout->setContentsMargins(10, 10, 10, 10);

        // Контейнер для изображения
        QWidget *imageContainer = new QWidget(listingCard);
        imageContainer->setFixedSize(350, 200); // Уменьшенный размер изображения
        imageContainer->setStyleSheet(
            "background-color: #f9f9f9;"
            "border-radius: 15px 15px 0 0;"
            "position: relative;"
        );

        QLabel *image = new QLabel(imageContainer);
        QByteArray photoData = ad["photo"].toByteArray();
        if (!photoData.isEmpty()) {
            QPixmap pixmap;
            pixmap.loadFromData(photoData);
            image->setPixmap(pixmap.scaled(350, 200, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        } else {
            image->setText("Нет изображения");
            image->setAlignment(Qt::AlignCenter);
        }
        image->setFixedSize(350, 200);
        image->setStyleSheet("border-radius: 15px 15px 0 0;");
        QVBoxLayout *imageLayout = new QVBoxLayout(imageContainer);
        imageLayout->setContentsMargins(0, 0, 0, 0);
        imageLayout->addWidget(image);

        // Кнопка "сердечко"
        if (ad["user"] != AppContext::instance()->getCurrentUser()["id"] && !AppContext::instance()->isAdmin())
        {
            QPushButton *favIcon = new QPushButton(imageContainer);
            favIcon->setFixedSize(50, 50); // Уменьшенная кнопка
            favIcon->setIconSize(QSize(45, 45));
            if (AppContext::instance()->isFavorite(productId)) {
                favIcon->setIcon(QIcon(":/images/heart.png"));
                favIcon->setStyleSheet(
                    "background-color: #ffffff;"
                    "border: 3px solid #dc49b0;"
                    "border-radius: 25px;"
                    "position: absolute;"
                );
            }
            else {
                favIcon->setIcon(QIcon(":/images/greyed_heart.png"));
                favIcon->setStyleSheet(
                    "background-color: #ffffff;"
                    "border: 3px solid #8c8b8c;"
                    "border-radius: 25px;"
                    "position: absolute;"
                );
            }
            favIcon->move(300, 10);
            if (AppContext::instance()->isLoggedIn()){
                connect(favIcon, &QPushButton::clicked, this, [productId, favIcon]() {
                    if (AppContext::instance()->isFavorite(productId)) {
                        if (DatabaseHandler::instance()->removeFavourite(qvariant_cast<int>(AppContext::instance()->getCurrentUser()["id"]), productId)) {
                            AppContext::instance()->removeFromFavorites(productId);
                            favIcon->setIcon(QIcon(":/images/greyed_heart.png"));
                            favIcon->setStyleSheet(
                                "background-color: #ffffff;"
                                "border: 3px solid #8c8b8c;"
                                "border-radius: 25px;"
                                "position: absolute;"
                            );
                        }
                    } else {
                        if (DatabaseHandler::instance()->addFavourite(qvariant_cast<int>(AppContext::instance()->getCurrentUser()["id"]), productId)) {
                            AppContext::instance()->addToFavorites(productId);
                            favIcon->setIcon(QIcon(":/images/heart.png"));
                            favIcon->setStyleSheet(
                                "background-color: #ffffff;"
                                "border: 3px solid #dc49b0;"
                                "border-radius: 25px;"
                                "position: absolute;"
                            );
                        }
                    }
                });
            } else {
                connect(favIcon, &QPushButton::clicked, this, [this](){
                        LoginDialog loginDialog(this);
                        if (loginDialog.exec() == QDialog::Accepted) {
                            this->refreshListings();
                        }
                });
            }
        }

        cardLayout->addWidget(imageContainer);

        // Заголовок
        QLabel *title = new QLabel(ad["name"].toString(), this);
        title->setStyleSheet("font-size: 18px; font-weight: bold; margin-top: 5px; border: none;");

        cardLayout->addWidget(title);

        // Цена, имя пользователя и рейтинг в одной строке
        QHBoxLayout *topRow = new QHBoxLayout();
        QLabel *price = new QLabel(QString::number(ad["price"].toDouble()) + " руб.", this);
        price->setStyleSheet("border: none; font-size: 24px; color: #E91E63;");

        QLabel *username = new QLabel(ad["username"].toString(), this);
        username->setStyleSheet("border: none; font-size: 20px; color: #555;");

        double reviewsValue = ad["reviews"].toDouble();
        QLabel *reviewstarlol = new QLabel(" ★", this);
        reviewstarlol->setStyleSheet("border: none; font-size: 20px; color: ;");

        QLabel *reviews = new QLabel(QString::number(reviewsValue, 'f', 2), this);
        reviews->setStyleSheet("border: none; font-size: 20px;");

        topRow->addWidget(price);
        topRow->addStretch(5);
        topRow->addWidget(username);
        topRow->addStretch(1);
        topRow->addWidget(reviews);
        topRow->addWidget(reviewstarlol);


        cardLayout->addLayout(topRow);

        // Дата и город
        QHBoxLayout *lowerrow = new QHBoxLayout();
        QString formattedDate = QDateTime::fromString(ad["date"].toString(), Qt::ISODate).toString("dd MMMM yyyy, HH:mm");
        QLabel *location = new QLabel(ad["geolocation"].toString(), this);
        location->setStyleSheet("border: none; font-size: 14px; color: #888;");
        QLabel *date = new QLabel(formattedDate, this);
        date->setStyleSheet("border: none; font-size: 14px; color: #888;");

        lowerrow->addWidget(location);
        lowerrow->addStretch();
        lowerrow->addWidget(date);
        cardLayout->addLayout(lowerrow);

        newListingsLayout->addWidget(listingCard);
        connect(listingCard, &ListingCard::clicked, this, &MainWindow::showProductPage);
    }
}

void MainWindow::showAdminPanel() {
    AppContext::instance()->pushPreviousPage("admin");
    showLoadingOverlay();
    if (adminPage) {
        stackedWidget->removeWidget(adminPage);
        adminPage->setParent(nullptr);
        delete adminPage;
    }
    adminPage = new AdminPage(this);
    stackedWidget->addWidget(adminPage);
    stackedWidget->setCurrentWidget(adminPage);
    hideLoadingOverlay();
}


// Деструктор
MainWindow::~MainWindow() {
    delete ui;
}
