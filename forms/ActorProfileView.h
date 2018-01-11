#ifndef ACTORPROFILEVIEW_H
#define ACTORPROFILEVIEW_H
#include "ui_ActorProfileView.h"
#include "definitions.h"
#include "imageeditor.h"
#include <QWidget>
#include <QShortcut>
#include <QLineEdit>
#include <QTextEdit>
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
    void acceptSceneCount(int);

private slots:
    void reloadProfilePhoto();
    void on_selectNewPhoto_clicked();
    void on_downloadPhoto_clicked();
    void on_deletePhoto_clicked();

    void on_deleteActor_clicked();
    void on_saveProfile_clicked();
    void on_editProfile_clicked();
    void on_updateFromWeb_clicked();

    void on_clearFields_clicked();
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
    void onTimeout(void);

private:
    void setupFields();
    void clearFields();
    void setResetAndSaveButtons(bool enabled=true);
    Ui::ActorProfileView *ui;
    ActorPtr current;
    QTimer *timer;
    QSharedPointer<ImageEditor> editor;
    QShortcut *sc_downloadCurrentProfile;
    QShortcut *sc_saveChangesToActor;
    QShortcut *sc_hideProfile;
    QShortcut *sc_chooseNewPhoto;
    QVector<QLineEdit *> lineEdits;
    QVector<QTextEdit *> textEdits;
signals:
    void requestSceneCount();
    void updateFromWeb  (ActorPtr a);
    void saveToDatabase (ActorPtr a);
    void deleteCurrent  ();
    void deleteActor    (ActorPtr a);
    void downloadPhoto  (ActorPtr a);
    void clearChanges();
    void hidden();
    void chooseNewPhoto();
    void reloadProfile();
};

#endif // ACTORPROFILEVIEW_H
