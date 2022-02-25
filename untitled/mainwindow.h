#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <chart.h>
#include <chartview.h>
#include <QtWidgets>
#include <QtSql>
#include <QtCharts>
#include <QtCore/QTimer>
#include <QtQuickWidgets/QQuickWidget>
struct BpiData
{
    char hdr[4];            //!< Заголовок при записи в файл BIV=
    uint32_t nNavCounter;   //!< Счетчик работы навигационного алгоритма
    uint32_t sns_utc;       //!< hhmmssxxx, пример 101420100 это 10:14:20.100
    uint32_t snd_utc;       //!< hhmmssxxx, пример 101420100 это 10:14:20.100, время СНД
    double V_X;             //!< Продольная составляющая земной скорости в ЛКС
    double V_Y;             //!< Вертикальная составляющая земной скорости в ЛКС
    double V_Z;             //!< Боковая составляющая земной скорости в ЛКС
    double Vair;            //!< Воздушная скорость, м/c
    double roll;            //!< Крен, радиан
    double pitch;           //!< Тангаж, радиан
    double head;            //!< Курс, радиан
    double head_magn;       //!< Курс магнитный, радиан
    double B;               //!< Широта в WGS-84, радианы
    double L;               //!< Долгота в WGS-84, радианы
    double h;               //!< Высота в WGS-84, метры
    double Wx;              //!< проекция угловой скорости на ось Ox в ССК, радиан/с
    double Wy;              //!< проекция угловой скорости на ось Oy в ССК, радиан/с
    double Wz;              //!< проекция угловой скорости на ось Oz в ССК, радиан/с
    double alfa;            //!< Угол атаки, радиан
    double beta;            //!< Угол скольжения, радиан
    double H_baro;          //!< Барометрическая высота, м
    double Pst;             //!< Статическое давление, Па
    double Tnv;             //!< Температура наружного воздуха °, К
    double Nx;              //!< Продольная перегрузка в ССК, g
    double Ny;              //!< Вертикальная перегрузка в ССК, g
    double Nz;              //!< Боковая перегрузка в ССК, g
    double coarse;          //!< Путевой угол, радианы
    double WindDir;         //!< Направление ветра, радиан
    double WindV;           //!< Скорость ветра, м/c
    uint8_t op_ns;          //!< Номер прошивки ОВ
    uint8_t op_ver;         //!< Версия ОВ
    uint8_t np_ns;          //!< Номер прошивки НВ
    uint8_t np_ver;         //!< Версия НВ
    uint8_t np_num;         //!< Номер НВ
    uint8_t align_time;     //!< Время до окончания выставки или прогрева
    uint8_t nav_mode;       //!< режим работы БИВ
    uint32_t nUDPCounter;   //!< Счетчик UDP пакетов
    uint64_t unix_time;
    uint16_t work_time;     //!< Время наработки БИВ в часах
    uint8_t priority;       //!< Приоритет источника (0 - НАВИГАЦИОННЫЙ, 1 - РЕЗЕРВНЫЙ)
    uint8_t gps_glo;        //!< мл. байт число GPS, старший - ГЛОНАСС
    uint8_t sns_mode;       //!< Режим работы СНС: @sa UNIT_PNI_SNS_MODE
    uint8_t sns_cmd;        //!< Режим работы СНС: @sa UNIT_PNI_SNS_CMD
    uint32_t pni_bits;      //!< Это биты достоверности данных  @sa UNIT_PNI_PNI_BITS
    uint32_t fix_bits;      //!< Признаки исправности устройств @sa UNIT_PNI_FIX_BITS
    uint32_t recv_status;   //!< статус приема данных для unit_pni_process_canmsg и unit_pni_get_data

};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QML_ELEMENT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    Q_INVOKABLE void setMapInfo(double latitude, double longitude, double zoom);



private:
     Ui::MainWindow *ui;
    QGraphicsScene *scene;
    //QQuickWidget* m_quickWidget;
    QString fileName;
    QString fileNameShort;
    int numberOfRows;
    QList<QString> chartTitles;
    QList<QString> chartViewTitles;
    const QString strTableQuery = "SELECT name FROM sqlite_master WHERE type = 'table'";
    const QString strMainTableCreate = "CREATE TABLE MainTable ("
                                       "name        TEXT,"
                                       "description     TEXT,"
                                       "additionDate     TEXT);";
    const QString strMainTableSelect = "SELECT * FROM MainTable";

    QSqlDatabase db;

    QStringList structListVarX;
    QStringList structListVariables;
    QStringList structListFull;
    QStringList structListTitlesX;
    QStringList structListTitlesY;

    QObject *qmlPage;
    QObject *mapPolyline;

    static bool createConnection();

    QQmlEngine *engine;
    QQmlContext *context;

    QString row;
    QString column;
    QString strSelect;
    bool fixAxisXWasChecked;
    bool loadedFileCorrupted = false;



    void connectSignals();

