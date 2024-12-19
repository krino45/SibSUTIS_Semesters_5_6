#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QStackedWidget>
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QFrame>
#include <QDebug>
#include <QMenuBar>
#include <QScrollArea>
#include <QStatusBar>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

#include "databasehandler.h"
#include "headerbar.h"
#include "userpage.h"
#include "favouritespage.h"
#include "messagespage.h"
#include "appcontext.h"
#include "listingcard.h"
#include "productpage.h"
#include "searchpage.h"
#include "dialogspage.h"
#include "dialogpage.h"
#include "adminpage.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void refreshListings();

private:
    QStackedWidget *stackedWidget;
    HeaderBar *headerBar;
    QWidget *mainPage;
    SearchPage *searchPage;
    UserPage *userPage;
    FavouritesPage *favouritesPage;
    MessagesPage *messagesPage;
    DialogsPage *dialogsPage;
    DialogPage *dialogPage;
    ProductPage *productPage;
    AdminPage *adminPage;
    QWidget *listingsArea;
    Ui::MainWindow *ui;
    QWidget *loadingOverlay;
    QGraphicsOpacityEffect *opacityEffect;
    QPropertyAnimation *fadeAnimation;

    void showLoadingOverlay();
    void hideLoadingOverlay();
    void goBack();
    void resizeEvent(QResizeEvent*);

private slots:
    void showMainPage();
    void showSearchPage_text(QString search_param);
    void showSearchPage_category(QString search_param);
    void showProductPage(int productId);
    void showUserPage();
    void showDialogPage(int productId, int user1, int user2);
    void showFavouritesPage();
    void showMessagesPage();
    void showOtherUserPage(int userId);
    void showAdminPanel();
    QHBoxLayout* setupCategories();
    void createMainPage();
    void createUserPage();
    void createOtherUserPage(int other_uid);
    void createFavouritesPage();
    void createMessagesPage();
    void createSearchPage(QString text_param, QString category_param);

};

#endif // MAINWINDOW_H


