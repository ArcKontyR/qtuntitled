#include "chart.h"

Chart::Chart(QGraphicsItem *parent, Qt::WindowFlags wFlags)
    : QChart(QChart::ChartTypeCartesian, parent, wFlags), m_series(nullptr),
      m_axisX(new QValueAxis()),m_daxisX(new QDateTimeAxis()), m_axisY(new QValueAxis()), m_step(0), m_x(0),
      m_y(0) {


    //m_timer.start();
}

Chart::~Chart() {}

void Chart::handleTimeout() {

}
