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
        qDebug() << "should clear";
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
}

void Chart::setSeries() {
    if (pen == QPen(NULL_PEN)) {
        m_series->setPen(defaultPen);
    } else {
        m_series->setPen(pen);
    }
}
void Chart::setNumberOfRows(int _numberOfRows) { numberOfRows = _numberOfRows; }
void Chart::setDBRow(QString _row) { row = _row; }
void Chart::setDBColumn(QString _column) { column = _column; }
void Chart::setFixXChecked(bool _checked) { fixXIsChecked = _checked; }
void Chart::setFixYChecked(bool _checked) { fixYIsChecked = _checked; }
void Chart::setPen(QPen _pen) { pen = _pen; }
void Chart::drawPoint(qreal _x, qreal _y) { m_series->append(_x, _y); }
void Chart::chartIsIndex(bool _isIndex) { isIndex = _isIndex; }
void Chart::setTableName(QString _tableName) {
    if (tableName != _tableName)
        tableName = _tableName;
}
void Chart::setDatabase(QSqlDatabase _db) { db = _db; }

void Chart::compute() {
    setSeries();
    QSqlQuery *query = new QSqlQuery(db);

    // qDebug() << row << column;
    int points = 0;

    if (row == "i") {
        strSelect = "SELECT %1 FROM '%2'";
        strSelect = strSelect.arg(column).arg(tableName);
    } else {
        strSelect = "SELECT %1, %2 FROM '%3'";
        strSelect = strSelect.arg(row).arg(column).arg(tableName);
    }
    // qDebug() << strSelect;
    query->exec(strSelect);
    emit getNumberOfRows(query);
    // int numberOfRows = countSelectQueryRows(query);
    while (query->next()) {
        if (query->value(1).toString() == "") {
            chartIsIndex(true);
            m_x = i;
            m_y = query->value(0).toDouble();
            //++i;
            if (isIncrement) {
                if (m_y > m_yLim) {
                    m_yLim = m_y;
                } else {
                    isIncrement = false;
                    shouldAppend = true;
                }
            } else {
                if (m_y < m_yLim) {
                    m_yLim = m_y;
                } else {
                    isIncrement = true;
                    shouldAppend = true;
                }
            }
            ++i;
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

            if (isIncrement) {
                if (m_y > m_yLim) {
                    m_yLim = m_y;
                } else {
                    isIncrement = false;
                    shouldAppend = true;
                }
            } else {
                if (m_y < m_yLim) {
                    m_yLim = m_y;
                } else {
                    isIncrement = true;
                    shouldAppend = true;
                }
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
            // qDebug() << progress << i << numberOfRows ;
            emit setDrawingProgress(progress);
        }
    }

    query->~QSqlQuery();
    db.close();
    db.~QSqlDatabase();
    db.removeDatabase(db.connectionName());

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
    } else {
        addAxis(m_axisX, Qt::AlignBottom);
        m_axisX->setRange(xmin, xmax);
        if (fixXIsChecked) {
            m_axisX->setTickType(QValueAxis::TicksDynamic);
            m_axisX->setTickInterval(m_axisX->max() / m_axisX->tickCount());
        }
        m_series->attachAxis(m_axisX);
    }

    addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisY);
    if (fixYIsChecked) {
        m_axisY->setTickType(QValueAxis::TicksDynamic);
        m_axisY->setTickInterval(m_axisY->max() / m_axisY->tickCount());
    }
    m_axisY->setRange(ymin, ymax);
    setTitle(row + "  " + column + " chart");
    int progress = qCeil(100 * i / numberOfRows);
    emit setDrawingProgress(progress);
    emit setPointsValue(points);
    emit setDrawingProgressDisabled();
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
