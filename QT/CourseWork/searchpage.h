#ifndef SEARCHPAGE_H
#define SEARCHPAGE_H

#include <QWidget>
#include <QDateTime>
#include "databasehandler.h"
#include "appcontext.h"
#include "listingcard.h"
#include "logindialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QListWidget>
#include <QCompleter>
#include <QDebug>

class SearchPage : public QWidget
{
    Q_OBJECT
public:
    explicit SearchPage(QWidget *parent = nullptr);
    explicit SearchPage(QString text_param, QString category_param, QWidget *parent = nullptr);

signals:
    void goToProductPage(int adId);
    void goToDialog(int productId, int user1, int user2);
    void plshelpme();

public slots:

private:
    void initialize();
    void performSearch(const QString &category = QString(), const QString &priceFrom = QString(), const QString &priceTo = QString());
    QLineEdit *searchLineEdit;
    QLineEdit *locationLineEdit;
    QLabel *resultCountLabel;
    QListWidget *resultsListWidget;
    QVBoxLayout *resultsLayout;
    QComboBox *categoryDropdown;
};

#endif // SEARCHPAGE_H
