#include "DeleteButtonDelegate.h"
#include <QtGui>
#include <QApplication>
#include <QDialog>
#define BUTTON_SIZE 20
void DeleteButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const{
    QStyleOptionButton button;
    QRect r = option.rect;
    int x = r.left() + r.width() - BUTTON_SIZE;
    int y = r.top();
    int w = BUTTON_SIZE;
    int h = BUTTON_SIZE;
    button.rect = QRect(x, y, w, h);
    button.icon = QIcon(":Icons/red_close_icon.png");
    button.state = QStyle::State_Enabled;
    QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter);
}
bool DeleteButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel */*model*/, const QStyleOptionViewItem &option, const QModelIndex &/*index*/){
    if (event->type() == QEvent::MouseButtonRelease){
        QMouseEvent *e = (QMouseEvent *) event;
        int clickX = e->x();
        int clickY = e->y();
        QRect r = option.rect;
        int x = r.left() + r.width() - BUTTON_SIZE;
        int y = r.top();
        int w = BUTTON_SIZE;
        int h = BUTTON_SIZE;
        if (clickX > x && clickX < (x + w)){
            if (clickY > y && clickY < (y + h)){
                QDialog *d = new QDialog();
                d->setGeometry(0, 0, 100, 100);
                d->show();
            }
        }
    }
    return true;
}
