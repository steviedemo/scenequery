#include "imageeditor.h"
#include "imagecropper.h"
#include <QPixmap>
#include "genericfunctions.h"
#include "filenames.h"
#include <QBuffer>
#include <QFileInfo>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QCheckBox>
#include <QSharedPointer>
ImageEditor::ImageEditor(QString file, QString target, QWidget *parent) :
    QWidget(parent), sourceFile(file), saveFile(target)
{
    if (QFileInfo(sourceFile).exists()){
        maxHeight = 600;
        // Set up the cropper
        m_imageCropper = QSharedPointer<ImageCropper>(new ImageCropper());
        //m_imageCropper->resize( 600, 600 );
        QImage image(sourceFile);
        QSize imageSize = getScaledSize(image, maxHeight);
        m_imageCropper->setFixedSize(imageSize);
        m_imageCropper->setImage(QPixmap(sourceFile));
        m_imageCropper->setBackgroundColor( Qt::lightGray );
        m_imageCropper->setCroppingRectBorderColor( Qt::magenta);
        m_imageCropper->setProportionFixed(true);
        // Set up the Cropped Image frame
        m_croppedImage = QSharedPointer<QLabel>(new QLabel());
        m_croppedImage->setPixmap(QPixmap(sourceFile).scaledToHeight(maxHeight));

//        m_croppedImage->setMaximumHeight(maxHeight);
        m_croppedImage->setScaledContents(true);
        m_croppedImage->setFixedSize(imageSize);
        // Set up the buttons
        this->pb_crop = new QPushButton("Crop", this);
        connect(pb_crop,        SIGNAL(clicked()), this, SLOT(crop()));
        this->pb_save = new QPushButton("Save", this);
        connect(pb_save,     SIGNAL(pressed()), this, SLOT(save()));
        QPushButton* selectButton = new QPushButton("Select Image", this);
        connect(selectButton,   SIGNAL(pressed()), this, SLOT(selectImage()));
        QPushButton* cancelButton = new QPushButton("Cancel", this);
        connect(cancelButton,   SIGNAL(pressed()), this, SLOT(cancel()));
        QCheckBox* fixedProportions = new QCheckBox("Fixed Proportions", this);
        fixedProportions->setChecked(true);
        connect(fixedProportions, SIGNAL(toggled(bool)), m_imageCropper.data(), SLOT(setProportionFixed(bool)));

        // Set up image Layout
        QHBoxLayout *imageLayout = new QHBoxLayout();
        imageLayout->addWidget(m_imageCropper.data());
        imageLayout->addWidget(m_croppedImage.data());
        // Set up button layout
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->addWidget(cancelButton);
        buttonLayout->addWidget(selectButton);
        buttonLayout->addWidget(fixedProportions);
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
    const QImage image = pm_croppedImage.toImage();
    QSize scaledSize = getScaledSize(image, maxHeight);
    QPixmap scaledImage = pm_croppedImage.scaled(scaledSize);
    m_croppedImage->setPixmap(scaledImage);
    m_croppedImage->setFixedSize(scaledSize);
//    m_croppedImage->setScaledContents(true);
    pb_save->setEnabled(true);
    save();
}

void ImageEditor::save(){
    if (!pm_croppedImage.isNull()){
        QFile file(saveFile);
        file.open(QIODevice::WriteOnly);
        pm_croppedImage.save(&file, "JPG");
        writeHeadshotThumbnail(saveFile);
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
