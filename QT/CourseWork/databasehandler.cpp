#include "databasehandler.h"
#include "appcontext.h"
#include <QDebug>
#include <QCoreApplication>


DatabaseHandler* DatabaseHandler::m_instance = nullptr;

DatabaseHandler::~DatabaseHandler()
{
    if (db.isOpen()) {
        db.close();
    }
}

DatabaseHandler* DatabaseHandler::instance()
{
    if (m_instance == nullptr) {
        m_instance = new DatabaseHandler();
    }
    return m_instance;
}

DatabaseHandler::DatabaseHandler(QObject *parent) : QObject(parent)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = QCoreApplication::applicationDirPath() + "../../../CourseWork/app_database.db";
    db.setDatabaseName(dbPath);
    initializeDatabase();
}

bool DatabaseHandler::initializeDatabase() {
    if (!db.open()) {
        qWarning() << "Не получилось открыть базу данных:" << db.lastError().text();
        return false;
    }

    QSqlQuery query;
    query.exec("PRAGMA foreign_keys = ON;");

    // Create Users table
    query.exec(R"(CREATE TABLE IF NOT EXISTS users (
               id INTEGER PRIMARY KEY AUTOINCREMENT,
               email TEXT UNIQUE NOT NULL,
               password TEXT NOT NULL,
               name TEXT NOT NULL,
               role TEXT CHECK(role IN ('admin', 'user')) DEFAULT 'user',
               reviews REAL DEFAULT 0,
               profile_picture BLOB,
               about_me TEXT
               ))");

    // Create Reviews table
    query.exec(R"(CREATE TABLE IF NOT EXISTS reviews (
               id INTEGER PRIMARY KEY AUTOINCREMENT,
               reviewer INTEGER REFERENCES users(id) ON DELETE CASCADE,
               reviewed INTEGER REFERENCES users(id) ON DELETE CASCADE,
               rating INTEGER CHECK(rating BETWEEN 1 AND 5),
               review_text TEXT,
               date TEXT DEFAULT CURRENT_TIMESTAMP
           ))");

    // Create Ads table
    query.exec(R"(CREATE TABLE IF NOT EXISTS ads (
               id INTEGER PRIMARY KEY AUTOINCREMENT,
               date TEXT DEFAULT CURRENT_TIMESTAMP,
               user INTEGER REFERENCES users(id) ON DELETE CASCADE,
               category INTEGER REFERENCES categories(id) ON DELETE SET NULL,
               name TEXT NOT NULL,
               description TEXT,
               geolocation TEXT,
               price REAL NOT NULL,
               phone TEXT NOT NULL,
               photo BLOB,
               active BOOLEAN DEFAULT TRUE
           ))");

    // Create Categories table
    query.exec(R"(CREATE TABLE IF NOT EXISTS categories (
               id INTEGER PRIMARY KEY AUTOINCREMENT,
               category TEXT NOT NULL,
               photo BLOB,
               inherits INTEGER REFERENCES categories(id) ON DELETE CASCADE
           ))");

    // Create Favourites table
    query.exec(R"(CREATE TABLE IF NOT EXISTS favourite (
               id INTEGER PRIMARY KEY AUTOINCREMENT,
               ad INTEGER REFERENCES ads(id) ON DELETE CASCADE,
               user INTEGER REFERENCES users(id) ON DELETE CASCADE,
               UNIQUE(ad, user)
           );)");

    // Create Dialogs table
    query.exec(R"(CREATE TABLE IF NOT EXISTS dialogs (
               id INTEGER PRIMARY KEY AUTOINCREMENT,
               ad_id INTEGER NOT NULL,
               user1_id INTEGER NOT NULL,
               user2_id INTEGER NOT NULL,
               last_message TEXT,
               last_message_date DATETIME DEFAULT CURRENT_TIMESTAMP,
               FOREIGN KEY (ad_id) REFERENCES ads(id) ON DELETE CASCADE,
               FOREIGN KEY (user1_id) REFERENCES users(id) ON DELETE CASCADE,
               FOREIGN KEY (user2_id) REFERENCES users(id) ON DELETE CASCADE);)");

    query.exec(R"(CREATE TABLE IF NOT EXISTS messages (
               id INTEGER PRIMARY KEY AUTOINCREMENT,
               dialog_id INTEGER NOT NULL,
               sender_id INTEGER NOT NULL,
               text TEXT NOT NULL,
               sent_date DATETIME DEFAULT CURRENT_TIMESTAMP,
               FOREIGN KEY (dialog_id) REFERENCES dialogs(id) ON DELETE CASCADE,
               FOREIGN KEY (sender_id) REFERENCES users(id) ON DELETE CASCADE);)");

    query.exec(R"(CREATE TABLE IF NOT EXISTS complaints (
               id INTEGER PRIMARY KEY,
               target_id INTEGER NOT NULL,
               target_ad INTEGER DEFAULT 0,
               author_id INTEGER NOT NULL,
               reason TEXT NOT NULL,
               explanation TEXT,
               FOREIGN KEY (target_id) REFERENCES users(id) ON DELETE CASCADE,
               FOREIGN KEY (author_id) REFERENCES users(id) ON DELETE CASCADE
               );)");


    if (query.lastError().isValid()) {
        qWarning() << "Ошибка создания таблиц:" << query.lastError().text();
        return false;
    }
    return true;
}

