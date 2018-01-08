#include "imageeditor.h"
#include "imagecropper.h"
#include <QPixmap>
#include <QBuffer>
#include <QFileInfo>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSharedPointer>
ImageEditor::ImageEditor(QString file, QString target, QWidget *parent) :
    QWidget(parent), sourceFile(file), saveFile(target)
{
    if (QFileInfo(sourceFile).exists()){
        maxHeight = 600;
        // Set up the cropper
        m_imageCropper = QSharedPointer<ImageCropper>(new ImageCropper());
        m_imageCropper->resize( 600, 600 );
        m_imageCropper->setImage(QPixmap(sourceFile));
        m_imageCropper->setBackgroundColor( Qt::lightGray );
        m_imageCropper->setCroppingRectBorderColor( Qt::magenta);
        // Set up the Cropped Image frame
        m_croppedImage = QSharedPointer<QLabel>(new QLabel());
        m_croppedImage->setPixmap(QPixmap(sourceFile).scaledToHeight(maxHeight));
        m_croppedImage->setMaximumHeight(maxHeight);
        m_croppedImage->setScaledContents(true);
        // Set up the buttons
        this->pb_crop = new QPushButton("Crop", this);
        this->pb_save = new QPushButton("Save", this);
        QPushButton* selectButton = new QPushButton("Select Image", this);
        QPushButton* cancelButton = new QPushButton("Cancel", this);
        connect(pb_crop,        SIGNAL(clicked()), this, SLOT(crop()));
        connect(selectButton,   SIGNAL(pressed()), this, SLOT(selectImage()));
        connect(cancelButton,   SIGNAL(pressed()), this, SLOT(cancel()));
        connect(pb_save,     SIGNAL(pressed()), this, SLOT(save()));
        // Set up image Layout
        QHBoxLayout *imageLayout = new QHBoxLayout();
        imageLayout->addWidget(m_imageCropper.data());
        imageLayout->addWidget(m_croppedImage.data());
        // Set up button layout
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->addWidget(cancelButton);
        buttonLayout->addWidget(selectButton);
        buttonLayout->insertStretch(2);
        buttonLayout->addWidget(pb_crop);
        buttonLayout->addWidget(pb_save);
        pb_save->setDisabled(true);
        // Set up main layout
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->addLayout(imageLayout);
        mainLayout->addLayout(buttonLayout);
        this->show();
    } else {
        QMessageBox::warning(this, tr("Error"), QString("Unable to open %1").arg(sourceFile), QMessageBox::Close);
    }
}

ImageEditor::~ImageEditor(){
    delete pb_save;
    delete pb_crop;
}

void ImageEditor::crop(){
    this->pm_croppedImage = QPixmap(m_imageCropper->cropImage());
    m_croppedImage->setPixmap(pm_croppedImage.scaledToHeight(maxHeight));
    m_croppedImage->setScaledContents(true);
    pb_save->setEnabled(true);
}

void ImageEditor::save(){
    if (!pm_croppedImage.isNull()){
        QFile file(saveFile);
        file.open(QIODevice::WriteOnly);
        pm_croppedImage.save(&file, "JPG");
        emit saved();
        QMessageBox box(QMessageBox::NoIcon, tr("File Saved"), "Cropped Image successfully saved!", QMessageBox::Close | QMessageBox::Retry, this);
        box.setWindowModality(Qt::WindowModal);
        box.setButtonText(QMessageBox::Retry, "Crop Again");
        if (box.exec() == QMessageBox::Close){
            emit finished();
            this->close();
        }
    } else {
        QMessageBox::warning(this, tr("Error"), QString("Error Saving File to %1").arg(saveFile), QMessageBox::Close);
    }
}

void ImageEditor::selectImage(){
    QString temp = QFileDialog::getOpenFileName(this, tr("Select Image"), QString(), "*.jpg;*.jpeg;*.png");
    if (!temp.isEmpty()){
        if (QFileInfo(temp).exists()){
            this->sourceFile = temp;
            this->m_imageCropper->setImage(QPixmap(sourceFile));
            this->m_croppedImage->setPixmap(QPixmap(sourceFile).scaledToHeight(maxHeight));
        } else {
            QMessageBox::warning(this, tr("Error"), QString("Unable to load %1").arg(temp), QMessageBox::Ok);
        }
    }
}

void ImageEditor::cancel(){
    emit finished();
    this->close();
}
