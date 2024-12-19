#ifndef CATEGORYMANAGER_H
#define CATEGORYMANAGER_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QByteArray>

class CategoryManager : public QWidget {
    Q_OBJECT

public:
    explicit CategoryManager(QWidget *parent = nullptr);

private:
    QListWidget *categoriesList;
    QLineEdit *categoryInput;
    QPushButton *addButton;
    QPushButton *deleteButton;
    QPushButton *selectImageButton;
    QLabel *imagePreviewLabel;
    QString imagePathInput;
    QByteArray imageData;

    void loadCategories();
    void selectImage();
    void addCategory();
    void deleteCategory();

signals:
    void categoriesUpdated();
};

#endif // CATEGORYMANAGER_H
