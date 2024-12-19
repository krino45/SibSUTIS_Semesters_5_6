#ifndef DIALOGSPAGE_H
#define DIALOGSPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QPushButton>
#include <QDateTime>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>

#include "databasehandler.h"
#include "appcontext.h"
#include "clickablelabel.h"
#include "listingcard.h"

class DialogsPage : public QWidget {
    Q_OBJECT

public:
    explicit DialogsPage(QWidget *parent = nullptr);
    void loadDialogs();

signals:
    void goToPostPage(int postId);
    void goToUserPage(int userId);
    void goToDialogPage(int dialogId, int user1, int user2);

private:
    QListWidget *incomingDialogsListWidget;
    QListWidget *outgoingDialogsListWidget;

    void addDialogCard(const QVariantMap &dialog, QListWidget *listWidget);
};

#endif // DIALOGSPAGE_H
