    #include "dialogspage.h"

    DialogsPage::DialogsPage(QWidget *parent)
        : QWidget(parent),
          incomingDialogsListWidget(new QListWidget(this)),
          outgoingDialogsListWidget(new QListWidget(this)) {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        QLabel *incomingLabel = new QLabel("Входящие диалоги:", this);
        mainLayout->addWidget(incomingLabel);
        mainLayout->addWidget(incomingDialogsListWidget);

        QLabel *outgoingLabel = new QLabel("Исходящие диалоги:", this);
        mainLayout->addWidget(outgoingLabel);
        mainLayout->addWidget(outgoingDialogsListWidget);

        loadDialogs();
        setLayout(mainLayout);
    }

    void DialogsPage::loadDialogs() {
        incomingDialogsListWidget->clear();
        outgoingDialogsListWidget->clear();

        int currentUserId = qvariant_cast<int>(AppContext::instance()->getCurrentUser()["id"]);

        QList<QVariantMap> incomingDialogs = DatabaseHandler::instance()->getIncomingDialogs(currentUserId);
        QList<QVariantMap> outgoingDialogs = DatabaseHandler::instance()->getOutgoingDialogs(currentUserId);

        for (const QVariantMap &dialog : incomingDialogs) {
            addDialogCard(dialog, incomingDialogsListWidget);
        }

        for (const QVariantMap &dialog : outgoingDialogs) {
            qDebug() << "Подгрузка исход. диалога...";
            addDialogCard(dialog, outgoingDialogsListWidget);
        }
    }

    void DialogsPage::addDialogCard(const QVariantMap &dialog, QListWidget *listWidget) {
        QListWidgetItem *item = new QListWidgetItem(listWidget);

        ListingCard *dialogCard = new ListingCard(dialog["id"].toInt(), this);
        dialogCard->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);

        QHBoxLayout *cardLayout = new QHBoxLayout(dialogCard);

        // Изображение поста
        QLabel *postImage = new QLabel(dialogCard);
        QByteArray photoData = dialog["post_photo"].toByteArray();
        if (!photoData.isEmpty()) {
            QPixmap pixmap;
            pixmap.loadFromData(photoData);
            postImage->setPixmap(pixmap.scaled(80, 80, Qt::KeepAspectRatio));
        } else {
            postImage->setText("Нет фото");
            postImage->setAlignment(Qt::AlignCenter);
        }
        cardLayout->addWidget(postImage);

        // Информация
        QVBoxLayout *infoLayout = new QVBoxLayout();
        QLabel *postName = new QLabel(dialog["post_name"].toString(), dialogCard);
        QLabel *otherUser = new QLabel("Собеседник: " + dialog["other_user_name"].toString(), dialogCard);
        QLabel *lastMessage = new QLabel(dialog["last_message"].toString(), dialogCard);
        QLabel *lastMessageDate = new QLabel(dialog["last_message_date"].toDateTime().toString("dd.MM.yyyy HH:mm"), dialogCard);

        infoLayout->addWidget(postName);
        infoLayout->addWidget(otherUser);
        infoLayout->addWidget(lastMessage);
        infoLayout->addWidget(lastMessageDate);
        cardLayout->addLayout(infoLayout);

        // Добавляем карточку в ListWidget
        item->setSizeHint(dialogCard->sizeHint());
        listWidget->setItemWidget(item, dialogCard);

        // Обработка кликов на карточке
        connect(dialogCard, &ListingCard::clicked, this, [this, dialog]() {
            emit goToDialogPage(dialog["ad_id"].toInt(), dialog["user1_id"].toInt(), dialog["user2_id"].toInt());
        });

        // Обработка кликов на картинке поста
        connect(postImage, &QLabel::linkActivated, this, [this, dialog]() {
            emit goToPostPage(dialog["ad_id"].toInt());
        });

        // Обработка кликов на имени пользователя
        connect(otherUser, &QLabel::linkActivated, this, [this, dialog]() {
            emit goToUserPage(dialog["other_user_id"].toInt());
        });
    }
