#include "dialogpage.h"
#include "appcontext.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDateTime>
#include <QListWidgetItem>

DialogPage::DialogPage(int currentDialogId, QWidget *parent)
    : QWidget(parent), currentDialogId(currentDialogId) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    messagesListWidget = new QListWidget(this);
    mainLayout->addWidget(messagesListWidget);

    QHBoxLayout *inputLayout = new QHBoxLayout();
    messageInput = new QLineEdit(this);
    sendButton = new QPushButton("Отправить", this);

    inputLayout->addWidget(messageInput);
    inputLayout->addWidget(sendButton);

    mainLayout->addLayout(inputLayout);

    loadDialog(currentDialogId);

    connect(sendButton, &QPushButton::clicked, this, &DialogPage::sendMessage);
}

void DialogPage::loadDialog(int dialogId) {
    messagesListWidget->clear();
    currentDialogId = dialogId;

    QList<QVariantMap> messages = DatabaseHandler::instance()->getMessages(dialogId);
    for (const QVariantMap &message : messages) {
        QListWidgetItem *item = new QListWidgetItem(messagesListWidget);
        QWidget *messageWidget = new QWidget();
        QHBoxLayout *messageLayout = new QHBoxLayout(messageWidget);

        QLabel *sender = new QLabel(message["sender_name"].toString());
        QLabel *text = new QLabel(message["text"].toString());
        QLabel *date = new QLabel(message["sent_date"].toDateTime().toString("dd.MM.yyyy HH:mm"));

        messageLayout->addWidget(sender);
        messageLayout->addWidget(text);
        messageLayout->addWidget(date);
        messageWidget->setLayout(messageLayout);

        item->setSizeHint(messageWidget->sizeHint());
        messagesListWidget->setItemWidget(item, messageWidget);
    }
}

void DialogPage::sendMessage() {
    if (currentDialogId == -1 || messageInput->text().isEmpty()) return;

    int senderId = qvariant_cast<int>(AppContext::instance()->getCurrentUser()["id"]);
    QString messageText = messageInput->text();

    DatabaseHandler::instance()->sendMessage(currentDialogId, senderId, messageText);
    loadDialog(currentDialogId);
    messageInput->clear();
}
