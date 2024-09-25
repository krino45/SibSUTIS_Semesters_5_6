#include "mainwindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), layout(new QVBoxLayout), shapeSelector(new QComboBox),
      input1(new QLineEdit), input2(new QLineEdit), resultLabel(new QLabel) {

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    centralWidget->setLayout(layout);

    shapeSelector->addItem("Circle");
    shapeSelector->addItem("Rectangle");
    shapeSelector->addItem("Triangle");

    layout->addWidget(shapeSelector);
    layout->addWidget(input1);
    layout->addWidget(input2);
    QPushButton *calculateButton = new QPushButton("Calculate", this);
    layout->addWidget(calculateButton);
    layout->addWidget(resultLabel);

    // Connect the shape selector to update input fields
    QObject::connect(shapeSelector, &QComboBox::currentIndexChanged, this, [this](int index) {
        input1->setPlaceholderText(index == 0 ? "Radius" : index == 1 ? "Width" : "Base");
        input2->setPlaceholderText(index == 1 ? "Height" : index == 2 ? "Height" : "");
        input2->setVisible(index == 1 || index == 2);
        input2->setEnabled(index == 1 || index == 2); // Enable or disable input2
    });

    // Connect the button click to the calculateArea slot
    connect(calculateButton, &QPushButton::clicked, this, &MainWindow::calculateArea);
}

MainWindow::~MainWindow() {}

void MainWindow::calculateArea() {
    QString shape = shapeSelector->currentText();
    double area = 0.0;

    if (shape == "Circle") {
        double radius = input1->text().toDouble();
        area = 3.14159 * radius * radius;
    } else if (shape == "Rectangle") {
        double width = input1->text().toDouble();
        double height = input2->text().toDouble();
        area = width * height;
    } else if (shape == "Triangle") {
        double base = input1->text().toDouble();
        double height = input2->text().toDouble();
        area = 0.5 * base * height;
    }

    resultLabel->setText("Area: " + QString::number(area));
}
