#ifndef EDITLISTINGDIALOG_H
#define EDITLISTINGDIALOG_H

#include <QDialog>

class EditListingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EditListingDialog(int adId, QWidget *parent = nullptr);

signals:

public slots:
};

#endif // EDITLISTINGDIALOG_H
