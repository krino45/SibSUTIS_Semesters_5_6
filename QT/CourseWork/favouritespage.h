#ifndef FAVOURITESPAGE_H
#define FAVOURITESPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>

class FavouritesPage : public QWidget
{
    Q_OBJECT
public:
    explicit FavouritesPage(QWidget *parent = nullptr);

signals:
    void goToProductPage(int ad_id);
    void goToDialog(int ad_id, int u1, int u2);

public slots:

private:
    QListWidget *favoritesListWidget;

    void updateFavorites();
};

#endif // FAVOURITESPAGE_H
