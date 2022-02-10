#ifndef CHART_H
#define CHART_H

#include <QtCharts>
#include <QtCore/QTimer>
#include <QtSql>
#include <QObject>

//class QLineSeries;
//class QValueAxis;
//class QDateTimeAxis;

class Chart : public QChart {
    Q_OBJECT
public:
    Chart(QGraphicsItem *parent = nullptr,
          Qt::WindowFlags wFlags = {});
    virtual ~Chart() override;
    void setSqlQueryString(QString query);

public slots:
    void handleTimeout();

private:
    QSqlQuery *query = new QSqlQuery;
    QString sqlQueryString;
    QTimer m_timer;
    // QScatterSeries *m_series;
    //QStringList m_titles;
    QLineSeries *m_series;
    QValueAxis *m_axisX;
    QDateTimeAxis *m_daxisX;
    QValueAxis *m_axisY;
    qreal m_step;
    qreal m_x;
    qreal m_y;
    qreal xmin = 0;
    qreal xmax = 0;
    qreal ymin = 0;
    qreal ymax = 0;
    QDateTime m_dx;
    long i = 0;

};

#endif // CHART_H
