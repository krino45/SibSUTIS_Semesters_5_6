#include "mainwindow.h"
#include "authorsdialog.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <iostream>
#include <cmath>
#include <float.h>
class AreaCalculator {
public:
    static double square(double side) {
        return side * side;
    }

    static double rectangle(double width, double height) {
        return width * height;
    }

    static double parallelogram(double base, double height) {
        return base * height;
    }

    static double rhombus(double diagonal1, double diagonal2) {
        return (diagonal1 * diagonal2) / 2;
    }

    static double triangle(double base, double height) {
        return (base * height) / 2;
    }

    static double trapezoid(double base1, double base2, double height) {
        return ((base1 + base2) * height) / 2;
    }

    static double circle(double radius) {
        return M_PI * radius * radius;
    }

    static double sector(double radius, double angle) {
        return (angle / 360.0) * M_PI * radius * radius;
    }

};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->arg1_label->hide();
    ui->arg2_label->hide();
    ui->arg3_label->hide();
    ui->arg1_spinBox->hide();
    ui->arg2_spinBox->hide();
    ui->arg3_spinBox->hide();
    ui->PictureLabel->hide();
    ui->FormulaLabel->hide();
    ui->err_label->hide();
    ui->result_label->hide();
    ui->calculateResult->hide();
    this->setWindowTitle("Калькулятор площади фигур");
    this->setWindowIcon(QIcon("C:/Users/krina/OneDrive/Desktop/v.jpg"));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_actionAuthors_triggered()
{
    AuthorsDialog au_dialog;
    au_dialog.exec();
}

void MainWindow::on_actionExit_triggered()
{
    QCoreApplication::quit();
}

void MainWindow::on_actionClear_triggered()
{
    ui->arg1_spinBox->setRange(0, DBL_MAX);
    ui->arg2_spinBox->setRange(0, DBL_MAX);
    ui->arg3_spinBox->setRange(0, DBL_MAX);
    ui->arg1_spinBox->setValue(0.0);
    ui->arg2_spinBox->setValue(0.0);
    ui->arg3_spinBox->setValue(0.0);
    ui->comboBox->setCurrentText("-");
}

void MainWindow::on_comboBox_currentTextChanged(const QString &arg1)
{
    ui->arg1_spinBox->setRange(0, DBL_MAX);
    ui->arg2_spinBox->setRange(0, DBL_MAX);
    ui->arg3_spinBox->setRange(0, DBL_MAX);
    ui->FormulaLabel->setText(arg1);
    ui->PictureLabel->setText(QString("hi"));
    if (QString::compare(arg1, QString("-")) == 0) {
        ui->arg1_label->hide();
        ui->arg2_label->hide();
        ui->arg3_label->hide();
        ui->arg1_spinBox->hide();
        ui->arg2_spinBox->hide();
        ui->arg3_spinBox->hide();
        ui->PictureLabel->hide();
        ui->FormulaLabel->hide();
        ui->err_label->hide();
        ui->result_label->hide();
        ui->calculateResult->hide();
        return;
    }
    else{
        ui->arg2_label->hide();
        ui->arg3_label->hide();
        ui->arg2_spinBox->hide();
        ui->arg3_spinBox->hide();
        ui->arg1_label->show();
        ui->arg1_spinBox->show();
        ui->FormulaLabel->show();
        ui->PictureLabel->show();
        ui->calculateResult->show();
    }
    if (QString::compare(arg1, QString("Квадрат")) == 0)
    {
        ui->FormulaLabel->setText("S = a * a");
        ui->PictureLabel->setPixmap(QPixmap("../lab1/S-kvadrata-1.png").scaledToHeight(150));
        ui->arg1_label->setText("Сторона a:");
    }
    else if (QString::compare(arg1, QString("Прямоугольник")) == 0)
    {
        ui->FormulaLabel->setText("S = a * b");
        ui->PictureLabel->setPixmap(QPixmap("../lab1/primoygolnik_red_a.png").scaledToHeight(150));
        ui->arg1_label->setText("Сторона a:");
        ui->arg2_label->show();
        ui->arg2_spinBox->show();
        ui->arg2_label->setText("Сторона b:");
    }
    else if (QString::compare(arg1, QString("Параллелограмм")) == 0)
    {
        ui->FormulaLabel->setText("S = a * h");
        ui->PictureLabel->setPixmap(QPixmap("../lab1/parrallel.png").scaledToHeight(150));
        ui->arg1_label->setText("Основание a:");
        ui->arg2_label->show();
        ui->arg2_spinBox->show();
        ui->arg2_label->setText("Высота h:");
    }
    else if (QString::compare(arg1, QString("Ромб")) == 0)
    {
        ui->FormulaLabel->setText("S = (d1 * d2) / 2");
        ui->PictureLabel->setPixmap(QPixmap("../lab1/rhomb.png").scaledToHeight(150));
        ui->arg1_label->setText("Диагональ d1:");
        ui->arg2_label->show();
        ui->arg2_spinBox->show();
        ui->arg2_label->setText("Диагональ d2:");
    }
    else if (QString::compare(arg1, QString("Треугольник")) == 0)
    {
        ui->FormulaLabel->setText("S = (a * h) / 2");
        ui->PictureLabel->setPixmap(QPixmap("../lab1/triangle.jpeg").scaledToHeight(150));
        ui->arg1_label->setText("Сторона a:");
        ui->arg2_label->show();
        ui->arg2_spinBox->show();
        ui->arg2_label->setText("Высота h:");
    }
    else if (QString::compare(arg1, QString("Трапеция")) == 0)
    {
        ui->FormulaLabel->setText("S = (a + b) * h / 2");
        ui->PictureLabel->setPixmap(QPixmap("../lab1/trapezosid.jpeg").scaledToHeight(150));
        ui->arg1_label->setText("Сторона a:");
        ui->arg2_label->show();
        ui->arg2_spinBox->show();
        ui->arg2_label->setText("Сторона b:");
        ui->arg3_label->show();
        ui->arg3_spinBox->show();
        ui->arg3_label->setText("Высота h:");
    }
    else if (QString::compare(arg1, QString("Круг")) == 0)
    {
        ui->FormulaLabel->setText("S = pi * r * r");
        ui->PictureLabel->setPixmap(QPixmap("../lab1/cirlc.png").scaledToHeight(150));
        ui->arg1_label->setText("Радиус r:");
    }
    else if (QString::compare(arg1, QString("Сектор")) == 0)
    {
        ui->FormulaLabel->setText("S = pi * R * R * (a / 360)");
        ui->PictureLabel->setPixmap(QPixmap("../lab1/sector.png").scaledToHeight(150));
        ui->arg1_label->setText("Радиус R:");
        ui->arg2_label->show();
        ui->arg2_spinBox->show();
        ui->arg2_label->setText("Угол a:");
        ui->arg2_spinBox->setRange(0, 360);
    }


}

