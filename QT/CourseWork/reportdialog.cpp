#include "reportdialog.h"
#include "databasehandler.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QMessageBox>

ReportDialog::ReportDialog(int targetId, bool isAd, QWidget *parent)
    : QDialog(parent), targetId(targetId), isAd(isAd) {
    setWindowTitle("Пожаловаться");
    setMinimumSize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *reasonLabel = new QLabel("Причина:", this);
    layout->addWidget(reasonLabel);

    reasonComboBox = new QComboBox(this);
    reasonComboBox->addItems({
        "Неверная категория",
        "Неполная информация",
        "Недопустимый контент",
        "Спам или мошенничество",
        "Нарушение правил платформы",
        "Фейковое объявление",
        "Оскорбительный контент",
        "Неактуальная информация",
        "Попытка обмана",
        "Контактные данные недействительны",
        "Не соответствует реальности",
        "Неадекватное поведение пользователя",
        "Несоответствие заявленной цене",
        "Неэтичное содержание",
        "Подозрительная активность"
    });
    layout->addWidget(reasonComboBox);


    QLabel *explanationLabel = new QLabel("Подробности (опционально):", this);
    layout->addWidget(explanationLabel);

    explanationEdit = new QTextEdit(this);
    layout->addWidget(explanationEdit);

    QPushButton *submitButton = new QPushButton("Отправить", this);
    layout->addWidget(submitButton);
    connect(submitButton, &QPushButton::clicked, this, &ReportDialog::submitReport);
}

void ReportDialog::submitReport() {
    QString reason = reasonComboBox->currentText();
    QString explanation = explanationEdit->toPlainText().trimmed();

    if (reason.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Укажите причину жалобы.");
        return;
    }

    // Добавляем жалобу в базу данных
    if (DatabaseHandler::instance()->addComplaint(targetId, isAd, reason, explanation)) {
        QMessageBox::information(this, "Успех", "Жалоба отправлена.");
        emit reportSubmitted();
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось отправить жалобу.");
    }
}
