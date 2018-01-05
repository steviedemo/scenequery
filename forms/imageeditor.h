#ifndef IMAGEEDITOR_H
#define IMAGEEDITOR_H
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "imagecropper.h"

class ImageEditor : public QWidget
{
    Q_OBJECT
public:
    explicit ImageEditor(QString source, QString target, QWidget *parent = 0);
    ~ImageEditor();

private slots:
    void crop();
    void save();
    void cancel();
    void selectImage();

private:
    ImageCropper *m_imageCropper;
    QLabel *m_croppedImage;
    QString sourceFile, saveFile;
    int maxHeight;
    bool preInitError;
signals:
    void finished();
    void saved();
};

#endif // IMAGEEDITOR_H
