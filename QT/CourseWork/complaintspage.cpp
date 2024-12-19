#include "complaintspage.h"
#include "databasehandler.h"

ComplaintsPage::ComplaintsPage(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    complaintsTable = new QTableWidget(this);
    complaintsTable->setColumnCount(4);
    complaintsTable->setHorizontalHeaderLabels({"ID", "Автор", "Объект", "Причина"});
    mainLayout->addWidget(complaintsTable);

    resolveButton = new QPushButton("Решить жалобу", this);
    connect(resolveButton, &QPushButton::clicked, this, &ComplaintsPage::resolveComplaint);
    mainLayout->addWidget(resolveButton);

    loadComplaints();
    setLayout(mainLayout);
}

void ComplaintsPage::loadComplaints() {
    complaintsTable->setRowCount(0);
    auto complaints = DatabaseHandler::instance()->getComplaints();
    for (const auto &complaint : complaints) {
        int row = complaintsTable->rowCount();
        complaintsTable->insertRow(row);

        complaintsTable->setItem(row, 0, new QTableWidgetItem(QString::number(complaint["id"].toInt())));
        complaintsTable->setItem(row, 1, new QTableWidgetItem("id: " + complaint["author_id"].toString()));
        complaintsTable->setItem(row, 2, new QTableWidgetItem((complaint["target_ad"].toString() == "1" ? "ad " : "user ") + complaint["target_id"].toString()));
        complaintsTable->setItem(row, 3, new QTableWidgetItem(complaint["reason"].toString()));
    }
}

void ComplaintsPage::resolveComplaint() {
    int row = complaintsTable->currentRow();
    if (row >= 0) {
        int complaintId = complaintsTable->item(row, 0)->text().toInt();
        if (DatabaseHandler::instance()->resolveComplaint(complaintId)) {
            complaintsTable->removeRow(row);
        }
    }
}
