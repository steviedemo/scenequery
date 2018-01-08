#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H
#include "Actor.h"
#include "definitions.h"
#include <QCloseEvent>
#include <QDialog>
#include <QVector>
#include <QLineEdit>
#include <QTextEdit>
namespace Ui {
class ProfileDialog;
}

class ProfileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProfileDialog(ActorPtr a, QWidget *parent = 0);
    explicit ProfileDialog(QString, QWidget *parent=0);
    ~ProfileDialog();
public slots:
    void ct_to_pd_receiveProfile(ActorPtr);
    void db_to_pd_receiveProfileWithID(ActorPtr);
private slots:
    void on_pb_retry_clicked();

    void on_pb_saveActor_clicked();

private:
    void setUpFields();
    Ui::ProfileDialog *ui;
    ActorPtr actor;
    void printDetails(ActorPtr);
    void clearDetails();
    QVector<QLineEdit *> lineEdits;
    QVector<QTextEdit *> textEdits;
signals:
    void closed();
    void pd_to_ct_getProfile(QString);
    void pd_to_db_saveProfile(ActorPtr);
    void pd_to_mw_addDisplayItem(ActorPtr);
};

#endif // PROFILEDIALOG_H
