#ifndef DATABASEHANDLER_H
#define DATABASEHANDLER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariantMap>
#include <QList>

class DatabaseHandler : public QObject
{
    Q_OBJECT

private:
    explicit DatabaseHandler(QObject *parent = nullptr);
    bool initializeDatabase();

    void updateUserRatings();

    static DatabaseHandler* m_instance;
    QSqlDatabase db;

public:
    static DatabaseHandler* instance();
    ~DatabaseHandler();

    // Пользователи
    bool verifyUser(const QString &email, const QString &password, QVariantMap &userDetails);
    bool verifyUser(const QString &email, const QString &password);
    bool registerUser(const QString &email, const QString &password, const QString &name,
                      const QByteArray &photo = nullptr, const QString &about_me = nullptr, const QString &role = "user");
    bool deleteUser(int userId);
    bool updateUser(int userId, QVariantMap &updatedData);
    QVariantMap getUserInfo(int userId);
    QList<QVariantMap> getAllUsers();

    // Категории
    bool createCategory(const QString &name, const QByteArray &photo, int inheritsId = -1);
    bool createCategory(const QString &name, int inheritsId = -1);
    bool deleteCategory(int categoryId);
    QList<QString> getCategoryNames();
    QList<QVariantMap> getMainCategories();
    QString getCategoryIdFromName(const QString &name);
    QString getCategoryNameFromId(int id);


    // Объявления
    bool createAd(const QVariantMap &adData);
    bool updateAd(const QVariantMap &ad);
    bool deactivateAd(int adId);
    bool activateAd(int adId);
    bool deleteAd(int adId);
    QList<QVariantMap> getUserAds(int userId);
    QList<QVariantMap> searchAds(const QString &text, const QString &location, const QString &category,
                                 double minPrice, double maxPrice, int searcher_id);
    QList<QVariantMap> getLatestAds(int count);
    QVariantMap getAdById(int ad_id);

    // Избранное
    bool addFavourite(int userId, int adId);
    bool removeFavourite(int userId, int adId);
    QList<QVariantMap> getUserFavourites(int userId);

    // Диалоги
    QList<QVariantMap> getIncomingDialogs(int userId);
    QList<QVariantMap> getOutgoingDialogs(int userId);
    int createOrGetDialog(int adId, int user1Id, int user2Id);
    QList<QVariantMap> getMessages(int dialogId);
    bool sendMessage(int dialogId, int senderId, const QString &text);

    // Отзывы
    bool addReview(int reviewerId, int reviewedId, int rating, const QString &reviewText);
    bool deleteReview(int reviewId);
    void updateUserRating(int userId);
    QList<QVariantMap> getUserReviews(int userId);

    // Жалобы
    bool addComplaint(int targetId, bool isAd, const QString &reason, const QString &explanation);
    QList<QVariantMap> getComplaints();
    bool resolveComplaint(int complaintId);
};

#endif // DATABASEHANDLER_H
