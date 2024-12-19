#include "categorymanager.h"
#include "databasehandler.h"
#include <QFileDialog>
#include <QLabel>
#include <QPixmap>
#include <QHBoxLayout>
#include <QBuffer>
#include <QImageReader>

CategoryManager::CategoryManager(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    categoriesList = new QListWidget(this);
    mainLayout->addWidget(categoriesList);

    categoryInput = new QLineEdit(this);
    categoryInput->setPlaceholderText("Новая категория");
    mainLayout->addWidget(categoryInput);

    // Кнопка для выбора изображения
    selectImageButton = new QPushButton("Выбрать изображение", this);
    imagePreviewLabel = new QLabel("Нет изображения", this); // Для предварительного просмотра

    QHBoxLayout *imageLayout = new QHBoxLayout();
    imageLayout->addWidget(selectImageButton);
    imageLayout->addWidget(imagePreviewLabel);

    mainLayout->addLayout(imageLayout);

    addButton = new QPushButton("Добавить", this);
    deleteButton = new QPushButton("Удалить", this);

    connect(addButton, &QPushButton::clicked, this, &CategoryManager::addCategory);
    connect(deleteButton, &QPushButton::clicked, this, &CategoryManager::deleteCategory);
    connect(selectImageButton, &QPushButton::clicked, this, &CategoryManager::selectImage);

    mainLayout->addWidget(addButton);
    mainLayout->addWidget(deleteButton);

    loadCategories();
    setLayout(mainLayout);
}

void CategoryManager::loadCategories() {
    categoriesList->clear();
    auto categories = DatabaseHandler::instance()->getCategoryNames();
    for (const QString &category : categories) {
        categoriesList->addItem(category);
    }
}

void CategoryManager::addCategory() {
    QString newCategory = categoryInput->text().trimmed();
    if (!newCategory.isEmpty() && !imageData.isEmpty()) {
        // Сохраняем изображение как бинарные данные (QByteArray)
        if (DatabaseHandler::instance()->createCategory(newCategory, imageData)) {
            loadCategories();
            categoryInput->clear();
            imagePreviewLabel->setText("Нет изображения");  // Сбросить текст
            imageData.clear(); // Очистить данные изображения
        }
    }
}

void CategoryManager::deleteCategory() {
    QListWidgetItem *selectedItem = categoriesList->currentItem();
    if (selectedItem) {
        int categoryId = DatabaseHandler::instance()->getCategoryIdFromName(selectedItem->text()).toInt();
        if (DatabaseHandler::instance()->deleteCategory(categoryId)) {
            delete selectedItem;
        }
    }
}

void CategoryManager::selectImage() {
    // Открытие диалога выбора файла
    QString filePath = QFileDialog::getOpenFileName(this, "Выберите изображение", "", "Images (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (!filePath.isEmpty()) {
        QPixmap pixmap(filePath);
        if (!pixmap.isNull()) {
            // Преобразуем изображение в QByteArray
            QByteArray byteArray;
            QBuffer buffer(&byteArray);
            buffer.open(QIODevice::WriteOnly);
            pixmap.save(&buffer, "PNG"); // Сохраняем как PNG в QByteArray

            imageData = byteArray;

            // Показываем выбранное изображение в метке
            imagePreviewLabel->setPixmap(pixmap.scaled(50, 50, Qt::KeepAspectRatio));  // Примерно размер
            imagePreviewLabel->setText("");  // Скрываем текст
        } else {
            imagePreviewLabel->setText("Ошибка загрузки изображения");
        }
    }
}