QList<QVariantMap> DatabaseHandler::getLatestAds(int count)
{

    QList<QVariantMap> adsList;
    QSqlQuery query;

    query.prepare("SELECT ads.*, users.name AS username, users.reviews FROM ads JOIN users ON ads.user = users.id WHERE ads.active = TRUE ORDER BY ads.date DESC LIMIT :count");
    query.bindValue(":count", count);

    qDebug() << "Получение последних объявлений. Параметры: count =" << count;

    if (!query.exec()) {
        qWarning() << "Не получилось получить объявления по причине:" << query.lastError().text();
        return adsList;
    }

    while (query.next()) {
        QVariantMap ad;
        ad["id"] = query.value("id");
        ad["date"] = query.value("date");
        ad["user"] = query.value("user");
        ad["category"] = query.value("category");
        ad["name"] = query.value("name");
        ad["description"] = query.value("description");
        ad["geolocation"] = query.value("geolocation");
        ad["price"] = query.value("price");
        ad["phone"] = query.value("phone");
        ad["photo"] = query.value("photo");
        ad["active"] = query.value("active");
        ad["username"] = query.value("username");
        ad["reviews"] = query.value("reviews");
        adsList.append(ad);
    }

    qDebug() << "Возвращены объявления:" << adsList;
    return adsList;
}

QList<QVariantMap> DatabaseHandler::getAllUsers() {
    qDebug() << "getting all users";
    QSqlQuery query;
    query.prepare("SELECT * FROM users");
    if (!query.exec())
    {
        qDebug() << "Не получилось получить users...";
        return QList<QVariantMap>();
    }
    QList<QVariantMap> list;
    while (query.next()) {
        QVariantMap varmap;
        varmap["id"] = query.value("id");
        varmap["name"] = query.value("name");
        varmap["email"] = query.value("email");
        varmap["role"] = query.value("role");
        varmap["profile_picture"] = query.value("profile_picture");
        varmap["about_me"] = query.value("about_me");
        varmap["reviews"] = query.value("reviews");
        list.append(varmap);
    }

    return list;
}

bool DatabaseHandler::deleteUser(int userId)
{
    qDebug() << "Запрос на удаление пользователя с ID:" << userId;

    QSqlQuery query;
    query.prepare("DELETE FROM users WHERE id = :id");
    query.bindValue(":id", userId);

    if (!query.exec()) {
        qWarning() << "Не удалось удалить пользователя:" << query.lastError().text();
        return false;
    }

    qWarning() << "Проверка на ошибки:" << query.lastError().text();

    qDebug() << "пользователь удален успешно. обновляю рейтинги...";
    updateUserRatings();
    return true;
}

// Пользователи
bool DatabaseHandler::registerUser(const QString &email, const QString &password, const QString &name, const QByteArray &photo, const QString &about_me, const QString &role) {
    qDebug() << "Регистрация пользователя. Параметры: email =" << email << ", name =" << name << ", role =" << role;

    QSqlQuery query;
    query.prepare("INSERT INTO users (email, password, name, profile_picture, about_me, role) VALUES (:email, :password, :name, :profile_picture, :about_me, :role)");

    query.bindValue(":email", email);
    query.bindValue(":password", password);
    query.bindValue(":name", name);
    query.bindValue(":profile_picture", photo);
    query.bindValue(":about_me", about_me);
    query.bindValue(":role", role);

    if (!query.exec()) {
        qWarning() << "Не получилось зарегистрировать пользователя:" << query.lastError().text();
        return false;
    }

    qDebug() << "Пользователь зарегистрирован успешно.";
    return true;
}

bool DatabaseHandler::verifyUser(const QString &email, const QString &password, QVariantMap &userDetails)
{
    qDebug() << "Проверка пользователя. Параметры: email =" << email;

    QSqlQuery query;
    query.prepare("SELECT id, name, role FROM users WHERE email = :email AND password = :password");
    query.bindValue(":email", email);
    query.bindValue(":password", password);

    if (!query.exec()) {
        qWarning() << "Не удалось выполнить запрос:" << query.lastError().text();
        return false;
    }

    if (!query.next()) {
        qWarning() << "Пользователь не найден или неверный пароль.";
        return false;
    }

    userDetails["id"] = query.value("id");
    userDetails["name"] = query.value("name");
    userDetails["role"] = query.value("role");

    qDebug() << "Пользователь найден. Информация:" << userDetails;
    return true;
}

