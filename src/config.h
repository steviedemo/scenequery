#ifndef CONFIG_H
#define CONFIG_H
#include <QDialog>
#include <QFileDialog>
#include <QToolButton>
#include <QString>
#include <QStringList>
#include <QMap>
#define DATA_FOLDER         ".scenequery"
#define HEADSHOT_FOLDER     "headshots"
#define THUMBNAIL_FOLDER    "thumbnails"
#define SETTINGS_FOLDER     "settings"
bool    makeDirectories();
QString findDataLocation();
QString findHeadshotLocation();
QString findThumbnailLocation();
QString findSettingsFile();
class Settings {
public:
    explicit Settings();
    explicit Settings(QString file);
    ~Settings(){}
    bool read(QString file);
    QString get(const QString &key) const;
    QStringList getList(const QString &key) const;
    void setList(const QString &key, const QStringList &values);
    void set(const QString &key, const QString &value);
    bool save(void) const;
private:
    QString path;
    QMap<QString,QString> configMap;
};
class SearchPathDialog : public QDialog{
    Q_OBJECT
public:
    explicit SearchPathDialog(QWidget *parent = 0);
    ~SearchPathDialog();
private slots:
    void showFileDialog();
    void addItem(QString);
    void fileDialogClosed();
    void save();
    void cancel();
private:
    QFileDialog *fileDialog;
    QStringList paths;
    QToolButton *addButton;
    Settings settings;
};

#endif // CONFIG_H
