#include "addreviewdialog.h"
#include "databasehandler.h"


AddReviewDialog::AddReviewDialog(int authorId, int targetUserId, QWidget *parent)
    : QDialog(parent), authorId(authorId), targetUserId(targetUserId) {
    setWindowTitle("Добавить отзыв");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *ratingLabel = new QLabel("Оценка (1–5):", this);
    mainLayout->addWidget(ratingLabel);

    ratingComboBox = new QComboBox(this);
    for (int i = 1; i <= 5; ++i) {
        ratingComboBox->addItem(QString::number(i));
    }
    mainLayout->addWidget(ratingComboBox);

    QLabel *textLabel = new QLabel("Текст отзыва:", this);
    mainLayout->addWidget(textLabel);

    reviewTextEdit = new QTextEdit(this);
    mainLayout->addWidget(reviewTextEdit);

    QPushButton *submitButton = new QPushButton("Отправить", this);
    connect(submitButton, &QPushButton::clicked, this, &AddReviewDialog::submitReview);
    mainLayout->addWidget(submitButton);
}

void AddReviewDialog::submitReview() {
    int rating = ratingComboBox->currentText().toInt();
    QString text = reviewTextEdit->toPlainText();

    if (text.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Текст отзыва не может быть пустым.");
        return;
    }

    if (DatabaseHandler::instance()->addReview(authorId, targetUserId, rating, text)) {
        QMessageBox::information(this, "Успех", "Ваш отзыв успешно добавлен.");
        accept();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить отзыв.");
    }
}