bool DatabaseHandler::verifyUser(const QString &email, const QString &password)
{
    qDebug() << "Проверка пользователя. Параметры: email =" << email;

    QSqlQuery query;
    query.prepare("SELECT id, name, role FROM users WHERE email = :email AND password = :password");
    query.bindValue(":email", email);
    query.bindValue(":password", password);

    if (!query.exec()) {
        qWarning() << "Не удалось выполнить запрос:" << query.lastError().text();
        return false;
    }

    if (!query.next()) {
        qWarning() << "Пользователь не найден или неверный пароль.";
        return false;
    }

    qDebug() << "Пользователь найден.";
    return true;
}

QVariantMap DatabaseHandler::getUserInfo(int uid)
{

    qDebug() << "Получение информации о пользователе. Параметры: uid =" << uid;

    QVariantMap varmap;
    QSqlQuery query;
    query.prepare("SELECT id, name, email, role, reviews, profile_picture, about_me FROM users WHERE id = :id");
    query.bindValue(":id", uid);

    if (!query.exec()) {
        qWarning() << "Не удалось выполнить запрос:" << query.lastError().text();
    }

    if (!query.next()) {
        qWarning() << "Не удалось получить информацию о пользователе: пользователь не найден.";
    }

    varmap["id"] = query.value("id");
    varmap["name"] = query.value("name");
    varmap["email"] = query.value("email");
    varmap["role"] = query.value("role");
    varmap["profile_picture"] = query.value("profile_picture");
    varmap["about_me"] = query.value("about_me");
    varmap["reviews"] = query.value("reviews");

    qDebug() << "Информация о пользователе:" << varmap;
    return varmap;
}

bool DatabaseHandler::updateUser(int userId, QVariantMap &updatedData)
{
    qDebug() << "updating user " << userId << " to " << updatedData;
    QSqlQuery query;
    query.prepare(R"(
                  UPDATE users SET
                    name = :name,
                    email = :email,
                    about_me = :about_me,
                    profile_picture = :pfp,
                    password = :pwd
                    WHERE id = :id;
                  )");
    query.bindValue(":name", updatedData["name"].toString());
    query.bindValue(":email", updatedData["email"].toString());
    query.bindValue(":about_me", updatedData["about_me"].toString());
    query.bindValue(":pfp", updatedData["profile_picture"]);
    query.bindValue(":pwd", updatedData["password"].toString());
    query.bindValue(":id", userId);
    if (!query.exec()) {
        qWarning() << "Не удалось обновить: " << query.lastError().text();
        return false;
    }

    qDebug() << "Успешно.";
    return true;

}

// Категории
bool DatabaseHandler::createCategory(const QString &name, const QByteArray &photo, int inheritsId)
{
    qDebug() << "Запрос на создание категории: название =" << name << ", фото размер =" << photo.size() << ", наследует ID =" << inheritsId;

    QSqlQuery query;
    query.prepare("INSERT INTO categories (category, photo, inherits) VALUES (:category, :photo, :inherits)");
    query.bindValue(":category", name);
    query.bindValue(":photo", photo);
    query.bindValue(":inherits", inheritsId > 0 ? inheritsId : QVariant(QVariant::Int));
    if (!query.exec()) {
        qWarning() << "Не удалось создать категорию:" << query.lastError().text();
        return false;
    }

    qDebug() << "Категория создана успешно.";
    return true;
}

// Категории
bool DatabaseHandler::createCategory(const QString &name, int inheritsId)
{
    qDebug() << "Запрос на создание категории: название =" << name << ", наследует ID =" << inheritsId;

    QSqlQuery query;
    query.prepare("INSERT INTO categories (category, inherits) VALUES (:category, :inherits)");
    query.bindValue(":category", name);
    query.bindValue(":inherits", inheritsId > 0 ? inheritsId : QVariant(QVariant::Int));
    if (!query.exec()) {
        qWarning() << "Не удалось создать категорию:" << query.lastError().text();
        return false;
    }

    qDebug() << "Категория создана успешно.";
    return true;
}

// Удаление категории
bool DatabaseHandler::deleteCategory(int categoryId)
{
    qDebug() << "Запрос на удаление категории с ID:" << categoryId;

    QSqlQuery query;
    query.prepare("DELETE FROM categories WHERE id = :id");
    query.bindValue(":id", categoryId);

    if (!query.exec()) {
        qWarning() << "Не удалось удалить категорию:" << query.lastError().text();
        return false;
    }

    qDebug() << "Категория удалена успешно.";
    return true;
}

QList<QString> DatabaseHandler::getCategoryNames()
{
    qDebug() << "getting category names";
    QSqlQuery query;
    query.prepare("SELECT * FROM categories");
    if (!query.exec())
    {
        qDebug() << "Не получилось получить категории names...";
        return QList<QString>();
    }
    QList<QString> list;
    while (query.next()) {
        QString category;
        category = query.value("category").toString();
        list.append(category);
    }

    return list;
}

