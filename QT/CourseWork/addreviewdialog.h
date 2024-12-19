#ifndef ADDREVIEWDIAGOG_H
#define ADDREVIEWDIAGOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
#include <QMessageBox>

class AddReviewDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddReviewDialog(int authorId, int targetUserId, QWidget *parent = nullptr);

signals:

public slots:

private:
    int authorId;
    int targetUserId;
    QComboBox *ratingComboBox;
    QTextEdit *reviewTextEdit;

    void submitReview();
};

#endif // ADDREVIEWDIAGOG_H