void MainWindow::on_calculateResult_clicked()
{
    QString arg1 = ui->comboBox->currentText();
    ui->result_label->show();
    if (QString::compare(arg1, QString("Квадрат")) == 0)
    {
        ui->result_label->setText("Результат:" +
                                  QString::number(AreaCalculator::square( ui->arg1_spinBox->value() ) ) );
    }
    else if (QString::compare(arg1, QString("Прямоугольник")) == 0)
    {
        ui->result_label->setText("Результат:" +
                                  QString::number(AreaCalculator::rectangle( ui->arg1_spinBox->value(),
                                                                             ui->arg2_spinBox->value()) ) );
    }
    else if (QString::compare(arg1, QString("Параллелограмм")) == 0)
    {
        ui->result_label->setText("Результат:" +
                                  QString::number(AreaCalculator::parallelogram( ui->arg1_spinBox->value(),
                                                                                 ui->arg2_spinBox->value()) ) );
    }
    else if (QString::compare(arg1, QString("Ромб")) == 0)
    {
        ui->result_label->setText("Результат:" +
                                  QString::number(AreaCalculator::rhombus( ui->arg1_spinBox->value(),
                                                                           ui->arg2_spinBox->value()) ) );
    }
    else if (QString::compare(arg1, QString("Треугольник")) == 0)
    {
        ui->result_label->setText("Результат:" +
                                  QString::number(AreaCalculator::triangle( ui->arg1_spinBox->value(),
                                                                            ui->arg2_spinBox->value()) ) );
    }
    else if (QString::compare(arg1, QString("Трапеция")) == 0)
    {
        ui->result_label->setText("Результат:" +
                                  QString::number(AreaCalculator::trapezoid( ui->arg1_spinBox->value(),
                                                                             ui->arg2_spinBox->value(),
                                                                             ui->arg3_spinBox->value()) ) );
    }
    else if (QString::compare(arg1, QString("Круг")) == 0)
    {
        ui->result_label->setText("Результат:" +
                                  QString::number(AreaCalculator::circle( ui->arg1_spinBox->value()) ) );
    }
    else if (QString::compare(arg1, QString("Сектор")) == 0)
    {
        ui->result_label->setText("Результат:" +
                                  QString::number(AreaCalculator::sector( ui->arg1_spinBox->value(),
                                                                          ui->arg2_spinBox->value()) ) );
    }
    else {
        ui->result_label->setText("Err: unexpected error");
    }
}