QList<QVariantMap> DatabaseHandler::getMainCategories()
{
    qDebug() << "getting category names";
    QSqlQuery query;
    query.prepare("SELECT * FROM categories WHERE inherits is NULL");
    if (!query.exec())
    {
        qDebug() << "Не получилось получить категории names...";
        return QList<QVariantMap>();
    }
    QList<QVariantMap> list;
    while (query.next()) {
        QVariantMap category;
        category["category"] = query.value("category");
        category["photo"] = query.value("photo");
        list.append(category);
    }

    return list;
}


QString DatabaseHandler::getCategoryIdFromName(const QString &name)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM categories WHERE category = :categoryname");
    query.bindValue(":categoryname", name);
    if (!query.exec() || !query.next())
    {
        qDebug() << "Failed получить Id из Name " << name << " категории";
        return QString();
    }
    qDebug() << "Саксес getting ид " << qvariant_cast<int>(query.value("id")) << " from имени " << name << " category";
    return query.value("id").toString();
}

QString DatabaseHandler::getCategoryNameFromId(int id)
{
    QSqlQuery query;
    query.prepare("SELECT category FROM categories WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec() || !query.next())
    {
        qDebug() << "Failed получить Name из id " << id << " категории";
        return QString();
    }
    qDebug() << "Саксес getting имя " << query.value("category").toString() << " from ид " << id << " category";
    return query.value("category").toString();
}

// Создание объявления
bool DatabaseHandler::createAd(const QVariantMap &adData)
{
    qDebug() << "Запрос на создание объявления с данными: "
             << "Дата:" << adData["date"]
             << ", Пользователь:" << adData["user"]
             << ", Категория:" << adData["category"]
             << ", Название:" << adData["name"]
             << ", Описание:" << adData["description"]
             << ", Геолокация:" << adData["geolocation"]
             << ", Цена:" << adData["price"]
             << ", Телефон:" << adData["phone"]
             << ", Фото размер:" << adData["photo"].toByteArray().size()
             << ", Активность:" << adData["active"];

    QSqlQuery query;
    query.prepare("INSERT INTO ads (date, user, category, name, description, geolocation, price, phone, photo, active) "
                  "VALUES (:date, :user, :category, :name, :description, :geolocation, :price, :phone, :photo, :active)");
    query.bindValue(":date", adData["date"]);
    query.bindValue(":user", adData["user"]);
    query.bindValue(":category", adData["category"]);
    query.bindValue(":name", adData["name"]);
    query.bindValue(":description", adData["description"]);
    query.bindValue(":geolocation", adData["geolocation"]);
    query.bindValue(":price", adData["price"]);
    query.bindValue(":phone", adData["phone"]);
    query.bindValue(":photo", adData["photo"]);
    query.bindValue(":active", adData["active"]);

    if (!query.exec()) {
        qWarning() << "Не удалось создать объявление:" << query.lastError().text();
        return false;
    }

    qDebug() << "Объявление создано успешно.";
    return true;
}

bool DatabaseHandler::updateAd(const QVariantMap &ad) {
    QSqlQuery query;
    query.prepare(R"(
        UPDATE ads SET
            category = :category,
            name = :name,
            description = :description,
            geolocation = :geolocation,
            price = :price,
            phone = :phone,
            photo = :photo
        WHERE id = :id
    )");
    query.bindValue(":category", ad["category"]);
    query.bindValue(":name", ad["name"]);
    query.bindValue(":description", ad["description"]);
    query.bindValue(":geolocation", ad["geolocation"]);
    query.bindValue(":price", ad["price"]);
    query.bindValue(":phone", ad["phone"]);
    query.bindValue(":photo", ad["photo"]);
    query.bindValue(":id", ad["id"]);
    return query.exec();
}

// Деактивация объявления
bool DatabaseHandler::deactivateAd(int adId)
{
    qDebug() << "Запрос на деактивацию объявления с ID:" << adId;

    QSqlQuery query;
    query.prepare("UPDATE ads SET active = FALSE WHERE id = :id");
    query.bindValue(":id", adId);

    if (!query.exec()) {
        qWarning() << "Не удалось деактивировать объявление:" << query.lastError().text();
        return false;
    }

    qDebug() << "Объявление деактивировано успешно.";
    return true;
}

bool DatabaseHandler::activateAd(int adId)
{
    qDebug() << "Запрос на активацию объявления с ID:" << adId;

    QSqlQuery query;
    query.prepare("UPDATE ads SET active = TRUE WHERE id = :id");
    query.bindValue(":id", adId);

    if (!query.exec()) {
        qWarning() << "Не удалось активировать объявление:" << query.lastError().text();
        return false;
    }

    qDebug() << "Объявление активировано успешно.";
    return true;
}

bool DatabaseHandler::deleteAd(int adId)
{
    qDebug() << "Запрос на удаление Объявления с ID:" << adId;

    QSqlQuery query;
    query.prepare("DELETE FROM ads WHERE id = :id");
    query.bindValue(":id", adId);

    if (!query.exec()) {
        qWarning() << "Не удалось удалить Объявления:" << query.lastError().text();
        return false;
    }

    qDebug() << "Объявления удалена успешно.";
    return true;
}

// Получение объявлений пользователя
QList<QVariantMap> DatabaseHandler::getUserAds(int userId)
{
    qDebug() << "Запрос на получение объявлений пользователя с ID:" << userId;

    QList<QVariantMap> adsList;
    QSqlQuery query;
    query.prepare("SELECT * FROM ads WHERE user = :userId ORDER BY date DESC");
    query.bindValue(":userId", userId);

    if (query.exec()) {
        while (query.next()) {
            QVariantMap ad;
            ad["id"] = query.value("id");
            ad["date"] = query.value("date");
            ad["user"] = query.value("user");
            ad["category"] = query.value("category");
            ad["name"] = query.value("name");
            ad["description"] = query.value("description");
            ad["geolocation"] = query.value("geolocation");
            ad["price"] = query.value("price");
            ad["phone"] = query.value("phone");
            ad["photo"] = query.value("photo");
            ad["active"] = query.value("active");
            adsList.append(ad);
        }
        qDebug() << "Получено объявлений для пользователя:" << adsList.size();
    } else {
        qWarning() << "Не удалось получить объявления пользователя:" << query.lastError().text();
    }

    return adsList;
}

QList<QVariantMap> DatabaseHandler::searchAds(const QString &text, const QString &location,
                                                   const QString &category, double minPrice, double maxPrice, int searcher_id) {
    QList<QVariantMap> results;

    QSqlQuery query;
    QString queryString = R"(
                          SELECT ads.*, users.name AS username, users.reviews, categories.category AS category_name
                                                    FROM ads
                                                    JOIN categories ON ads.category = categories.id
                                                    JOIN users ON ads.user = users.id
                                                    WHERE (1=1)
                                                    AND (:search IS NULL OR :search = '' OR
                                                        (ads.name LIKE '%' || :search || '%' OR
                                                        description LIKE '%' || :search || '%' OR
                                                        categories.category LIKE '%' || :search || '%'))
                                                    AND (:category IS NULL OR :category = '' OR categories.id = :category)
                                                    AND (:geolocation IS NULL OR :geolocation = '' OR geolocation LIKE '%' || :geolocation || '%')
                                                    AND (:minPrice IS NULL OR price >= :minPrice)
                                                    AND (:maxPrice IS NULL OR price <= :maxPrice)
                                                    AND (active = TRUE)
                                                    AND (user IS NOT :uid);
    )";

    query.prepare(queryString);
    query.bindValue(":search", text.isEmpty() ? QVariant(QVariant::String) : text);
    query.bindValue(":category", category.isEmpty() ? QVariant(QVariant::String) : category);
    query.bindValue(":geolocation", location.isEmpty() ? QVariant(QVariant::String) : location);
    query.bindValue(":minPrice", minPrice >= 0 ? minPrice : QVariant(QVariant::Double));
    query.bindValue(":maxPrice", maxPrice >= 0 ? maxPrice : QVariant(QVariant::Double));
    query.bindValue(":uid", searcher_id);

    if (query.exec()) {
        while (query.next()) {
            QVariantMap ad;
            ad["id"] = query.value("id");
            ad["date"] = query.value("date");
            ad["user"] = query.value("user");
            ad["username"] = query.value("username");
            ad["reviews"] = query.value("reviews");
            ad["category"] = query.value("category");
            ad["category_name"] = query.value("category_name");
            ad["name"] = query.value("name");
            ad["description"] = query.value("description");
            ad["geolocation"] = query.value("geolocation");
            ad["price"] = query.value("price");
            ad["phone"] = query.value("phone");
            ad["photo"] = query.value("photo");
            ad["active"] = query.value("active");
            results.append(ad);
        }
    } else {
        qDebug() << "Ошибка поиска:" << query.lastError().text();
    }

    return results;
}


