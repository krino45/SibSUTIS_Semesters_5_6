#ifndef AUTHORSDIALOG_H
#define AUTHORSDIALOG_H

#include <QDialog>

namespace Ui {
class AuthorsDialog;
}

class AuthorsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AuthorsDialog(QWidget *parent = nullptr);
    ~AuthorsDialog();

private:
    Ui::AuthorsDialog *ui;
};

#endif // AUTHORSDIALOG_H
