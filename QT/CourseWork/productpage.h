#ifndef PRODUCTPAGE_H
#define PRODUCTPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

class ProductPage : public QWidget {
    Q_OBJECT

public:
    explicit ProductPage(int productId, QWidget *parent = nullptr);

signals:
    void goToUserPage(int uid);
    void goToDialog(int productId, int u1, int u2);
    void goMain();

private:
    void loadProductData(int productId);
    void setUpProductPage(int productId);
};

#endif // PRODUCTPAGE_H