QVariantMap DatabaseHandler::getAdById(int id) {
    QSqlQuery query;
    query.prepare("SELECT ads.*, users.name AS username, users.profile_picture, users.reviews FROM ads JOIN users ON ads.user = users.id WHERE (ads.active = TRUE AND ads.id = :id)");
    query.bindValue(":id", id);
    query.exec();

    QVariantMap ad;
    if (query.next()) {
        ad["id"] = query.value("id");
        ad["date"] = query.value("date");
        ad["user"] = query.value("user");
        ad["category"] = query.value("category");
        ad["name"] = query.value("name");
        ad["description"] = query.value("description");
        ad["geolocation"] = query.value("geolocation");
        ad["price"] = query.value("price");
        ad["phone"] = query.value("phone");
        ad["photo"] = query.value("photo");
        ad["active"] = query.value("active");
        ad["username"] = query.value("username");
        ad["profile_picture"] = query.value("profile_picture");
        ad["reviews"] = query.value("reviews");
    }
    return ad;
}


// Добавление в избранное
bool DatabaseHandler::addFavourite(int userId, int adId)
{
    qDebug() << "Запрос на добавление в избранное: пользователь ID = " << userId << ", объявление ID = " << adId;

    QSqlQuery query;
    query.prepare("INSERT OR IGNORE INTO favourite (user, ad) VALUES (:user, :ad)");
    query.bindValue(":user", userId);
    query.bindValue(":ad", adId);

    if (!query.exec()) {
        qWarning() << "Не удалось добавить в избранное:" << query.lastError().text();
        return false;
    }

    qDebug() << "Объявление добавлено в избранное.";
    return true;
}

