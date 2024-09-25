#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void calculateArea();

private:
    QVBoxLayout *layout;
    QComboBox *shapeSelector;
    QLineEdit *input1;
    QLineEdit *input2;
    QLabel *resultLabel;
};

#endif // MAINWINDOW_H
