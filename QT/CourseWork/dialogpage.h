#ifndef DIALOGPAGE_H
#define DIALOGPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include "databasehandler.h"


class DialogPage : public QWidget {
    Q_OBJECT

public:
    explicit DialogPage(int currentDialogId, QWidget *parent = nullptr);
    void loadDialog(int dialogId);

signals:
    void goToUserPage(int userId);
    void goToPostPage(int postId);

private slots:
    void sendMessage();

private:
    QListWidget *messagesListWidget;
    QLineEdit *messageInput;
    QPushButton *sendButton;

    int currentDialogId;
};

#endif // DIALOGPAGE_H
