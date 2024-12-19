#include "messagespage.h"

MessagesPage::MessagesPage(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *label = new QLabel("Messages Page Placeholder", this);
    layout->addWidget(label);
    setLayout(layout);
}
