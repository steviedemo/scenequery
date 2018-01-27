#ifndef FILTERWIDGET_H
#define FILTERWIDGET_H

#include <QWidget>
#include "FilterSet.h"
#include "definitions.h"
namespace Ui {
class FilterWidget;
}

class FilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FilterWidget(QWidget *parent = 0);
    ~FilterWidget();
public slots:
    void setEthnicityValues(QStringList &l);
    void setHairValues(QStringList &l);
private slots:

private:
    Ui::FilterWidget *ui;
    void initCb(class QComboBox *);
signals:
    void applyActorFilters(FilterSet);
    void applySceneFilters(FilterSet);
};

#endif // FILTERWIDGET_H