// Удаление из избранного
bool DatabaseHandler::removeFavourite(int userId, int adId)
{
    qDebug() << "Запрос на удаление из избранного: пользователь ID = " << userId << ", объявление ID = " << adId;

    QSqlQuery query;
    query.prepare("DELETE FROM favourite WHERE user = :user AND ad = :ad");
    query.bindValue(":user", userId);
    query.bindValue(":ad", adId);

    if (!query.exec()) {
        qWarning() << "Не удалось удалить из избранного:" << query.lastError().text();
        return false;
    }

    qDebug() << "Объявление удалено из избранного.";
    return true;
}

// Получение избранных объявлений
QList<QVariantMap> DatabaseHandler::getUserFavourites(int userId)
{
    qDebug() << "Запрос на получение избранных объявлений пользователя с ID:" << userId;

    QList<QVariantMap> favouritesList;
    QSqlQuery query;
    query.prepare("SELECT ads.* FROM ads INNER JOIN favourite ON ads.id = favourite.ad WHERE favourite.user = :userId AND ads.active = TRUE");
    query.bindValue(":userId", userId);

    if (query.exec()) {
        while (query.next()) {
            QVariantMap ad;
            ad["id"] = query.value("id");
            ad["date"] = query.value("date");
            ad["user"] = query.value("user");
            ad["category"] = query.value("category");
            ad["name"] = query.value("name");
            ad["description"] = query.value("description");
            ad["geolocation"] = query.value("geolocation");
            ad["price"] = query.value("price");
            ad["phone"] = query.value("phone");
            ad["photo"] = query.value("photo");
            ad["active"] = query.value("active");
            favouritesList.append(ad);
        }
        qDebug() << "Получено избранных объявлений для пользователя:" << favouritesList.size();
    } else {
        qWarning() << "Не удалось получить избранные объявления:" << query.lastError().text();
    }

    return favouritesList;
}

QList<QVariantMap> DatabaseHandler::getIncomingDialogs(int userId) {
    QList<QVariantMap> results;
    QSqlQuery query;
    query.prepare("SELECT d.*, a.name AS post_name, u.name AS other_user_name, u.profile_picture AS other_user_profile_picture, "
                  "a.photo AS post_photo, u.id AS other_user_id "
                  "FROM dialogs d "
                  "JOIN ads a ON d.ad_id = a.id "
                  "JOIN users u ON u.id = d.user1_id "
                  "WHERE d.user2_id = :userId "
                  "ORDER BY d.last_message_date DESC");
    query.bindValue(":userId", userId);
    if (query.exec()) {
        while (query.next()) {
            QVariantMap dialog;
            dialog["id"] = query.value("id");
            dialog["ad_id"] = query.value("ad_id");
            dialog["user1_id"] = query.value("user1_id");
            dialog["user2_id"] = query.value("user2_id");
            dialog["post_name"] = query.value("post_name");
            dialog["post_photo"] = query.value("post_photo");
            dialog["other_user_name"] = query.value("other_user_name");
            dialog["last_message"] = query.value("last_message");
            dialog["last_message_date"] = query.value("last_message_date");
            dialog["other_user_id"] = query.value("other_user_id");
            dialog["other_user_profile_picture"] = query.value("other_user_profile_picture");
            results.append(dialog);
        }
    } else {
        qDebug() << "Ошибка при получении входящих диалогов: " << query.lastError();
    }
    return results;
}


QList<QVariantMap> DatabaseHandler::getOutgoingDialogs(int userId) {
    QList<QVariantMap> results;
    QSqlQuery query;
    qDebug() << "Получаем исход. диалоги от юзера " << userId;
    query.prepare("SELECT d.*, a.name AS post_name, u.name AS other_user_name, u.profile_picture AS other_user_profile_picture, "
                  "a.photo AS post_photo, u.id AS other_user_id "
                  "FROM dialogs d "
                  "JOIN ads a ON d.ad_id = a.id "
                  "JOIN users u ON u.id = d.user2_id "
                  "WHERE d.user1_id = :userId "
                  "ORDER BY d.last_message_date DESC");
    query.bindValue(":userId", userId);
    if (query.exec()) {
        while (query.next()) {
            QVariantMap dialog;
            dialog["id"] = query.value("id");
            dialog["ad_id"] = query.value("ad_id");
            dialog["user1_id"] = query.value("user1_id");
            dialog["user2_id"] = query.value("user2_id");
            dialog["post_name"] = query.value("post_name");
            dialog["post_photo"] = query.value("post_photo");
            dialog["other_user_name"] = query.value("other_user_name");
            dialog["last_message"] = query.value("last_message");
            dialog["last_message_date"] = query.value("last_message_date");
            dialog["other_user_id"] = query.value("other_user_id");
            dialog["other_user_profile_picture"] = query.value("other_user_profile_picture");
            results.append(dialog);
        }
    } else {
        qDebug() << "Ошибка при получении исходящих диалогов: " << query.lastError();
    }
    return results;
}


