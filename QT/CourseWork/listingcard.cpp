#include "listingcard.h"
#include <QMouseEvent>

ListingCard::ListingCard(int productId, QWidget *parent) : QFrame(parent), productId(productId) {
    setCursor(Qt::PointingHandCursor);
}

void ListingCard::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(productId);
    }
    QFrame::mousePressEvent(event);
}
