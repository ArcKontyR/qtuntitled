#ifndef MOVEITEM_H
#define MOVEITEM_H

#include <QtWidgets>
#include <QObject>
#include <QGraphicsItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QCursor>
class MoveItem : public QObject, public QGraphicsItem
{
    Q_OBJECT

public:
    explicit MoveItem(QObject *parent = nullptr);
    ~MoveItem() override;


private:
    QRectF boundingRect() const override;
    QPointF mouseCoords;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void setScale(qreal factor);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
};

#endif // MOVEITEM_H
