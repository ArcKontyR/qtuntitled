#include "moveitem.h"

MoveItem::MoveItem(QObject *parent) :
                                      QObject(parent), QGraphicsItem()
{

}

MoveItem::~MoveItem()
{

}

QRectF MoveItem::boundingRect() const
{
    QPixmap image("C:/Users/USER/Documents/Verhoturov/map.png");
    return QRectF (0,0,image.width(),image.height());
}

void MoveItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setPen(Qt::black);
    painter->setBrush(Qt::green);
    QPointF point(0,0);
    QRectF source(0,0,0,0);
    painter->drawPixmap(point,QPixmap("C:/Users/USER/Documents/Verhoturov/map.png"),source);
}

void MoveItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event){
    //this->setScale(1.5);
}

void MoveItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    /* Устанавливаем позицию графического элемента
     * в графической сцене, транслировав координаты
     * курсора внутри графического элемента
     * в координатную систему графической сцены
     * */
    QPointF coords = event->pos();

    this->setPos(mapToScene(coords) - mouseCoords);
}

void MoveItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    /* При нажатии мышью на графический элемент
     * заменяем курсор на руку, которая держит этот элемента
     * */
     mouseCoords = event->pos();
    this->setCursor(QCursor(Qt::ClosedHandCursor));
}

void MoveItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    /* При отпускании мышью элемента
     * заменяем на обычный курсор стрелку
     * */
    this->setCursor(QCursor(Qt::ArrowCursor));
}

QVariant MoveItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene()) {
        // value is the new position.
        QPointF newPos = value.toPointF();
        double rectx1 = -boundingRect().width()+scene()->sceneRect().width();
        double recty1 = -boundingRect().height()+scene()->sceneRect().height();
        double rectx2 = boundingRect().width()-scene()->sceneRect().width();
        double recty2 = boundingRect().height()-scene()->sceneRect().height();
        QRectF rectf = QRectF(rectx1,recty1,rectx2,recty2);
        if (!rectf.contains(newPos)) {
            // Keep the item inside the scene rect.
            newPos.setX(qMin(rectf.right()-20, qMax(newPos.x(), rectf.left()+20)));
            newPos.setY(qMin(rectf.bottom()-20,qMax(newPos.y(), rectf.top()+20)));
            return newPos;
        }
    }
    return QGraphicsItem::itemChange(change, value);
}
