#ifndef RATINGEDITOR_H
#define RATINGEDITOR_H
#include <QWidget>
#include <QSize>
#include <QMouseEvent>
#include <QPaintEvent>
#include "Rating.h"

class RatingEditor : public QWidget{
    Q_OBJECT
public:
    RatingEditor(QWidget *parent = 0);
    QSize sizeHint() const override;
    void setRating(const Rating &rating){
        this->myRating = rating;
    }
    Rating rating() {   return myRating;    }
signals:
    void editingFinished();
protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
private:
    int starAtPosition(int x);
    Rating myRating;
};

#endif // RATINGEDITOR_H
