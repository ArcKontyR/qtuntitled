#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include "chart.h"
#include <QtCore/QAbstractItemModel>
#include <QtCharts>
class ChartView : public QChartView {
    Q_OBJECT

public:
    ChartView(QWidget *parent = nullptr, Chart *chart = nullptr);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;

private:
    QPointF m_lastMousePos;
};
#endif // CHARTVIEW_H
