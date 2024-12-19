#ifndef ADMINPAGE_H
#define ADMINPAGE_H

#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include "categorymanager.h"
#include "complaintspage.h"
#include "usercontrolpage.h"

class AdminPage : public QWidget {
    Q_OBJECT

public:
    explicit AdminPage(QWidget *parent = nullptr);

private:
    QTabWidget *tabs;

    CategoryManager *categoryManager;
    ComplaintsPage *complaintsPage;
    UserControlPage *userControlPage;
};

#endif // ADMINPAGE_H
