#ifndef COMPLAINTSPAGE_H
#define COMPLAINTSPAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>

class ComplaintsPage : public QWidget {
    Q_OBJECT

public:
    explicit ComplaintsPage(QWidget *parent = nullptr);

private:
    QTableWidget *complaintsTable;
    QPushButton *resolveButton;

    void loadComplaints();
    void resolveComplaint();

signals:
    void complaintsUpdated();
};

#endif // COMPLAINTSPAGE_H
