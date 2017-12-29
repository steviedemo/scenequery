#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H
#include "Actor.h"
#include "definitions.h"
#include <QDialog>

namespace Ui {
class ProfileDialog;
}

class ProfileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProfileDialog(ActorPtr a, QWidget *parent = 0);
    ~ProfileDialog();

private slots:
    void on_closeButton_clicked();
    void on_tryAgainButton_clicked();

private:
    Ui::ProfileDialog *ui;
    ActorPtr actor;
signals:
    void closed();
};

#endif // PROFILEDIALOG_H
