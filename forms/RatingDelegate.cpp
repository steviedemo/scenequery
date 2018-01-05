#include "RatingDelegate.h"
#include "RatingEditor.h"
#include "Rating.h"

RatingDelegate::RatingDelegate(QWidget *parent) : QStyledItemDelegate(parent){
}
void RatingDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const{
    if (index.data().canConvert<Rating>()){
        Rating rating = qvariant_cast<Rating>(index.data());
        if (option.state & QStyle::State_Selected){
            painter->fillRect(option.rect, option.palette.highlight());
        }
        rating.paint(painter, option.rect, option.palette, Rating::ReadOnly);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QWidget *RatingDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const{
    if (index.data().canConvert<Rating>()){
        RatingEditor *editor = new RatingEditor(parent);
        connect(editor, &RatingEditor::editingFinished, this, &RatingDelegate::commitAndCloseEditor);
        return editor;
    } else {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

void RatingDelegate::commitAndCloseEditor(){
    RatingEditor *editor = qobject_cast<RatingEditor *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}

void RatingDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const{
    if (index.data().canConvert<Rating>()){
        Rating rating = qvariant_cast<Rating>(index.data());
        RatingEditor *ratingEditor = qobject_cast<RatingEditor *>(editor);
        ratingEditor->setRating(rating);
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}
void RatingDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const{
    if (index.data().canConvert<Rating>()){
        RatingEditor *ratingEditor = qobject_cast<RatingEditor *>(editor);
        model->setData(index, QVariant::fromValue(ratingEditor->rating()));
    }
}

QSize RatingDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const{
    if (index.data().canConvert<Rating>()){
        Rating rating = qvariant_cast<Rating>(index.data());
        return rating.sizeHint();
    } else {
        return QStyledItemDelegate::sizeHint(option, index);
    }
}
