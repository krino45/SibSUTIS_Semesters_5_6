#include "usercontrolpage.h"
#include "databasehandler.h"

UserControlPage::UserControlPage(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    usersTable = new QTableWidget(this);
    usersTable->setColumnCount(5);
    usersTable->setHorizontalHeaderLabels({"ID", "Имя", "Почта", "Оценка", "О себе", "Роль"});
    mainLayout->addWidget(usersTable);

    deleteUserButton = new QPushButton("Удалить пользователя", this);
    connect(deleteUserButton, &QPushButton::clicked, this, &UserControlPage::deleteUser);
    mainLayout->addWidget(deleteUserButton);

    loadUsers();
    setLayout(mainLayout);
}

void UserControlPage::loadUsers() {
    usersTable->setRowCount(0);
    auto users = DatabaseHandler::instance()->getAllUsers();
    for (const auto &user : users) {

        int row = usersTable->rowCount();
        usersTable->insertRow(row);

        usersTable->setItem(row, 0, new QTableWidgetItem(QString::number(user["id"].toInt())));
        usersTable->setItem(row, 1, new QTableWidgetItem(user["name"].toString()));
        usersTable->setItem(row, 2, new QTableWidgetItem(user["email"].toString()));
        usersTable->setItem(row, 3, new QTableWidgetItem(user["reviews"].toString()));
        usersTable->setItem(row, 4, new QTableWidgetItem(user["about_me"].toString()));
        usersTable->setItem(row, 5, new QTableWidgetItem(user["role"].toString()));
    }
}

void UserControlPage::deleteUser() {
    int row = usersTable->currentRow();
    if (row >= 0) {
        int userId = usersTable->item(row, 0)->text().toInt();
        if (DatabaseHandler::instance()->deleteUser(userId)) {
            usersTable->removeRow(row);
        }
    }
}
