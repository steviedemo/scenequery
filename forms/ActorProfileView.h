#ifndef ACTORPROFILEVIEW_H
#define ACTORPROFILEVIEW_H
#include "ui_ActorProfileView.h"
#include "definitions.h"
#include "imageeditor.h"
#include "SceneList.h"
#include "DataManager.h"
#include "SQL.h"
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
    void setData(QSharedPointer<DataManager> vault){ this->vault = vault; }
public slots:
    void loadActorProfile(ActorPtr a);
    void loadActorProfile(QString name);
    void showProfileView(QString name);
    void updateProfileView(QString name);
    void hideProfileView();
private slots:
    void reloadProfilePhoto();
    void on_selectNewPhoto_clicked();
    void on_downloadPhoto_clicked();
    void on_deletePhoto_clicked();
    void on_tb_editName_clicked();

    void on_deleteActor_clicked();
    void on_saveProfile_clicked();
    void on_editProfile_clicked();
    void on_updateFromWeb_clicked();

    void on_tb_saveNameEdit_clicked();
    void on_tb_cancelNameEdit_clicked();
    void hideEvent(QHideEvent *event);
    void clearFields();

private:
    void setupFields();
    void outputDetails(ActorPtr);
    void setResetAndSaveButtons(bool enabled=true);
    Ui::ActorProfileView *ui;
    QString oldName, newName;
    ActorPtr current;
    QTimer *timer;
    QSharedPointer<ImageEditor> editor;
    QShortcut *sc_downloadCurrentProfile;
    QShortcut *sc_saveChangesToActor;
    QShortcut *sc_hideProfile;
    QShortcut *sc_chooseNewPhoto;
    QVector<QLineEdit *> lineEdits;
    QVector<QTextEdit *> textEdits;
    QVector<ScenePtr> updateList;
    QSharedPointer<DataManager> vault;
signals:
    void profileChanged(ActorPtr a);
    void updateFromWeb  (ActorPtr a);
    void saveToDatabase (ActorPtr a);
    void deleteCurrent  ();
    void deleteActor    (ActorPtr a);
    void deleteActor(QString name);
    void downloadPhoto  (ActorPtr a);
    void hidden();
    void chooseNewPhoto();
    void reloadProfile();
    void renameFile(ScenePtr s);
    void apv_to_ct_updateBio(QString name);
    void requestActor(QString name);
    void apv_to_mw_sendNewActor(ActorPtr);
    void selectActor(QString);
};

#endif // ACTORPROFILEVIEW_H
