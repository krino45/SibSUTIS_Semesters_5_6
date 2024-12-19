#ifndef USERCONTROLPAGE_H
#define USERCONTROLPAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>

class UserControlPage : public QWidget {
    Q_OBJECT

public:
    explicit UserControlPage(QWidget *parent = nullptr);

private:
    QTableWidget *usersTable;
    QPushButton *deleteUserButton;

    void loadUsers();
    void deleteUser();

signals:
    void usersUpdated();
};

#endif // USERCONTROLPAGE_H