int DatabaseHandler::createOrGetDialog(int adId, int user1Id, int user2Id) {
    QSqlQuery query;
    query.prepare("SELECT id FROM dialogs "
                  "WHERE ad_id = :adId AND ((user1_id = :user1Id AND user2_id = :user2Id) "
                  "OR (user1_id = :user2Id AND user2_id = :user1Id))");
    query.bindValue(":adId", adId);
    query.bindValue(":user1Id", user1Id);
    query.bindValue(":user2Id", user2Id);

    if (query.exec() && query.next()) {
        return query.value("id").toInt(); // Диалог уже существует
    }

    // Если диалога нет, создаем его
    query.prepare("INSERT INTO dialogs (ad_id, user1_id, user2_id) "
                  "VALUES (:adId, :user1Id, :user2Id)");
    query.bindValue(":adId", adId);
    query.bindValue(":user1Id", user1Id);
    query.bindValue(":user2Id", user2Id);
    if (!query.exec()) {
        qDebug() << "Ошибка создания диалога: " << query.lastError();
        return -1;
    }
    return query.lastInsertId().toInt();
}


QList<QVariantMap> DatabaseHandler::getMessages(int dialogId) {
    QList<QVariantMap> results;
    QSqlQuery query;
    query.prepare("SELECT m.id, m.sender_id, u.name AS sender_name, u.profile_picture, m.text, m.sent_date "
                  "FROM messages m "
                  "JOIN users u ON m.sender_id = u.id "
                  "WHERE m.dialog_id = :dialogId "
                  "ORDER BY m.sent_date ASC");
    query.bindValue(":dialogId", dialogId);
    if (query.exec()) {
        while (query.next()) {
            QVariantMap message;
            message["id"] = query.value("id");
            message["sender_id"] = query.value("sender_id");
            message["sender_name"] = query.value("sender_name");
            message["text"] = query.value("text");
            message["sent_date"] = query.value("sent_date");
            message["profile_picture"] = query.value("profile_picture");
            results.append(message);
        }
    } else {
        qDebug() << "Ошибка при получении сообщений: " << query.lastError();
    }
    return results;
}



bool DatabaseHandler::sendMessage(int dialogId, int senderId, const QString &text) {
    QSqlQuery query;

    // Добавляем сообщение
    query.prepare("INSERT INTO messages (dialog_id, sender_id, text) "
                  "VALUES (:dialogId, :senderId, :text)");
    query.bindValue(":dialogId", dialogId);
    query.bindValue(":senderId", senderId);
    query.bindValue(":text", text);

    if (!query.exec()) {
        qDebug() << "Ошибка при отправке сообщения: " << query.lastError();
        return false;
    }

    // Обновляем последний текст и дату в таблице диалогов
    query.prepare("UPDATE dialogs SET last_message = :lastMessage, last_message_date = CURRENT_TIMESTAMP "
                  "WHERE id = :dialogId");
    query.bindValue(":lastMessage", text);
    query.bindValue(":dialogId", dialogId);

    if (!query.exec()) {
        qDebug() << "Ошибка обновления последнего сообщения в диалоге: " << query.lastError();
        return false;
    }

    return true;
}



// Добавление отзыва
bool DatabaseHandler::addReview(int reviewerId, int reviewedId, int rating, const QString &reviewText)
{
    qDebug() << "Запрос на добавление отзыва от пользователя с ID:" << reviewerId << "о пользователе с ID:" << reviewedId
             << ", рейтинг:" << rating << ", текст отзыва:" << reviewText;

    QSqlQuery query;
    query.prepare("INSERT INTO reviews (reviewer, reviewed, rating, review_text) VALUES (:reviewer, :reviewed, :rating, :reviewText)");
    query.bindValue(":reviewer", reviewerId);
    query.bindValue(":reviewed", reviewedId);
    query.bindValue(":rating", rating);
    query.bindValue(":reviewText", reviewText);

    if (!query.exec()) {
        qWarning() << "Не удалось добавить отзыв:" << query.lastError().text();
        return false;
    }

    updateUserRating(reviewedId);
    qDebug() << "Отзыв добавлен успешно.";
    return true;
}

