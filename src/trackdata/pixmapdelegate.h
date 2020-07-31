#ifndef PIXMAPDELEGATE_H
#define PIXMAPDELEGATE_H

#include <QStyledItemDelegate>

class PixmapDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    PixmapDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // PIXMAPDELEGATE_H
