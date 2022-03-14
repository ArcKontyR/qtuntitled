#ifndef CHART_H
#define CHART_H

#include <QtCharts>
#include <QtCore/QTimer>
#include <QtSql>
#include <QObject>

class Chart : public QChart {
    Q_OBJECT
public:
    Chart(QGraphicsItem *parent = nullptr, Qt::WindowFlags wFlags = {});

public slots:
    void setPen(QPen);
    void drawPoint(qreal, qreal);
    void chartIsIndex(bool isIndex = false);
    void compute();
    void setTableName(QString);
    void clearChart();
    void setFixXChecked(bool);
    void setFixYChecked(bool);
    void setDBRow(QString);
    void setDBColumn(QString);
    void setDBTitleRow(QString);
    void setDBTitleColumn(QString);
    void setNumberOfRows(int);
    void setUseOptimization(bool);
    void setUseAltOptimization(bool);
    void setAODensity(int);
    void setChartTheme(int);

    QDateTime calculateTime(qreal);
private:
    QString tableName;
    QPen pen;
    QLineSeries *m_series = new QLineSeries;
    QValueAxis *m_axisX = new QValueAxis;
    QDateTimeAxis *m_daxisX = new QDateTimeAxis;
    QValueAxis *m_axisY = new QValueAxis;

    int numberOfRows;
    QString titleRow = "";
    QString titleColumn = "";
    QString row = "";
    QString column = "";
    QString strSelect = "";
    qreal m_x;
    qreal m_y;
    qreal m_xPrev;
    qreal m_yPrev;
    qreal xmin;
    qreal xmax;
    qreal ymin;
    qreal ymax;
    QDateTime m_dx;
    bool isIndex;
    long i;
    bool isIncrement;
    bool shouldAppend;
    qreal m_yLim;
    qreal m_xMin;
    bool isYMinZero;
    bool fixXIsChecked;
    bool fixYIsChecked;
    bool useOptimization;
    bool useAltOptimization;
    int aoDensity;

signals:
    void getNumberOfRows(QSqlQuery *);
    void setDrawingProgress(int);
    void setPointsValue(int);
    void setDrawingProgressDisabled();

private slots:
    void isCoordLimit(qreal _y, qreal _yLim);
};

#endif // CHART_H
