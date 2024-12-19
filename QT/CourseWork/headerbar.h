#ifndef HEADERBAR_H
#define HEADERBAR_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QDialog>
#include <QFormLayout>
#include <QMessageBox>

#include "logindialog.h"
#include "registerdialog.h"
#include "databasehandler.h"
#include "appcontext.h"
#include "addlistingdialog.h"
#include "QObject"
#include "QDebug"

class HeaderBar : public QWidget
{
    Q_OBJECT
public:
    explicit HeaderBar(QWidget *parent = nullptr);
    void updateHeader();

signals:
    void goBack();
    void goToMainPage();
    void goToUserPage();
    void goToFavouritesPage();
    void goToMessagesPage();
    void listingsUpdated();
    void searchInit(QString search_text);
    void goToAdminPanel();


private slots:
    void showLoginDialog();
    void showRegisterDialog();
    void createListing();

private:
    QVBoxLayout *mainLayout;
    QHBoxLayout *topBarLayout;

    QPushButton *logo;
    QLineEdit *searchBar;
    QPushButton *searchButton;

    QPushButton *loginButton;
    QPushButton *registerButton;

    QPushButton *userIcon;
    QPushButton *favoritesIcon;
    QPushButton *messagesIcon;
    QPushButton *addListingButton;
    QPushButton *backButton;

    QPushButton *adminButton;
    QPushButton *adminLogoutButton;

    QLineEdit *loginEmail;
    QLineEdit *loginPassword;
    QLineEdit *registerName;
    QLineEdit *registerEmail;
    QLineEdit *registerPassword;

};

#endif // HEADERBAR_H
