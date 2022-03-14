#ifndef TDCHART_H
#define TDCHART_H

#include <QtCore/QObject>
#include <QtWidgets/QWidget>
#include <QtDataVisualization>

using namespace QtDataVisualization;

class TDChart : public QObject {
    Q_OBJECT
public:
    explicit TDChart(Q3DScatter *scatter);
    ~TDChart();

    void addData();
    void changeStyle();
    void changeLabelStyle();
    void changeFontSize(int fontsize);
    void setBackgroundEnabled(int enabled);
    void setGridEnabled(int enabled);
    void setSmoothDots(int smooth);
    void toggleItemCount();
    void start();

public slots:
    void setFileName(QString fileName);
    void setNumberOfRows(int rowsCount);
    void changeStyle(int style);
    void changeTheme(int theme);
    void changeShadowQuality(int quality);
    void
    shadowQualityUpdatedByVisual(QAbstract3DGraph::ShadowQuality shadowQuality);
    void initiate();
    void setDensity(int density);

signals:
    void backgroundEnabledChanged(bool enabled);
    void gridEnabledChanged(bool enabled);
    void shadowQualityChanged(int quality);
    void fontChanged(QFont font);
    void pointCountChanged(int count);
    void averageLatitudeChanged(double avgx);
    void averageLongitudeChanged(double avgz);

private:
    QScatterDataProxy *proxy;
    int rowsCount;
    QString fileName;
    Q3DScatter *graph;
    int fontSize;
    QAbstract3DSeries::Mesh style;
    bool smooth;
    int density;
};


#endif // TDCHART_H
