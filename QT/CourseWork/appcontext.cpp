#include "appcontext.h"

void AppContext::setUserFavorites(const QList<int> &favorites) {
    userFavorites = favorites;
}

const QList<int> &AppContext::getUserFavorites() const {
    return userFavorites;
}

void AppContext::addToFavorites(int adId) {
    if (!userFavorites.contains(adId)) userFavorites.append(adId);
}

void AppContext::removeFromFavorites(int adId) {
    userFavorites.removeAll(adId);
}

bool AppContext::isFavorite(int adId) const {
    return userFavorites.contains(adId);
}
