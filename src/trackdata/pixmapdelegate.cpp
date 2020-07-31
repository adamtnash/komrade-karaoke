#include "pixmapdelegate.h"
#include <QPainter>

PixmapDelegate::PixmapDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{

}

void PixmapDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto pixmap = index.data().value<QPixmap>();
    painter->drawImage(option.rect, pixmap.toImage(), pixmap.rect());
}
