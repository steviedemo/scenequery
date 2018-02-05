#ifndef DELETEBUTTONDELEGATE_H
#define DELETEBUTTONDELEGATE_H
#include <QItemDelegate>

class DeleteButtonDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    DeleteButtonDelegate(QObject *parent = 0) : QItemDelegate(parent){}
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
};

#endif // DELETEBUTTONDELEGATE_H