public slots:

    void on_pbAddMap_clicked();
    void updateFilePath();
    void on_pbDBSave_clicked();
    void openTable(QSqlDatabase _db);
    void on_pbSetChart_clicked();
    void on_pbSaveFile_clicked();
    void on_pbDBTableDelete_clicked();
    void on_pbDeleteChart_clicked();

    void on_pbClearMap_clicked();
    void on_sbMapCenterLatValue_valueChanged(double arg1);
    void on_sbMapCenterLonValue_valueChanged(double arg1);
    void on_sbMapZoomLevelValue_valueChanged(double arg1);
    void on_cbMapType_currentIndexChanged(int index);

    void on_sbMapCoordDensityValue_valueChanged(double arg1);
    void on_pbDbTableChangeDescription_clicked();

    int countSelectQueryRows(QSqlQuery*);
    int countInsertQueryRows(QFile *file);

    void setMapPath();
    void clearMapFromPath();
    void onMapDrawingProgressChanged(int progress);
    void onMapDrawingProgressBarVisibilityChanged();
    void onMapCoordCountValueChanged(int coords);


    void onChartDrawingProgressChanged(int progress);
    void onChartDrawingProgressBarVisibilityChanged();
    void onChartPointsValueChanged(int coords);

    void dbError();
    void dbErrorClear();
    void saveDB(int _numberOfRows, QString _fileName, QString _description);
    void onDBProgressChanged(int progress);
    void onDBProgressBarVisibilityChanged();
    void on_cbChartRow_currentIndexChanged(int index);

    void startStatsWindow(bool);

signals:
    void setMapPlugin(QString plugin);
    void setMapCoordinate(double latitude, double longitude);
    void setMapCenter(double latitude, double longitude);
    void setMapCircleCoordinate(double latitude, double longitude);
    void setMapCircleRadius(int radius);
    void setMapCircleBorderColor(QColor bordercolor);
    void setMapCircleColor(QColor color);
    void setMapCircleBorderWidth(int borderwidth);
    void setMapZoomLevel(double zoom);
    void clearMapCoordinates();
    void setMapType(int typeValue);
    void setMapOfflineDirectory(QString directory);
    void setMapHighDPI(bool value);
    void setMapLineColor(QColor color);
    void setMapLineWidth(int width);
    void setMapDrawingProgress(int progress);
    void setMapDrawingProgressDisabled();
    void setMapCoordCountValue(int coords);

    void open(QSqlDatabase);

    void setChartDatabase(QSqlDatabase);
    void setChartTableName(QString);
    void setChartFixXChecked(bool);
    void setChartFixYChecked(bool);
    void setChartDBRow(QString);
    void setChartDBColumn(QString);
    void setChartDBTitleRow(QString);
    void setChartDBTitleColumn(QString);
    void setChartNumberOfRows(int);
    void setChartAODensity(int);
    void setChartPen(QPen);
    void setChartTheme(int);
    void drawChart();
    void clearChart();
    void setChartUseOptimization(bool);
    void setChartUseAltOptimization(bool);
    void setChartDrawingProgress(int);
    void setChartDrawingProgressDisabled();
    void setChartPointsValue(int);

    void dbErrorAppeared();
    void setDBProgress(int);
    void setDBProgressDisabled();

    void openStatsWindow(bool);
    void setStatsWindow();
    void getStatsWindow();
private slots:

    void on_sbChartAODensity_valueChanged(double arg1);
    void on_chbChartUseAltOptimization_stateChanged(int arg1);
    void on_chbChartUseOptimization_stateChanged(int arg1);

    void on_pbMapDrawingColorSelection_clicked();
    void on_sbMapDrawingWidthValue_valueChanged(double arg1);
    void on_pbMapSelectionAccept_clicked();
    void installOSMPlugin();
    void installESRIPlugin();
    void on_pbMapDPISelectionAccept_clicked();
    void on_pbMapCircleBorderColor_clicked();
    void on_sbMapCircleBorderWidth_valueChanged(int arg1);
    void on_sbMapCircleRadius_valueChanged(int arg1);
    void on_pbMapCircleColor_clicked();
    void on_chbMapCirclesDrawing_stateChanged(int arg1);
    void on_tvDatabases_doubleClicked(const QModelIndex &index);
    void on_pbChartPenColorSelection_clicked();
    void on_cbChartTheme_currentIndexChanged(int index);
    void on_sbChartPenWidth_valueChanged(int arg1);
    void on_pbStartStatsWindow_clicked();
};

#endif // MAINWINDOW_H
