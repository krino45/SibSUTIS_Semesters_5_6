#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include <QObject>
#include <QStack>
#include <QVariantMap>

class AppContext : public QObject {
    Q_OBJECT

public:
    static AppContext* instance() {
        static AppContext ctx;
        return &ctx;
    }

    bool isLoggedIn() const { return !currentUser.isEmpty(); }
    QVariantMap getCurrentUser() const { return currentUser; }
    bool isAdmin() const {
        return getCurrentUser()["role"].toString() == "admin";
    }


    void pushPreviousPage(const QString &page) {
        pageHistory.push(page);
    }

    QString popPreviousPage() {
        if (!pageHistory.isEmpty()) {
            return pageHistory.pop();
        }
        return {};
    }

    bool hasPreviousPage() const {
        return !pageHistory.isEmpty();
    }

    void setCurrentUser(QVariantMap user) {
        currentUser["id"] = user["id"];
        currentUser["name"] = user["name"];
        currentUser["role"] = user["role"];
        emit userLoggedIn();
    }

    void clearCurrentUser() {
        currentUser.clear();
        pageHistory.clear();
        userFavorites.clear();
        emit userLoggedOut();
    }

    void setUserFavorites(const QList<int> &favorites);
    const QList<int> &getUserFavorites() const;

    void addToFavorites(int adId);
    void removeFromFavorites(int adId);
    bool isFavorite(int adId) const;


signals:
    void userLoggedIn();
    void userLoggedOut();

private:
    AppContext() {}
    QVariantMap currentUser;
    QStack<QString> pageHistory;
    QList<int> userFavorites;
};

#endif // APPCONTEXT_H
