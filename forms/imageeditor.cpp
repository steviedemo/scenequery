#include "imageeditor.h"
#include "imagecropper.h"
#include <QPixmap>
#include <QFileInfo>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

ImageEditor::ImageEditor(QString file, QString target, QWidget *parent) :
    QWidget(parent), sourceFile(file), saveFile(target)
{
    if (QFileInfo(sourceFile).exists()){
        maxHeight = 600;
        // Set up the cropper
        m_imageCropper = new ImageCropper(this);
        m_imageCropper->resize( 600, 600 );
        m_imageCropper->setImage(QPixmap(sourceFile));
        m_imageCropper->setBackgroundColor( Qt::lightGray );
        m_imageCropper->setCroppingRectBorderColor( Qt::magenta);
        // Set up the Cropped Image frame
        m_croppedImage = new QLabel(this);
        m_croppedImage->setPixmap(QPixmap(":/img.jpg").scaledToHeight(maxHeight));
        m_croppedImage->setMaximumHeight(maxHeight);
        m_croppedImage->setScaledContents(true);
        // Set up the buttons
        QPushButton* cropBtn = new QPushButton("Crop", this);
        QPushButton* selectButton = new QPushButton("Select Image", this);
        QPushButton* cancelButton = new QPushButton("Cancel", this);
        QPushButton* saveButton = new QPushButton("Save", this);
        connect(cropBtn,        SIGNAL(clicked()), this, SLOT(crop()));
        connect(selectButton,   SIGNAL(pressed()), this, SLOT(selectImage()));
        connect(cancelButton,   SIGNAL(pressed()), this, SLOT(cancel()));
        connect(saveButton,     SIGNAL(pressed()), this, SLOT(save()));
        // Set up image Layout
        QHBoxLayout *imageLayout = new QHBoxLayout();
        imageLayout->addWidget(m_imageCropper);
        imageLayout->addWidget(m_croppedImage);
        // Set up button layout
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->addWidget(cancelButton);
        buttonLayout->addWidget(selectButton);
        buttonLayout->insertStretch(2);
        buttonLayout->addWidget(cropBtn);
        buttonLayout->addWidget(saveButton);
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
    delete m_imageCropper;
    delete m_croppedImage;
}
void ImageEditor::crop(){
    m_croppedImage->setPixmap(QPixmap(m_imageCropper->cropImage()).scaledToHeight(maxHeight));
    m_croppedImage->setScaledContents(true);
}
void ImageEditor::save(){
    QFile photo(sourceFile);
    if (photo.copy(sourceFile, saveFile)){
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
