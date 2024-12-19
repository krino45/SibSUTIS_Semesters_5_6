#include "headerbar.h"

HeaderBar::HeaderBar(QWidget *parent) : QWidget(parent), adminButton(nullptr)
{
    mainLayout = new QVBoxLayout(this);

    // Top Navigation Bar
    QFrame *topBar = new QFrame(this);
    topBar->setContentsMargins(0, 0, 0, 0);
    topBar->setFrameShape(QFrame::NoFrame);
    topBar->setStyleSheet("background-color: #2B2228; padding: 10px; margin: 0px;");
    topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(0, 0, 0, 0);

    backButton = new QPushButton("<", this);
        backButton->setMinimumSize(60, 60);
        backButton->setStyleSheet(R"(
            QPushButton { font-size: 28px; font-weight: bold; background-color: #DC49B0; color: white; border-radius: 10px; }
            QPushButton:hover { background-color: #CC39A0; }
            QPushButton:pressed { background-color: #BC2990; }
        )");

        connect(backButton, &QPushButton::clicked, this, [this]() {
            if (AppContext::instance()->hasPreviousPage()) {
                emit goBack();
            }
        });


    // Логотип
    logo = new QPushButton(this);
    //logo->setPixmap(QPixmap(":/images/logo.png").scaled(250, 250, Qt::KeepAspectRatio));
    logo->setMinimumSize(400,120);
    logo->setStyleSheet("border-image: url(:/images/logo.png); border: none; margin-right: 10px;");

    // Поиск
    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText("Поиск объявлений...");
    searchBar->setMinimumSize(400, 60);
    searchBar->setStyleSheet("border-radius: 7px; padding: 5px; background-color: #FFFFFF;");

    searchButton = new QPushButton(QIcon(), "->", this);
    searchButton->setMinimumSize(60, 60);
    searchButton->setStyleSheet("QPushButton { border: none; font-size: 28px; border-radius: 10px; color: black; background-color: #DC49B0; }"
                                "QPushButton:hover { background-color: #CC39A0; }");
    connect(searchButton, &QPushButton::clicked, this, [this](){
        QString param = this->searchBar->text();
        emit searchInit(param);
    });

    // Кнопки входа и регистрации
    loginButton = new QPushButton("Вход", this);
    loginButton->setMinimumSize(150, 85);
    loginButton->setStyleSheet(R"(
                               QPushButton { border: none; font-size: 28px; background-color: #DC49B0;
                                             color: white; border-radius: 32px; padding: 5px 5px; }
                               QPushButton:hover { background-color: #CC39A0; }
                               QPushButton:pressed { background-color: #BC2990; }
                               )");
    connect(loginButton, &QPushButton::clicked, this, &HeaderBar::showLoginDialog);

    registerButton = new QPushButton("Регистрация", this);
    registerButton->setMinimumSize(250, 85);
    registerButton->setStyleSheet(R"(
                                  QPushButton { border: none; font-size: 28px; background-color: #6C757D;
                                                color: white; border-radius: 32px; padding: 5px 5px; }
                                  QPushButton:hover { background-color: #5C656D; }
                                  QPushButton:pressed { background-color: #2C454D; }
                                  )");
    connect(registerButton, &QPushButton::clicked, this, &HeaderBar::showRegisterDialog);

    userIcon = new QPushButton(this);
    favoritesIcon = new QPushButton(this);
    messagesIcon = new QPushButton(this);
    userIcon->setMinimumSize(70,70);
    favoritesIcon->setMinimumSize(70,70);
    messagesIcon->setMinimumSize(70,70);

    // Убираем тень, задаем стиль для кнопок
    userIcon->setStyleSheet("QPushButton { border-image: url(:/images/user.png); border: none; }"
                            "QPushButton:hover { background-color: rgba(0, 0, 0, 20%); }"
                            "QPushButton:pressed { background-color: rgba(0, 0, 0, 40%); }");

    favoritesIcon->setStyleSheet("QPushButton { border-image: url(:/images/heart.png); border: none; }"
                                 "QPushButton:hover { background-color: rgba(0, 0, 0, 20%); }"
                                 "QPushButton:pressed { background-color: rgba(0, 0, 0, 40%); }");

    messagesIcon->setStyleSheet("QPushButton { border-image: url(:/images/dialog.png); border: none; }"
                                "QPushButton:hover { background-color: rgba(0, 0, 0, 20%); }"
                                "QPushButton:pressed { background-color: rgba(0, 0, 0, 40%); }");


    addListingButton = new QPushButton("Разместить объявление", this);
    addListingButton->setMinimumSize(350, 85);
    addListingButton->setStyleSheet(R"(
                                    QPushButton { border: none; font-size: 28px; background-color: #DC49B0;
                                                  color: white; border-radius: 32px; padding: 5px 5px; }
                                    QPushButton:hover { background-color: #CC39A0; }
                                    QPushButton:pressed { background-color: #BC2990; }
                                    )");

    adminButton = new QPushButton("Управление", this);
    adminButton->setMinimumSize(200, 85);
    adminButton->setStyleSheet(R"(
        QPushButton { border: none; font-size: 28px; background-color: #EE5733;
                      color: white; border-radius: 32px; padding: 5px 5px; }
        QPushButton:hover { background-color: #E64A19; }
        QPushButton:pressed { background-color: #D84315; }
    )");

    adminLogoutButton = new QPushButton(this);
    adminLogoutButton->setMinimumSize(70,70);
    adminLogoutButton->setStyleSheet("QPushButton { border-image: url(:/images/logout.png); border: none; }"
                            "QPushButton:hover { background-color: rgba(0, 0, 0, 20%); }"
                            "QPushButton:pressed { background-color: rgba(0, 0, 0, 40%); }");

    connect(addListingButton, &QPushButton::clicked, this, &HeaderBar::createListing);
    connect(logo, &QPushButton::clicked, this, &HeaderBar::goToMainPage);
    connect(userIcon, &QPushButton::clicked, this, &HeaderBar::goToUserPage);
    connect(favoritesIcon, &QPushButton::clicked, this, &HeaderBar::goToFavouritesPage);
    connect(messagesIcon, &QPushButton::clicked, this, &HeaderBar::goToMessagesPage);
    connect(adminButton, &QPushButton::clicked, this, [this]() {emit goToAdminPanel(); });
    connect(adminLogoutButton, &QPushButton::clicked, this, []() { AppContext::instance()->clearCurrentUser(); });


    // Обработчик смены состояния пользователя
    connect(AppContext::instance(), &AppContext::userLoggedIn, this, &HeaderBar::updateHeader);
    connect(AppContext::instance(), &AppContext::userLoggedOut, this, &HeaderBar::updateHeader);
    updateHeader();

    // Расположение всех элементов в горизонтальном Layout
    topBarLayout->addWidget(backButton);
    topBarLayout->addWidget(logo);

    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->setContentsMargins(0, 0, 0, 0);
    searchLayout->setSpacing(0);                  // Убираем промежутки между виджетами

    searchLayout->addWidget(searchBar);
    searchLayout->addWidget(searchButton);
    connect(searchBar, &QLineEdit::returnPressed, searchButton, &QPushButton::click);
    topBarLayout->addLayout(searchLayout);

    topBarLayout->addStretch();

    topBarLayout->addWidget(userIcon);
    topBarLayout->addWidget(favoritesIcon);
    topBarLayout->addWidget(messagesIcon);
    topBarLayout->addWidget(addListingButton);
    topBarLayout->addWidget(adminButton);
    topBarLayout->addWidget(adminLogoutButton);

    // Расположение кнопок для незалогиненных пользователей
    topBarLayout->addWidget(loginButton);
    topBarLayout->addWidget(registerButton);

    mainLayout->addWidget(topBar);
    setLayout(mainLayout);
}

void HeaderBar::updateHeader()
{
    if (AppContext::instance()->isLoggedIn()) {
        if (AppContext::instance()->isAdmin()) {
            // Для админа
            loginButton->hide();
            registerButton->hide();

            userIcon->hide();
            addListingButton->hide(); // Админ не добавляет объявления
            favoritesIcon->hide();    // Избранное скрыто
            messagesIcon->hide();     // Сообщения скрыты

            adminButton->show();
            adminLogoutButton->show();
        } else {
            // Для обычного пользователя
            loginButton->hide();
            registerButton->hide();
            if (adminButton) {
                adminButton->hide();
                adminLogoutButton->hide();
            }
            userIcon->show();
            favoritesIcon->show();
            messagesIcon->show();
            addListingButton->show();
        }
    } else {
        // Неавторизованный пользователь
        userIcon->hide();
        favoritesIcon->hide();
        messagesIcon->hide();
        addListingButton->hide();

        if (adminButton) {
            adminButton->hide();
            adminLogoutButton->hide();
        }

        loginButton->show();
        registerButton->show();
    }
}

void HeaderBar::createListing()
{
    AddListingDialog addListingDialog;
    if (addListingDialog.exec() == QDialog::Accepted) {
        qDebug() << "Объявление добавлено!";
    }
    emit listingsUpdated();
}

void HeaderBar::showRegisterDialog()
{
    RegisterDialog registerDialog(this);
    registerDialog.exec();
}

void HeaderBar::showLoginDialog()
{
    LoginDialog loginDialog(this);
    if (loginDialog.exec() == QDialog::Accepted) {
        // Обновляем интерфейс HeaderBar
        updateHeader();
        emit listingsUpdated();
    }
}
