#include "chart.h"

Chart::Chart(QGraphicsItem *parent, Qt::WindowFlags wFlags)
    : QChart(QChart::ChartTypeCartesian, parent, wFlags) {

    legend()->hide();
    setTitle("Blank chart");
    setAnimationOptions(QChart::SeriesAnimations);
    setDropShadowEnabled(false);
}

void Chart::clearChart() {
    if (!axes(Qt::Horizontal).isEmpty() && !axes(Qt::Vertical).isEmpty() &&
        !series().isEmpty()) {
        while (!axes(Qt::Horizontal).isEmpty()) {
            removeAxis(axes(Qt::Horizontal).first());
        }
        while (!axes(Qt::Vertical).isEmpty()) {
            removeAxis(axes(Qt::Vertical).first());
        }
        removeAllSeries();
    }
    numberOfRows = 0;
    strSelect = "";
    setTitle("There's nothing left");
    m_series = new QLineSeries;
    m_axisX = new QValueAxis;
    m_daxisX = new QDateTimeAxis;
    m_axisY = new QValueAxis;
    m_x = 0;
    m_y = 0;
    xmin = 0;
    xmax = 0;
    ymin = 0;
    ymax = 0;
    m_dx.setTime(QTime(0, 0, 0, 0));
    chartIsIndex(false);
    i = 0;
    isIncrement = true;
    shouldAppend = true;
    m_yLim = 0;
    isYMinZero = true;
    emit setPointsValue(0);
}

void Chart::setChartTheme(int _theme) { setTheme(QChart::ChartTheme(_theme)); }
void Chart::setNumberOfRows(int _numberOfRows) { numberOfRows = _numberOfRows; }
void Chart::setDBRow(QString _row) { row = _row; }
void Chart::setDBColumn(QString _column) { column = _column; }
void Chart::setDBTitleRow(QString _tRow) { titleRow = _tRow; }
void Chart::setDBTitleColumn(QString _tColumn) { titleColumn = _tColumn; }
void Chart::setFixXChecked(bool _checked) { fixXIsChecked = _checked; }
void Chart::setFixYChecked(bool _checked) { fixYIsChecked = _checked; }
void Chart::setPen(QPen _pen) {
    m_series->setPen(_pen);
}
void Chart::drawPoint(qreal _x, qreal _y) { m_series->append(_x, _y); }
void Chart::chartIsIndex(bool _isIndex) { isIndex = _isIndex; }
void Chart::setTableName(QString _tableName) {
    if (tableName != _tableName)
        tableName = _tableName;
}
void Chart::setDatabase(QSqlDatabase _db) { db = _db; }
void Chart::setUseOptimization(bool _useOpt) { useOptimization = _useOpt; }
void Chart::setUseAltOptimization(bool _useAltOpt) { useAltOptimization = _useAltOpt; }
void Chart::setAODensity(int _aoDensity) { aoDensity = _aoDensity; }
void Chart::isCoordLimit(qreal _y, qreal _yLim) {
    if (isIncrement) {
        if (_y > _yLim) {
            m_yLim = _y;
        } else {
            isIncrement = false;
            shouldAppend = true;
        }
    } else {
        if (_y < _yLim) {
            m_yLim = _y;
        } else {
            isIncrement = true;
            shouldAppend = true;
        }
    }
}

