#include "tdchart.h"
#include "geotranslator.h"
#include <QtDataVisualization>
#include <QtWidgets>
#include <QtSql>
#include <QtMath>
#include <QGeoCoordinate>
using namespace QtDataVisualization;

TDChart::TDChart(Q3DScatter *scatter)
    : graph(scatter), fontSize(40.0f),
      style(QAbstract3DSeries::MeshPoint), smooth(true), density(100)
{}

void TDChart::setDensity(int _density) {
    density = _density;
    graph->seriesList().at(0)->dataProxy()->resetArray(nullptr);
    addData();
}

void TDChart::setNumberOfRows(int _rows) { rowsCount = _rows; }

void TDChart::initiate() {
    QFont font = graph->activeTheme()->font();
    font.setPointSize(fontSize);
    graph->activeTheme()->setFont(font);
    graph->scene()->activeCamera()->setCameraPreset(
        Q3DCamera::CameraPresetIsometricLeftHigh);

    proxy = new QScatterDataProxy;
    QScatter3DSeries *series = new QScatter3DSeries(proxy);
    series->setItemLabelFormat(
        QStringLiteral("X: @xLabel Y: @yLabel Z: @zLabel"));
    series->setMeshSmooth(smooth);
    series->setMesh(style);
    graph->addSeries(series);
    addData();
}

TDChart::~TDChart() { delete graph; }

void TDChart::setFileName(QString _fileName) { fileName = _fileName; }

void TDChart::addData() {
    if (QSqlDatabase::connectionNames().contains("3DChartConnection")) {
        QSqlDatabase::removeDatabase("3DChartConnection");
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "3DChartConnection");
    db.setDatabaseName("databases/" + fileName + ".sqlite");
    db.open();
    QSqlQuery *query = new QSqlQuery(db);
    query->exec("SELECT generalValue FROM StatisticsGeneralTable WHERE "
                "generalName == 'Записей в файле'");
    query->first();
    rowsCount = static_cast<int>(query->value(0).toDouble());
    query->clear();
    query->exec("SELECT B, L, h FROM DataTable");

    graph->axisX()->setTitle("X - Широта в метрах от центра рассчетов");
    graph->axisX()->setTitleVisible(true);
    graph->axisY()->setTitle("Y - Высота");
    graph->axisY()->setTitleVisible(true);
    graph->axisZ()->setTitle("Z - Долгота в метрах от центра рассчетов");
    graph->axisZ()->setTitleVisible(true);

    QScatterDataArray *dataArray = new QScatterDataArray;
    dataArray->resize(rowsCount / density);
    QScatterDataItem *ptrToDataArray = &dataArray->first();
    int i = 0;
    int pointCount = 0;

    double minx, miny, minz, maxx, maxy, maxz;
    minx = miny = minz = 1000000;
    maxx = maxy = maxz = -1000000;
    while (query->next()) {

        double latitude = qRadiansToDegrees(query->value(0).toDouble());
        double longitude = qRadiansToDegrees(query->value(1).toDouble());
        double altitude = query->value(2).toDouble();
        if (qFuzzyCompare(latitude, 0) && qFuzzyCompare(longitude, 0)) {
            continue;
        }
        if (minx > latitude) {
            minx = latitude;
        } else if (maxx < latitude) {
            maxx = latitude;
        }
        if (minz > longitude) {
            minz = longitude;
        } else if (maxz < longitude) {
            maxz = longitude;
        }
        if (miny > altitude) {
            miny = altitude;
        } else if (maxy < altitude) {
            maxy = altitude;
        }
    }
    double avgx, avgz;
    avgx = (maxx + minx) / 2;
    avgz = (maxz + minz) / 2;

    minx = miny = minz = 1000000;
    maxx = maxy = maxz = -1000000;
    GeoTranslator translator;
    translator.setBaseLatitude(avgx);
    translator.setBaseLongitude(avgz);
    query->first();
    query->previous();
    while (query->next()) {
        double latitude = qRadiansToDegrees(query->value(0).toDouble());
        double longitude = qRadiansToDegrees(query->value(1).toDouble());
        double altitude = query->value(2).toDouble();
        if (qFuzzyCompare(latitude, 0) && qFuzzyCompare(longitude, 0)) {
            continue;
        }
        latitude = translator.LatToZ(latitude);
        longitude = translator.LonToX(longitude);
        if (minx > latitude) {
            minx = latitude;
        } else if (maxx < latitude) {
            maxx = latitude;
        }
        if (miny > altitude) {
            miny = altitude;
        } else if (maxy < altitude) {
            maxy = altitude;
        }
        if (minz > longitude) {
            minz = longitude;
        } else if (maxz < longitude) {
            maxz = longitude;
        }
        if (i % density == 0) {
            ptrToDataArray->setPosition(QVector3D(
                static_cast<float>(latitude), static_cast<float>(altitude),
                static_cast<float>(longitude)));
            ptrToDataArray++;
            pointCount++;
        }
        ++i;
    }
    graph->axisX()->setRange(static_cast<float>(minx),
                             static_cast<float>(maxx));
    graph->axisY()->setRange(static_cast<float>(miny),
                             static_cast<float>(maxy));
    graph->axisZ()->setRange(static_cast<float>(minz),
                             static_cast<float>(maxz));
    graph->axisZ()->setReversed(true);
    emit pointCountChanged(pointCount);
    emit averageLatitudeChanged(avgx);
    emit averageLongitudeChanged(avgz);
    graph->seriesList().at(0)->dataProxy()->resetArray(dataArray);

    translator.~GeoTranslator();
    query->~QSqlQuery();
    db.close();
}

void TDChart::changeStyle(int _style)
{
    QComboBox *cbSender = qobject_cast<QComboBox *>(sender());
    if (cbSender) {
    style = QAbstract3DSeries::Mesh(cbSender->itemData(_style).toInt());
        if (graph->seriesList().size())
            graph->seriesList().at(0)->setMesh(style);
    }
}

void TDChart::setSmoothDots(int smooth)
{
    smooth = bool(smooth);
    QScatter3DSeries *series = graph->seriesList().at(0);
    series->setMeshSmooth(smooth);
}

void TDChart::changeTheme(int theme)
{
    Q3DTheme *currentTheme = graph->activeTheme();
    currentTheme->setType(Q3DTheme::Theme(theme));
    emit backgroundEnabledChanged(currentTheme->isBackgroundEnabled());
    emit gridEnabledChanged(currentTheme->isGridEnabled());
    emit fontChanged(currentTheme->font());
}

void TDChart::changeLabelStyle()
{
    graph->activeTheme()->setLabelBackgroundEnabled(!graph->activeTheme()->isLabelBackgroundEnabled());
}

void TDChart::shadowQualityUpdatedByVisual(QAbstract3DGraph::ShadowQuality sq)
{
    int quality = int(sq);
    emit shadowQualityChanged(quality);
}

void TDChart::changeShadowQuality(int quality)
{
    QAbstract3DGraph::ShadowQuality sq = QAbstract3DGraph::ShadowQuality(quality);
    graph->setShadowQuality(sq);
}

void TDChart::setBackgroundEnabled(int enabled)
{
    graph->activeTheme()->setBackgroundEnabled(static_cast<bool>(enabled));
}

void TDChart::setGridEnabled(int enabled)
{
    graph->activeTheme()->setGridEnabled(static_cast<bool>(enabled));
}
