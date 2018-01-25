#ifndef __RATING_H__
#define __RATING_H__
#include <QString>
#define GRAPHIC_RATING
#ifdef GRAPHIC_RATING
#include <QMetaType>
#include <QPointF>
#include <QPolygon>
#include <QPainter>
#endif
#include <QVector>
#define MAX_STAR_RATING 10
#define MIN_STAR_RATING 0

class Rating{
public:
    enum EditMode{ Editable, ReadOnly };
    explicit Rating();
    explicit Rating(QString ratingGrade);
#ifdef GRAPHIC_RATING
    void    setupView();
    void    paint(QPainter *painter, const QRect &rect, const QPalette &palette, EditMode mode) const;
    QSize   sizeHint() const;
    int     starCount() const    { return starRating; }
    int     maxStarCount() const { return MAX_STAR_RATING;   }
    void    setStarCount(int stars) {  fromStars(stars);    }
#endif
    void    fromStars(int i);
    void    fromString(QString s);
    bool    equals(const Rating &other) const;
    // operators
    bool    operator == (Rating other) const;
    bool    operator <  (Rating other) const;
    bool    operator >  (Rating other) const;
    bool    operator <= (Rating other) const;
    bool    operator >= (Rating other) const;
    bool    operator != (Rating other) const;
    bool    isEmpty() const {   return this->gradeRating.isEmpty(); }
    bool    isValid() const {   return !isEmpty();  }
    QString sqlSafe() const;
    int     stars(void) const   {   return starRating;  }
    QString grade(void) const   {   return gradeRating; }
private:
#ifdef GRAPHIC_RATING
    QPolygonF starPolygon, diamondPolygon;
#endif
    QString StarsToGrade(int i);
    int     GradeToStars(QString s);
    QString gradeRating;
    int     starRating;
};
#ifdef GRAPHIC_RATING
Q_DECLARE_METATYPE(Rating)
#endif
#endif
