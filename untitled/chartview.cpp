#include "chartview.h"
#include <QtCharts>

ChartView::ChartView(QWidget *parent,Chart* chart)
    : QChartView(chart, parent)
{
    setDragMode(QGraphicsView::NoDrag);
    this->setMouseTracking(true);
    setRenderHint(QPainter::Antialiasing);
    setRubberBand(QChartView::VerticalRubberBand);
}

void ChartView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton)
    {
        QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));
        m_lastMousePos = event->pos();
        event->accept();
    }

    QChartView::mousePressEvent(event);
}

void ChartView::mouseMoveEvent(QMouseEvent *event)
{
    // pan the chart with a middle mouse drag
    if (event->buttons() & Qt::MiddleButton)
    {
        auto dPos = event->pos() - m_lastMousePos;
        chart()->scroll(-dPos.x(), dPos.y());

        m_lastMousePos = event->pos();
        event->accept();


    }
    QApplication::restoreOverrideCursor();
    QChartView::mouseMoveEvent(event);
}

void ChartView::wheelEvent(QWheelEvent *event)
{
    qreal factor;
    if ( event->angleDelta().y() > 0 )
        factor = 2.0;
    else
        factor = 0.5;

    QRectF r = QRectF(chart()->plotArea().left(),chart()->plotArea().top(),
                      chart()->plotArea().width()/factor,chart()->plotArea().height()/factor);
    QPointF mousePos = mapFromGlobal(QCursor::pos());
    r.moveCenter(mousePos);
    chart()->zoomIn(r);
    QPointF delta = chart()->plotArea().center() -mousePos;
    chart()->scroll(delta.x(),-delta.y());
}
