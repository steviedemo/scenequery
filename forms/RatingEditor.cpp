#include "RatingEditor.h"
#include <QtGui>
RatingEditor::RatingEditor(QWidget *parent) : QWidget(parent){
    setMouseTracking(true);
    setAutoFillBackground(true);
}

QSize RatingEditor::sizeHint() const{
    return this->myRating.sizeHint();
}

void RatingEditor::paintEvent(QPaintEvent *){
    QPainter painter(this);
    myRating.paint(&painter, rect(), this->palette(), Rating::Editable);
}

void RatingEditor::mouseMoveEvent(QMouseEvent *event){
    int star = starAtPosition(event->x());
    if (star != myRating.starCount() && star != -1){
        myRating.setStarCount(star);
        update();
    }
}

void RatingEditor::mouseReleaseEvent(QMouseEvent *){
    emit editingFinished();
}

int RatingEditor::starAtPosition(int x){
    int star = (x / (myRating.sizeHint().width()/myRating.maxStarCount())) + 1;
    if (star < 0 || star > myRating.maxStarCount()){
        return -1;
    }
    return star;
}