bool DatabaseHandler::deleteReview(int reviewId) {
    qDebug() << "Удаляем ревью по ид " << reviewId;
    QSqlQuery query;
    query.prepare("SELECT reviews.reviewed FROM reviews WHERE id = :review_id");
    query.bindValue(":review_id", reviewId);

    if (!query.exec()) {
        return false;
    }
    int reviewed = qvariant_cast<int>(query.value("reviewed"));

    query.prepare("DELETE FROM reviews WHERE id = :review_id");
    query.bindValue(":review_id", reviewId);

    if (query.exec()) {
        updateUserRating(reviewed);
        return true;
    }
    return false;
}

void DatabaseHandler::updateUserRating(int userId) {
    QSqlQuery query;
    qDebug() << "Обновляем Userrating для " << userId;
    query.prepare("UPDATE users SET reviews = (SELECT AVG(rating) FROM reviews WHERE reviewed = :user_id) "
                  "WHERE id = :user_id");
    query.bindValue(":user_id", userId);
    if (!query.exec()) qDebug() << "Неуспешно.";
}

void DatabaseHandler::updateUserRatings() {
    qDebug() << "Обновляем Userrating для всех пользователей";

    QSqlQuery query;

    // Получаем всех пользователей
    QList<int> userIds;
    query.prepare("SELECT id FROM users");
    if (!query.exec()) {
        qDebug() << "Не удалось получить пользователей: " << query.lastError().text();
        return;
    }

    while (query.next()) {
        userIds.append(query.value(0).toInt());
    }

    // Обновляем рейтинг для каждого пользователя
    for (int userId : userIds) {
        QSqlQuery updateQuery;
        updateQuery.prepare(
            "UPDATE users SET reviews = (SELECT AVG(rating) FROM reviews WHERE reviewed = :user_id) "
            "WHERE id = :user_id");
        updateQuery.bindValue(":user_id", userId);

        if (!updateQuery.exec()) {
            qDebug() << "Не удалось обновить рейтинг для пользователя с ID" << userId
                     << ": " << updateQuery.lastError().text();
        } else {
            qDebug() << "Рейтинг успешно обновлен для пользователя с ID" << userId;
        }
    }
}



// Получение отзывов пользователя
QList<QVariantMap> DatabaseHandler::getUserReviews(int userId)
{
    qDebug() << "Запрос на получение отзывов для пользователя с ID:" << userId;

    QList<QVariantMap> reviewsList;
    QSqlQuery query;
    query.prepare("SELECT reviews.*, users.name AS reviewer_name, users.profile_picture AS reviewer_pfp FROM reviews JOIN users ON reviews.reviewer = users.id WHERE reviewed = :userId ORDER BY date DESC;");
    query.bindValue(":userId", userId);

    if (query.exec()) {
        while (query.next()) {
            QVariantMap review;
            review["id"] = query.value("id");
            review["reviewer"] = query.value("reviewer");
            review["reviewed"] = query.value("reviewed");
            review["rating"] = query.value("rating");
            review["review_text"] = query.value("review_text");
            review["date"] = query.value("date");
            review["reviewer_name"] = query.value("reviewer_name");
            review["reviewer_pfp"] = query.value("reviewer_pfp");
            reviewsList.append(review);
        }
        qDebug() << "Получено отзывов для пользователя:" << reviewsList.size();
    } else {
        qWarning() << "Не удалось получить отзывы:" << query.lastError().text();
    }

    return reviewsList;
}

bool DatabaseHandler::addComplaint(int targetId, bool isAd, const QString &reason, const QString &explanation) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO complaints (target_id, target_ad, author_id, reason, explanation) "
                  "VALUES (:target_id, :target_ad, :author_id, :reason, :explanation)");
    query.bindValue(":target_id", targetId);
    query.bindValue(":target_ad", isAd ? 1 : 0);
    query.bindValue(":author_id", AppContext::instance()->getCurrentUser()["id"].toInt());
    query.bindValue(":reason", reason);
    query.bindValue(":explanation", explanation);

    if (!query.exec()) {
        qWarning() << "Ошибка добавления жалобы:" << query.lastError();
        return false;
    }
    return true;
}

QList<QVariantMap> DatabaseHandler::getComplaints() {
    QList<QVariantMap> complaints;
    QSqlQuery query(db);
    query.prepare("SELECT * FROM complaints");

    if (!query.exec()) {
        qWarning() << "Ошибка загрузки жалоб:" << query.lastError();
        return complaints;
    }

    while (query.next()) {
        QVariantMap complaint;
        complaint["id"] = query.value("id");
        complaint["target_id"] = query.value("target_id");
        complaint["target_ad"] = query.value("target_ad").toBool();
        complaint["author_id"] = query.value("author_id");
        complaint["reason"] = query.value("reason");
        complaint["explanation"] = query.value("explanation");
        complaints.append(complaint);
    }
    return complaints;
}

bool DatabaseHandler::resolveComplaint(int complaintId) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM complaints WHERE id = :id");
    query.bindValue(":id", complaintId);

    if (!query.exec()) {
        qWarning() << "Ошибка удаления жалобы:" << query.lastError();
        return false;
    }
    return true;
}

