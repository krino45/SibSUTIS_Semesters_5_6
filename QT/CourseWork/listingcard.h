#ifndef LISTINGCARD_H
#define LISTINGCARD_H

#include <QFrame>

class ListingCard : public QFrame {
    Q_OBJECT

public:
    explicit ListingCard(int productId, QWidget *parent = nullptr);

signals:
    void clicked(int productId);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    int productId;
};

#endif // LISTINGCARD_H
