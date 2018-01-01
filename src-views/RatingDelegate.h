#ifndef RATINGDELEGATE_H
#define RATINGDELEGATE_H
#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QAbstractItemModel>

class RatingDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    RatingDelegate(QWidget *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
     void setEditorData(QWidget *editor, const QModelIndex &index) const override;
     void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
private slots:
     void commitAndCloseEditor();
};

#endif // RATINGDELEGATE_H
