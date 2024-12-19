#include "adminpage.h"

AdminPage::AdminPage(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    tabs = new QTabWidget(this);

    // Добавление вкладок
    categoryManager = new CategoryManager(this);
    tabs->addTab(categoryManager, "Категории");

    complaintsPage = new ComplaintsPage(this);
    tabs->addTab(complaintsPage, "Жалобы");

    userControlPage = new UserControlPage(this);
    tabs->addTab(userControlPage, "Пользователи");

    mainLayout->addWidget(tabs);
    setLayout(mainLayout);
}
