#ifndef REPORTDIALOG_H
#define REPORTDIALOG_H

#include <QDialog>

class QLineEdit;
class QTextEdit;
class QComboBox;

class ReportDialog : public QDialog {
    Q_OBJECT
public:
    explicit ReportDialog(int targetId, bool isAd, QWidget *parent = nullptr);

signals:
    void reportSubmitted();

private slots:
    void submitReport();

private:
    int targetId;
    bool isAd;
    QComboBox *reasonComboBox;
    QTextEdit *explanationEdit;
};

#endif // REPORTDIALOG_H
