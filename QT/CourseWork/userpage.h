#ifndef USERPAGE_H
#define USERPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStackedWidget>
#include <QListWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QDate>

#include "appcontext.h"

class UserPage : public QWidget {
    Q_OBJECT

public:
    explicit UserPage(int requestingUserId, int requestedUserId, const QString &requestingUserRole, QWidget *parent = nullptr);

signals:
    void goToProductPage(int adId);
    void goToDialog(int adId);
    void plshelpme();
    void goMain();

private:
    int requestingUserId;
    int requestedUserId;
    QString requestingUserRole;

    QStackedWidget *stackedWidget;
    QWidget *infoWidget;
    QWidget *adsPage;
    QWidget *reviewsPage;
    QListWidget *adsListWidget;
    QListWidget *reviewsListWidget;

    QLabel *usernameLabel; // Лейбл для имени пользователя

    void setupUserInfo();  // Метод для отображения информации о пользователе

public slots:
    void showAdsPage();
    void showReviewsPage();
    void setupAdsPage();
    void setupReviewsPage();
    void updateAds();
    void updateReviews();
};

#endif // USERPAGE_H