void Chart::compute() {
    clearChart();
    QSqlQuery *query = new QSqlQuery(db);

    int points = 0;
    int truePoints = 0;

    if (row == "i") {
        strSelect = "SELECT %1 FROM DataTable";
        strSelect = strSelect.arg(column);
    } else {
        strSelect = "SELECT %1, %2 FROM DataTable";
        strSelect = strSelect.arg(row).arg(column);
    }
    query->exec(strSelect);
    emit getNumberOfRows(query);
    while (query->next()) {
        if (query->value(1).toString() == "") {
            chartIsIndex(true);
            m_x = i;
            m_y = query->value(0).toDouble();
            ++i;
            if (useOptimization) {
                isCoordLimit(m_y, m_yLim);
                if (shouldAppend)
                    ++truePoints;
            } else {
                m_yLim = m_y;
                shouldAppend = true;
                ++truePoints;
            }

            if (useAltOptimization) {

                if ((truePoints % aoDensity) == 0 && shouldAppend) {
                    shouldAppend = true;
                } else {
                    shouldAppend = false;
                }
            }

        } else {
            m_x = query->value(0).toDouble();

            QString num = query->value(0).toString();
            if (num.length() == 8) {
                m_dx.setTime(
                    QTime(num.left(1).toInt(), num.left(3).right(2).toInt(),
                          num.right(5).left(2).toInt(), num.right(3).toInt()));
            } else if (num.length() == 9) {
                m_dx.setTime(
                    QTime(num.left(2).toInt(), num.left(4).right(2).toInt(),
                          num.right(5).left(2).toInt(), num.right(3).toInt()));
            }
            m_y = query->value(1).toDouble();
            if (useOptimization) {
                isCoordLimit(m_y, m_yLim);
            } else {
                m_yLim = m_y;
                shouldAppend = true;
            }
            ++i;
        }

        if (!qFuzzyCompare(m_y, 0) && isYMinZero) {
            ymax = ymin = m_y;
            isYMinZero = false;
        }
        if (m_x > xmax) {
            xmax = m_x;
        } else if (m_x < xmin) {
            xmin = m_x;
        }
        if (m_y > ymax) {
            ymax = m_y;
        } else if (m_y < ymin) {
            ymin = m_y;
        }


        if (shouldAppend) {
            if (isIndex) {
                drawPoint(m_x, m_yLim);
            } else {
                drawPoint(m_dx.toMSecsSinceEpoch(), m_yLim);
            }
            points++;
            shouldAppend = false;
            int progress = qCeil(100 * i / numberOfRows);
            emit setDrawingProgress(progress);
        }
    }

    if (isIndex) {
        drawPoint(m_x, m_y);
    } else {
        drawPoint(m_dx.toMSecsSinceEpoch(), m_y);
    }
    points++;
    shouldAppend = false;

    addSeries(m_series);

    if (!isIndex) {
        addAxis(m_daxisX, Qt::AlignBottom);
        QDateTime dxmin, dxmax;
        dxmin = calculateTime(xmin);
        dxmax = calculateTime(xmax);
        m_daxisX->setFormat("hh:mm:ss.zzz");
        m_daxisX->setRange(dxmin, dxmax);

        m_series->attachAxis(m_daxisX);
        m_daxisX->setTitleText(titleRow);
    } else {
        addAxis(m_axisX, Qt::AlignBottom);
        m_axisX->setRange(xmin, xmax);
        if (fixXIsChecked) {
            m_axisX->setTickType(QValueAxis::TicksDynamic);
            m_axisX->setTickInterval(m_axisX->max() / m_axisX->tickCount());
        }
        m_series->attachAxis(m_axisX);
        m_axisX->setTitleText(titleRow);
    }

    addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisY);
    if (fixYIsChecked) {
        m_axisY->setTickType(QValueAxis::TicksDynamic);
        m_axisY->setTickInterval(m_axisY->max() / m_axisY->tickCount());
    }
    m_axisY->setRange(ymin, ymax);
    QString chartTitle = "График отношения параметров: '%1' и '%2'";
    chartTitle = chartTitle.arg(titleRow).arg(titleColumn);
    setTitle(chartTitle);
    m_axisY->setTitleText(titleColumn);
    int progress = qCeil(100 * i / numberOfRows);
    emit setDrawingProgress(progress);
    emit setPointsValue(points);
    emit setDrawingProgressDisabled();
    query->~QSqlQuery();
}

QDateTime Chart::calculateTime(qreal _time) {
    int ms = QString::number(_time).right(3).toInt();
    int s = QString::number(_time).right(5).left(2).toInt();
    int m = QString::number(_time).right(7).left(2).toInt();
    int h = QString::number(_time).right(9).left(2).toInt();
    QDateTime final;
    final.setTime(QTime(h, m, s, ms));
    return final;
}
