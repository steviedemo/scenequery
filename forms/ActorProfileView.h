#ifndef ACTORPROFILEVIEW_H
#define ACTORPROFILEVIEW_H
#include "definitions.h"
#include <QWidget>

namespace Ui {
class ActorProfileView;
}

class ActorProfileView : public QWidget
{
    Q_OBJECT

public:
    explicit ActorProfileView(QWidget *parent = 0);
    ~ActorProfileView();

public slots:
    void loadActorProfile(ActorPtr a);

private slots:
    void on_updateFromWeb_clicked();

    void on_saveProfile_clicked();

    void on_closeProfile_clicked();

    void on_reloadFromDb_clicked();
    void on_birthDateDateEdit_userDateChanged(const QDate &date);
    void on_hairColorLineEdit_textChanged(const QString &arg1);
    void on_ethnicityLineEdit_textChanged(const QString &arg1);
    void on_nationalityLineEdit_textEdited(const QString &arg1);
    void on_heightLineEdit_textEdited(const QString &arg1);
    void on_weightLineEdit_textEdited(const QString &arg1);
    void on_eyeColorLineEdit_textEdited(const QString &arg1);
    void on_measurementsLineEdit_textEdited(const QString &arg1);
    void on_aliasesTextEdit_textChanged();
    void on_piercingsTextEdit_textChanged();
    void on_tattoosTextEdit_textChanged();

private:
    void setResetAndSaveButtons(bool enabled=true);
    Ui::ActorProfileView *ui;
    ActorPtr current;
signals:
    void updateFromWeb(ActorPtr a);
    void saveToDatabase(ActorPtr a);
    void clearChanges();
    void hideWindow(void);
    void chooseNewPhoto();
};

#endif // ACTORPROFILEVIEW_H
