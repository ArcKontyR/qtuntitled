#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "chart.h"
#include "moveitem.h"
#include "filedialog.h"
#include <QtSql>
#include <QtCharts/QChart>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtMath>
#include <QtConcurrent/QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
      , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->width(), this->height());

    /*--Настройка баз данных--*/

    db = QSqlDatabase::addDatabase("QSQLITE","mainConnection");
    db.setDatabaseName("database.sqlite");

    if (!db.open()) {
        qDebug() << "Cannot open database:" << db.lastError();
    }
    updateTables();
    /*--Установка списков--*/
    structListFull << "nNavCounter"
                   << "sns_utc"
                   << "snd_utc"
                   << "V_X"
                   << "V_Y"
                   << "V_Z"
                   << "Vair"
                   << "roll"
                   << "pitch"
                   << "head"
                   << "head_magn"
                   << "B"
                   << "L"
                   << "h"
                   << "Wx"
                   << "Wy"
                   << "Wz"
                   << "alfa"
                   << "beta"
                   << "H_baro"
                   << "Pst"
                   << "Tnv"
                   << "Nx"
                   << "Ny"
                   << "Nz"
                   << "coarse"
                   << "WindDir"
                   << "WindV"
                   << "op_ns"
                   << "op_ver"
                   << "np_ns"
                   << "np_ver"
                   << "np_num"
                   << "align_time"
                   << "nav_mode"
                   << "nUDPCounter"
                   << "unix_time"
                   << "work_time"
                   << "priority"
                   << "gps_glo"
                   << "sns_mode"
                   << "sns_cmd"
                   << "pni_bits"
                   << "fix_bits"
                   << "recv_status";

    structListVarX << "i"
                   << "sns_utc"
                   << "snd_utc";

    structListVariables << "nNavCounter"
                        << "V_X"
                        << "V_Y"
                        << "V_Z"
                        << "Vair"
                        << "roll"
                        << "pitch"
                        << "head"
                        << "head_magn"
                        << "B"
                        << "L"
                        << "h"
                        << "Wx"
                        << "Wy"
                        << "Wz"
                        << "alfa"
                        << "beta"
                        << "H_baro"
                        << "Pst"
                        << "Tnv"
                        << "Nx"
                        << "Ny"
                        << "Nz"
                        << "coarse"
                        << "WindDir"
                        << "WindV"
                        << "op_ns"
                        << "op_ver"
                        << "np_ns"
                        << "np_ver"
                        << "np_num"
                        << "align_time"
                        << "nav_mode"
                        << "nUDPCounter"
                        << "unix_time"
                        << "work_time"
                        << "priority"
                        << "gps_glo"
                        << "sns_mode"
                        << "sns_cmd"
                        << "pni_bits"
                        << "fix_bits"
                        << "recv_status";

    /*--Настройка карты (старое)--*/

    /*scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setSceneRect(0, 0, ui->gvMap->width(), ui->gvMap->height());

    ui->gvMap->setScene(scene);
    ui->gvMap->setRenderHint(QPainter::Antialiasing);
    ui->gvMap->setCacheMode(QGraphicsView::CacheBackground);
    ui->gvMap->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);*/

    /*--Настройка карты--*/

    ui->qwMap->setSource(QUrl("qrc:/map.qml"));
    //ui->qwMap->resize(ui->qwMap->size());
    ui->qwMap->setResizeMode(QQuickWidget::SizeRootObjectToView);
    engine = ui->qwMap->engine();
    context = engine->rootContext();
    context->setContextProperty(QStringLiteral("main"), this);
    setMapInfo(56.394, 61.9334, 12);

    /*--Настройка графиков--*/

    chart = new Chart();
    chart->setTitle("Blank chart");
    chart->legend()->hide();
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setDropShadowEnabled(false);

    ui->chartView->setRenderHint(QPainter::Antialiasing);
    ui->chartView->setChart(chart);
    ui->chartView->setRubberBand(QChartView::VerticalRubberBand);

    /*--Настройка прогресс бара--*/

    ui->prbrDBSave->setVisible(false);
    ui->prbrMapDrawing->setVisible(false);

    connect(this, SIGNAL(setMapProgress(int)), this,
            SLOT(onMapProgressChanged(int)));
    connect(this, SIGNAL(setMapProgressDisabled()), this,
            SLOT(onMapProgressBarVisibilityChanged()));
    connect(this, SIGNAL(setMapCoordCountValue(int)), this,
            SLOT(onMapCoordCountValueChanged(int)));


}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::clearMapFromPath() {
    emit clearMapCoordinates();
    ui->lblMapCoordCountValue->setText("0");
}

void MainWindow::onMapProgressChanged(int progress) {
    if (ui->prbrMapDrawing->isVisible() == false) {
        ui->prbrMapDrawing->setVisible(true);
    }
    ui->prbrMapDrawing->setValue(progress);
}

void MainWindow::onMapProgressBarVisibilityChanged() {
    ui->prbrMapDrawing->setVisible(false);
}

void MainWindow::onMapCoordCountValueChanged(int coords) {
    ui->lblMapCoordCountValue->setText(QString::number(coords));
}
void MainWindow::setMapPath() {
    clearMapFromPath();

    qDebug() << QSqlDatabase::connectionNames();

    QSqlDatabase _db = QSqlDatabase::addDatabase("QSQLITE", "newConnection");
    _db.setDatabaseName("database.sqlite");
    _db.open();
    QSqlQuery *query = new QSqlQuery(_db);
    strSelect = "SELECT B, L FROM '%1'";
    strSelect = strSelect.arg(fileNameShort);

    qDebug() << QSqlDatabase::connectionNames();

    query->exec(strSelect);
    int numberOfRows = 0;

    if (query->last()) {
        numberOfRows = query->at() + 1;
        query->first();
        query->previous();
    }

    double lat = 0;
    double lon = 0;
    int coords = 0;
    int trueCoords = 0;
    int density = qCeil(ui->sbMapCoordDensityValue->value());
    while (query->next()) {
        lat = qRadiansToDegrees(query->value(0).toDouble());
        lon = qRadiansToDegrees(query->value(1).toDouble());
        ++coords;
        if (!qFuzzyCompare(lat, 0) && !qFuzzyCompare(lon, 0)) {
            if ((coords % density) == 0) {
                emit setMapCoordinate(lat, lon);
                int progress = qCeil(100 * coords / numberOfRows);
                emit setMapProgress(progress);
                ++trueCoords;
            }
        }
    }
    query->~QSqlQuery();
    _db.close();
    _db.~QSqlDatabase();
    _db.removeDatabase(_db.connectionName());
    emit setMapProgressDisabled();
    //_db.removeDatabase("newConnection");

    qDebug() << QSqlDatabase::connectionNames();

    emit setMapCenter(lat, lon);
    emit setMapCoordCountValue(trueCoords);
    ui->pbAddMap->setText("Перерисовать маршрут");
}

void MainWindow::on_pbAddMap_clicked() {
    QtConcurrent::run(this, &MainWindow::setMapPath);
//    scene->clear();
//    MoveItem *item = new MoveItem();
//    item->setPos(0,0);
//    item->setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
//    scene->addItem(item);
//    ui->chartView->chart()->setTitle("");
}

void MainWindow::on_pbClearMap_clicked() {
    clearMapFromPath();
    emit setMapCenter(56.394, 61.9334);
    emit setMapZoomLevel(12);
    ui->pbAddMap->setText("Создать маршрут по данным из базы");
}

void MainWindow::setMapInfo(double latitude, double longitude, double zoom) {
    ui->lblMapCenterLatValue->setText(QString::number(latitude));
    ui->lblMapCenterLonValue->setText(QString::number(longitude));
    ui->lblMapZoomValue->setText(QString::number(zoom));
    ui->sbMapCenterLatValue->setValue(latitude);
    ui->sbMapCenterLonValue->setValue(longitude);
    ui->sbMapZoomLevelValue->setValue(zoom);
}

void MainWindow::on_sbMapCenterLatValue_valueChanged(double arg1) {
    emit setMapCenter(arg1, ui->sbMapCenterLonValue->value());
}

void MainWindow::on_sbMapCenterLonValue_valueChanged(double arg1)
{
    emit setMapCenter(ui->sbMapCenterLatValue->value(),arg1);
}

void MainWindow::on_sbMapZoomLevelValue_valueChanged(double arg1) {
    emit setMapZoomLevel(arg1);
}

void MainWindow::on_comboBox_currentIndexChanged(int index) {
    emit setMapType(index);
}



void MainWindow::on_pbDBSave_clicked() {
    FileDialog *dialog = new FileDialog();
    dialog->show();
    dialog->exec();
    QString description;
    if (dialog->result()) {
        fileName = dialog->selectedFiles().first();
        description = dialog->getDescription();
    } else {
        return;
    }


    qDebug() << fileName;
    if (fileName == "") {
        return;
    }
    QFile *file = new QFile(fileName);
    //QStringList sl = ;
    fileNameShort = fileName.split("/")[fileName.split("/").size() - 1];

    dialog->close();

    ui->prbrDBSave->setVisible(true);
    ui->prbrDBSave->setValue(0);

    QSqlQuery *query = new QSqlQuery(db);
    // QSqlQuery* query2 = new QSqlQuery;

    QString strSelect = "SELECT * FROM '%1'";
    QString strAddTable = "INSERT INTO MainTable VALUES('%1','%2','%3');";

    strAddTable = strAddTable.arg(fileNameShort)
                      .arg(description)
                      .arg(QDateTime::currentDateTime().toString("dd.MM.yyyy"));
    qDebug() << strAddTable;


//    QString strCreate = "CREATE TABLE '%1' ("
//                   "nNavCounter UINT32_T,"
//                   "sns_utc     UINT32_T,"
//                   "snd_utc     UINT32_T,"
//                   "V_X         DOUBLE,"
//                   "V_Y         DOUBLE,"
//                   "V_Z         DOUBLE,"
//                   "Vair        DOUBLE,"
//                   "roll        DOUBLE,"
//                   "pitch       DOUBLE,"
//                   "head        DOUBLE,"
//                   "head_magn   DOUBLE,"
//                   "B           DOUBLE,"
//                   "L           DOUBLE,"
//                   "h           DOUBLE,"
//                   "Wx          DOUBLE,"
//                   "Wy          DOUBLE,"
//                   "Wz          DOUBLE,"
//                   "alfa        DOUBLE,"
//                   "beta        DOUBLE,"
//                   "H_baro      DOUBLE,"
//                   "Pst         DOUBLE,"
//                   "Tnv         DOUBLE,"
//                   "Nx          DOUBLE,"
//                   "Ny          DOUBLE,"
//                   "Nz          DOUBLE,"
//                   "coarse      DOUBLE,"
//                   "WindDir     DOUBLE,"
//                   "WindV       DOUBLE,"
//                   "op_ns       UINT8_T,"
//                   "op_ver      UINT8_T,"
//                   "np_ns       UINT8_T,"
//                   "np_ver      UINT8_T,"
//                   "np_num      UINT8_T,"
//                   "align_time  UINT8_T,"
//                   "nav_mode    UINT8_T,"
//                   "nUDPCounter UINT32_T,"
//                   "unix_time   UINT64_T,"
//                   "work_time   UINT16_T,"
//                   "priority    UINT8_T,"
//                   "gps_glo     UINT8_T,"
//                   "sns_mode    UINT8_T,"
//                   "sns_cmd     UINT8_T,"
//                   "pni_bits    UINT32_T,"
//                   "fix_bits    UINT32_T,"
//                   "recv_status UINT32_T"
//                   ");";
    QString strCreate = "CREATE TABLE '%1' ("
                        "nNavCounter TEXT,"
                        "sns_utc     TEXT,"
                        "snd_utc     TEXT,"
                        "V_X         TEXT,"
                        "V_Y         TEXT,"
                        "V_Z         TEXT,"
                        "Vair        TEXT,"
                        "roll        TEXT,"
                        "pitch       TEXT,"
                        "head        TEXT,"
                        "head_magn   TEXT,"
                        "B           TEXT,"
                        "L           TEXT,"
                        "h           TEXT,"
                        "Wx          TEXT,"
                        "Wy          TEXT,"
                        "Wz          TEXT,"
                        "alfa        TEXT,"
                        "beta        TEXT,"
                        "H_baro      TEXT,"
                        "Pst         TEXT,"
                        "Tnv         TEXT,"
                        "Nx          TEXT,"
                        "Ny          TEXT,"
                        "Nz          TEXT,"
                        "coarse      TEXT,"
                        "WindDir     TEXT,"
                        "WindV       TEXT,"
                        "op_ns       TEXT,"
                        "op_ver      TEXT,"
                        "np_ns       TEXT,"
                        "np_ver      TEXT,"
                        "np_num      TEXT,"
                        "align_time  TEXT,"
                        "nav_mode    TEXT,"
                        "nUDPCounter TEXT,"
                        "unix_time   TEXT,"
                        "work_time   TEXT,"
                        "priority    TEXT,"
                        "gps_glo     TEXT,"
                        "sns_mode    TEXT,"
                        "sns_cmd     TEXT,"
                        "pni_bits    TEXT,"
                        "fix_bits    TEXT,"
                        "recv_status TEXT"
                        ");";

    strCreate = strCreate.arg(fileNameShort);
    //strCreate2 = strCreate2.arg(fileNameShort+"_");
    ui->prbrDBSave->setValue(5);
    if (!query->exec(strCreate) //&& !query2->exec(strCreate2)
        ) {
        //qDebug() << !query->exec(strCreate) << "a";
    } else {
        //qDebug() << "insert";
        //qDebug() << strCreate2;
        query->exec(strAddTable);
        ui->prbrDBSave->setValue(10);
        if (file->open(QIODevice::ReadOnly)) {
            QDataStream stream(file);
            stream.setByteOrder(QDataStream::LittleEndian);
            stream.setVersion(QDataStream::Qt_5_15);
            ui->prbrDBSave->setValue(15);
            db.transaction();
            while (stream.atEnd() != true &&
                   stream.status() == QDataStream::Ok) {
                BpiData tmpData;
                stream.readRawData(tmpData.hdr, sizeof(tmpData.hdr));
                stream >> tmpData.nNavCounter >> tmpData.sns_utc >>
                    tmpData.snd_utc >> tmpData.V_X >> tmpData.V_Y >>
                    tmpData.V_Z >> tmpData.Vair >> tmpData.roll >>
                    tmpData.pitch >> tmpData.head >> tmpData.head_magn >>
                    tmpData.B >> tmpData.L >> tmpData.h >> tmpData.Wx >>
                    tmpData.Wy >> tmpData.Wz >> tmpData.alfa >> tmpData.beta >>
                    tmpData.H_baro >> tmpData.Pst >> tmpData.Tnv >>
                    tmpData.Nx >> tmpData.Ny >> tmpData.Nz >> tmpData.coarse >>
                    tmpData.WindDir >> tmpData.WindV >> tmpData.op_ns >>
                    tmpData.op_ver >> tmpData.np_ns >> tmpData.np_ver >>
                    tmpData.np_num >> tmpData.align_time >> tmpData.nav_mode >>
                    tmpData.nUDPCounter >> tmpData.unix_time >>
                    tmpData.work_time >> tmpData.priority >> tmpData.gps_glo >>
                    tmpData.sns_mode >> tmpData.sns_cmd >> tmpData.pni_bits >>
                    tmpData.fix_bits >> tmpData.recv_status;

                if (tmpData.pni_bits != 0){
//                    QString strInsert =
//                        "INSERT INTO '%1' (nNavCounter, sns_utc, snd_utc, "
//                        "V_X, V_Y, V_Z, Vair, roll, pitch, head, head_magn, B, "
//                        "L, h, "
//                        "Wx, Wy, Wz, alfa, beta, H_baro, Pst, Tnv, Nx, Ny, Nz, "
//                        "coarse, "
//                        "WindDir, WindV, op_ns, op_ver, np_ns, np_ver, "
//                        "np_num, align_time, nav_mode, nUDPCounter,unix_time, "
//                        "work_time, priority, gps_glo, sns_mode, sns_cmd, "
//                        "pni_bits, fix_bits, recv_status) "
//                        "VALUES "
//                        "(%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%16,%17,%18,%19,%20,%21,"
//                        "%22,%23,%24,%25,%26,%27,%28,%29,%30,%31,%32,%33,%34,%35,%36,%37,%38,%39,%40,%41,%42,%43,%44,%45,%46);";
                    QString strInsert =
                        "INSERT INTO '%1' (nNavCounter, sns_utc, snd_utc, "
                        "V_X, V_Y, V_Z, Vair, roll, pitch, head, head_magn, B, "
                        "L, h, "
                        "Wx, Wy, Wz, alfa, beta, H_baro, Pst, Tnv, Nx, Ny, Nz, "
                        "coarse, "
                        "WindDir, WindV, op_ns, op_ver, np_ns, np_ver, "
                        "np_num, align_time, nav_mode, nUDPCounter,unix_time, "
                        "work_time, priority, gps_glo, sns_mode, sns_cmd, "
                        "pni_bits, fix_bits, recv_status) "
                        "VALUES "
                        "('%2','%3','%4','%5','%6','%7','%8','%9','%10','%11','%12','%13','%14','%15','%16','%17','%18','%19','%20','%21',"
                        "'%22','%23','%24','%25','%26','%27','%28','%29','%30','%31','%32','%33','%34','%35','%36','%37','%38','%39','%40',"
                        "'%41','%42','%43','%44','%45','%46');";

                    //                    strInsert =
                    //                    strInsert.arg(fileNameShort)
                    //                                    .arg(tmpData.nNavCounter)
                    //                                    .arg(tmpData.sns_utc)
                    //                                    .arg(tmpData.snd_utc)
                    //                                    .arg(tmpData.V_X)
                    //                                    .arg(tmpData.V_Y)
                    //                                    .arg(tmpData.V_Z)
                    //                                    .arg(tmpData.Vair)
                    //                                    .arg(tmpData.roll)
                    //                                    .arg(tmpData.pitch)
                    //                                    .arg(tmpData.head)
                    //                                    .arg(tmpData.head_magn)
                    //                                    .arg(tmpData.B)
                    //                                    .arg(tmpData.L)
                    //                                    .arg(tmpData.h)
                    //                                    .arg(tmpData.Wx)
                    //                                    .arg(tmpData.Wy)
                    //                                    .arg(tmpData.Wz)
                    //                                    .arg(tmpData.alfa)
                    //                                    .arg(tmpData.beta)
                    //                                    .arg(tmpData.H_baro)
                    //                                    .arg(tmpData.Pst)
                    //                                    .arg(tmpData.Tnv)
                    //                                    .arg(tmpData.Nx)
                    //                                    .arg(tmpData.Ny)
                    //                                    .arg(tmpData.Nz)
                    //                                    .arg(tmpData.coarse)
                    //                                    .arg(tmpData.WindDir)
                    //                                    .arg(tmpData.WindV)
                    //                                    .arg(tmpData.op_ns)
                    //                                    .arg(tmpData.op_ver)
                    //                                    .arg(tmpData.np_ns)
                    //                                    .arg(tmpData.np_ver)
                    //                                    .arg(tmpData.np_num)
                    //                                    .arg(tmpData.align_time)
                    //                                    .arg(tmpData.nav_mode)
                    //                                    .arg(tmpData.nUDPCounter)
                    //                                    .arg(tmpData.unix_time)
                    //                                    .arg(tmpData.work_time)
                    //                                    .arg(tmpData.priority)
                    //                                    .arg(tmpData.gps_glo)
                    //                                    .arg(tmpData.sns_mode)
                    //                                    .arg(tmpData.sns_cmd)
                    //                                    .arg(tmpData.pni_bits)
                    //                                    .arg(tmpData.fix_bits)
                    //                                    .arg(tmpData.recv_status);
                    int precision = 16;
                    strInsert = strInsert.arg(fileNameShort)
                                     .arg(QString::number(tmpData.nNavCounter))
                                    .arg(QString::number(tmpData.sns_utc))
                                    .arg(QString::number(tmpData.snd_utc))
                                    .arg(QString::number(tmpData.V_X,'g',precision))
                                    .arg(QString::number(tmpData.V_Y,'g',precision))
                                    .arg(QString::number(tmpData.V_Z,'g',precision))
                                    .arg(QString::number(tmpData.Vair,'g',precision))
                                    .arg(QString::number(tmpData.roll,'g',precision))
                                    .arg(QString::number(tmpData.pitch,'g',precision))
                                    .arg(QString::number(tmpData.head,'g',precision))
                                    .arg(QString::number(tmpData.head_magn,'g',precision))
                                    .arg(QString::number(tmpData.B,'g',precision))
                                    .arg(QString::number(tmpData.L,'g',precision))
                                    .arg(QString::number(tmpData.h,'g',precision))
                                    .arg(QString::number(tmpData.Wx,'g',precision))
                                    .arg(QString::number(tmpData.Wy,'g',precision))
                                    .arg(QString::number(tmpData.Wz,'g',precision))
                                    .arg(QString::number(tmpData.alfa,'g',precision))
                                    .arg(QString::number(tmpData.beta,'g',precision))
                                    .arg(QString::number(tmpData.H_baro,'g',precision))
                                    .arg(QString::number(tmpData.Pst,'g',precision))
                                    .arg(QString::number(tmpData.Tnv,'g',precision))
                                    .arg(QString::number(tmpData.Nx,'g',precision))
                                    .arg(QString::number(tmpData.Ny,'g',precision))
                                    .arg(QString::number(tmpData.Nz,'g',precision))
                                    .arg(QString::number(tmpData.coarse,'g',precision))
                                    .arg(QString::number(tmpData.WindDir,'g',precision))
                                    .arg(QString::number(tmpData.WindV,'g',precision))
                                    .arg(QString::number(tmpData.op_ns))
                                    .arg(QString::number(tmpData.op_ver))
                                    .arg(QString::number(tmpData.np_ns))
                                    .arg(QString::number(tmpData.np_ver))
                                    .arg(QString::number(tmpData.np_num))
                                    .arg(QString::number(tmpData.align_time))
                                    .arg(QString::number(tmpData.nav_mode))
                                    .arg(QString::number(tmpData.nUDPCounter))
                                    .arg(QString::number(tmpData.unix_time))
                                    .arg(QString::number(tmpData.work_time))
                                    .arg(QString::number(tmpData.priority))
                                    .arg(QString::number(tmpData.gps_glo))
                                    .arg(QString::number(tmpData.sns_mode))
                                    .arg(QString::number(tmpData.sns_cmd))
                                    .arg(QString::number(tmpData.pni_bits))
                                    .arg(QString::number(tmpData.fix_bits))
                                    .arg(QString::number(tmpData.recv_status));
                    if (!query->exec(strInsert)) {
                        qDebug() << "Unable to make insert operation"
                                 << query->lastError() << strInsert;
                    }
//                    if (!query2->exec(strInsert2)) {
//                        qDebug() << "Unable to make insert operation"
//                                 << query2->lastError() << strInsert2;
//                    }
                }
                tmpData.~BpiData();
            }
            file->close();
            stream.~QDataStream();
            file->~QFile();
        }
        db.commit();

    }
    query->~QSqlQuery();
    ui->prbrDBSave->setValue(99);
    updateTables();
    ui->prbrDBSave->setValue(100);
    QTimer::singleShot(1000, this, SLOT(progressBarDisable()));
}

void MainWindow::progressBarDisable() {
    ui->prbrDBSave->setVisible(false);
}

void MainWindow::openTable() {

    if (ui->tvSqlTable->model() != nullptr) {
        ui->tvSqlTable->model()->~QAbstractItemModel();
    }

    QSqlQuery *query = new QSqlQuery(db);
    QString strSelect = "SELECT *"
                        "FROM '%1'";
    strSelect = strSelect.arg(fileNameShort);

    QSqlQueryModel *sqlModel = new QSqlQueryModel;
//    for (int o = 0; o < structListFull.size(); ++o) {
//        sqlModel->setHeaderData(o, Qt::Horizontal, structListFull.at(o));
//    }

    if (query->exec(strSelect)) {

        sqlModel->setQuery(*query);

        if (sqlModel->lastError().isValid()) {
            qDebug() << sqlModel->lastError() << "model error";
        }
        qDebug() << query->lastError() << "query error";
        ui->tvSqlTable->setModel(sqlModel);
    }
    query->~QSqlQuery();
}

//Кнопка создания графика

void MainWindow::on_pbSetChart_clicked() {

    QSqlQuery *query = new QSqlQuery(db);
    row = "";
    column = "";
    strSelect = "";
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
    isIndex = false;
    i = 0;
    isIncrement = true;
    shouldAppend = true;
    m_yLim = 0;
    m_xMin = 0;
    isYMinZero = true;
    //bool xEquals = false;
    int points = 0;
    on_pbDeleteChart_clicked();

    row = structListVarX.at(ui->cbChartRow->currentIndex());
    column = structListVariables.at(ui->cbChartColumn->currentIndex());
    if (row == "i") {
        strSelect = "SELECT %1 FROM '%2'";
        strSelect = strSelect.arg(column).arg(fileNameShort);
    } else {
        strSelect = "SELECT %1, %2 FROM '%3'";
        strSelect = strSelect.arg(row).arg(column).arg(fileNameShort);
    }

    QPen pen(Qt::black);
    pen.setWidth(1);
    m_series->setPen(pen);

    query->exec(strSelect);
    while (query->next()) {
        if (query->value(1).toString() == "") {
            isIndex = true;
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
        }

        if (!qFuzzyCompare(m_y,0) && isYMinZero) {
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
                m_series->append(m_x, m_yLim);
                points++;
                shouldAppend = false;
            } else {
                m_series->append(m_dx.toMSecsSinceEpoch(), m_yLim);
                points++;
                //qDebug() << m_dx.toMSecsSinceEpoch() << m_y;
                shouldAppend = false;
            }
        }
    }

    query->~QSqlQuery();

    if (isIndex) {
        m_series->append(m_x, m_yLim);
        points++;
        shouldAppend = false;
    } else {
        m_series->append(m_dx.toMSecsSinceEpoch(), m_y);
        points++;
        qDebug() << m_dx.toMSecsSinceEpoch() << m_y;
        shouldAppend = false;
    }

    ui->lblChartPointsValue->setText(QString::number(points));
    chart->addSeries(m_series);
    if (!isIndex) {
        chart->addAxis(m_daxisX, Qt::AlignBottom);
        m_daxisX->setTickCount(10);
        QDateTime dxmin, dxmax;
        int msmin = QString::number(xmin).right(3).toInt();
        int smin = QString::number(xmin).right(5).left(2).toInt();
        int mmin = QString::number(xmin).right(7).left(2).toInt();
        int hmin = QString::number(xmin).right(9).left(2).toInt();
        dxmin.setTime(QTime(hmin, mmin, smin, msmin));
        int msmax = QString::number(xmax).right(3).toInt();
        int smax = QString::number(xmax).right(5).left(2).toInt();
        int mmax = QString::number(xmax).right(7).left(2).toInt();
        int hmax = QString::number(xmax).right(9).left(2).toInt();
        dxmax.setTime(QTime(hmax, mmax, smax, msmax));
        m_daxisX->setFormat("hh:mm:ss.zzz");
        m_daxisX->setRange(dxmin, dxmax);

        m_series->attachAxis(m_daxisX);
    } else {
        chart->addAxis(m_axisX, Qt::AlignBottom);
        m_axisX->setTickCount(10);
        m_axisX->setRange(xmin, xmax);

        m_series->attachAxis(m_axisX);
    }
    chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisY);
    m_axisY->setTickCount(5);
    m_axisY->setRange(ymin, ymax);
    chart->setTitle(row + "  " + column + " chart");

}

void MainWindow::on_pbSaveFile_clicked() {
    QSqlQuery *query = new QSqlQuery(db);
    QString strSelect = "SELECT * FROM '%1'";
    strSelect = strSelect.arg(fileNameShort);
    if (!query->exec(strSelect)) {
        //qDebug() << query->lastError();
    }
    //"	" - табуляция
    i = 0;
    QString fileName =
        QDateTime::currentDateTime().date().toString(Qt::ISODate) + "_" +
        QString::number(QTime::currentTime().hour()) + "-" +
        QString::number(QTime::currentTime().minute()) + "-" +
        QString::number(QTime::currentTime().second()) + ".txt";
    QFile *file = new QFile(fileName);
    if (file->open(QIODevice::WriteOnly)) {
        QTextStream stream(file);
        stream << "	" << "NAV="  << "	"
               << "nNavCounter" << "	"
               << "sns_utc" << "	"
               << "snd_utc" << "	"
               << "V_X" << "	"
               << "V_Y" << "	"
               << "V_Z" << "	"
               << "Vair" << "	"
               << "roll" << "	"
               << "pitch" << "	"
               << "head" << "	"
               << "head_magn" << "	"
               << "B" << "	"
               << "L" << "	"
               << "h" << "	"
               << "Wx" << "	"
               << "Wy" << "	"
               << "Wz" << "	"
               << "alfa" << "	"
               << "beta" << "	"
               << "H_baro" << "	"
               << "Pst" << "	"
               << "Tnv" << "	"
               << "Nx" << "	"
               << "Ny" << "	"
               << "Nz" << "	"
               << "coarse" << "	"
               << "WindDir" << "	"
               << "WindV" << "	"
               << "op_ns" << "	"
               << "op_ver" << "	"
               << "np_ns" << "	"
               << "np_ver" << "	"
               << "np_num" << "	"
               << "align_time" << "	"
               << "nav_mode" << "	"
               << "nUDPCounter" << "	"
               << "unix_time" << "	"
               << "work_time" << "	"
               << "priority" << "	"
               << "gps_glo" << "	"
               << "sns_mode" << "	"
               << "sns_cmd" << "	"
               << "pni_bits" << "	"
               << "fix_bits" << "	"
               << "recv_status" << "\r\n"
               ;

        while (query->next()) {
            ++i;
            stream << i << "	"
                   << "NAV=" << "	"
                   << query->value(0).toString() << "	"
                   << query->value(1).toString() << "	" << "	"
                   << query->value(2).toString() << "	"
                   << query->value(3).toString() << "	"
                   << query->value(4).toString() << "	"
                   << query->value(5).toString() << "	"
                   << query->value(6).toString() << "	"
                   << query->value(7).toString() << "	"
                   << query->value(8).toString() << "	"
                   << query->value(9).toString() << "	"
                   << query->value(10).toString() << "	"
                   << query->value(11).toString() << "	"
                   << query->value(12).toString() << "	"
                   << query->value(13).toString() << "	"
                   << query->value(14).toString() << "	"
                   << query->value(15).toString() << "	"
                   << query->value(16).toString() << "	"
                   << query->value(17).toString() << "	"
                   << query->value(18).toString() << "	"
                   << query->value(19).toString() << "	"
                   << query->value(20).toString() << "	"
                   << query->value(21).toString() << "	"
                   << query->value(22).toString() << "	"
                   << query->value(23).toString() << "	"
                   << query->value(24).toString() << "	"
                   << query->value(25).toString() << "	"
                   << query->value(26).toString() << "	"
                   << query->value(27).toString() << "	"
                   << query->value(28).toString() << "	"
                   << query->value(29).toString() << "	"
                   << query->value(30).toString() << "	"
                   << query->value(31).toString() << "	"
                   << query->value(32).toString() << "	"
                   << query->value(33).toString() << "	"
                   << query->value(34).toString() << "	"
                   << query->value(35).toString() << "	"
                   << query->value(36).toString() << "	"
                   << query->value(37).toString() << "	"
                   << query->value(38).toString() << "	"
                   << query->value(39).toString() << "	"
                   << query->value(40).toString() << "	"
                   << query->value(41).toString() << "	"
                   << query->value(42).toString() << "	"
                   << query->value(43).toString() << "	"
                   << query->value(44).toString() << "\r\n";
        }
        file->close();
    }
    query->clear();
    //qDebug() << fileName;
}


void MainWindow::on_pbDBTableDelete_clicked() {
    if (ui->tvSqlTable->model() != nullptr) {
        ui->tvSqlTable->model()->~QAbstractItemModel();
    }
    QSqlQuery *query = new QSqlQuery(db);
    QString strDropTable = "DROP TABLE '%1'";
    QString strDeleteTable = "DELETE FROM MainTable WHERE name = '%1'";

    strDropTable = strDropTable.arg(fileNameShort);
    strDeleteTable = strDeleteTable.arg(fileNameShort);
    query->exec(strDropTable);
    query->exec(strDeleteTable);
    updateTables();
    // ui->tvSqlTable
    query->~QSqlQuery();

}

void MainWindow::updateTables() {
    if (ui->tvSqlTable_Tables->model() != nullptr) {
        ui->tvSqlTable_Tables->model()->~QAbstractItemModel();
    }
    if (ui->tvSqlTable->model() != nullptr) {
        ui->tvSqlTable->model()->~QAbstractItemModel();
    }

    QSqlQuery *query = new QSqlQuery(db);
    QSqlQueryModel *sqlModel = new QSqlQueryModel;

    if (query->exec(strMainTableSelect)) {
        sqlModel->setQuery(*query);
        if (sqlModel->lastError().isValid()) {
            qDebug() << sqlModel->lastError() << "model error";
        }
        // qDebug() << query->lastError() << "query error";

    } else {
        query->exec(strMainTableCreate);
        query->exec(strMainTableSelect);
    }
    if (query->isValid()) {
        fileNameShort =
            ui->tvSqlTable_Tables->model()->index(0, 0).data().toString();
        ui->lblSqlTableSelectedValue->setText(fileNameShort);
        ui->lblMapSelectedTableValue->setText(fileNameShort);
        ui->teDBTableDescription->setText(
            ui->tvSqlTable_Tables->model()->index(0, 1).data().toString());
        ui->pbDbTableChangeDescription->setEnabled(true);
    } else {
        fileNameShort = "";
        ui->lblSqlTableSelectedValue->setText("");
        ui->lblMapSelectedTableValue->setText("");
        ui->teDBTableDescription->setText("");
        ui->pbDbTableChangeDescription->setEnabled(false);
    }
    ui->tvSqlTable_Tables->setModel(sqlModel);
    query->clear();

    qDebug() << query << "3";
    query->~QSqlQuery();
}


void MainWindow::on_pbDeleteChart_clicked() {
    if (!chart->axes(Qt::Horizontal).isEmpty() &&
        !chart->axes(Qt::Vertical).isEmpty() && !chart->series().isEmpty()) {
        while (!chart->axes(Qt::Horizontal).isEmpty()) {
            chart->removeAxis(chart->axes(Qt::Horizontal).first());
        }
        while (!chart->axes(Qt::Vertical).isEmpty()) {
            chart->removeAxis(chart->axes(Qt::Vertical).first());
        }
        chart->setTitle("There's nothing left");
        chart->removeAllSeries();
    }
}

void MainWindow::on_tvSqlTable_Tables_doubleClicked(const QModelIndex &index) {
    if (ui->tvSqlTable->model() != nullptr) {
        ui->tvSqlTable->model()->~QAbstractItemModel();
    }
    ui->tvSqlTable_Tables->indexAt(QPoint(index.row(), 0));
    fileNameShort = index.siblingAtColumn(0).data().toString();
    ui->lblSqlTableSelectedValue->setText(fileNameShort);
    ui->lblMapSelectedTableValue->setText(fileNameShort);
    ui->teDBTableDescription->setText(index.siblingAtColumn(1).data().toString());
    ui->pbDbTableChangeDescription->setEnabled(true);
    openTable();
}


void MainWindow::on_sbMapCoordDensityValue_valueChanged(double arg1) {
    if (arg1 >= 101 && arg1 <= 500) {
        ui->lblMapCoordDensityWarning->setText(
            "Для лучшего качества уменьшите значение параметра");
        ui->lblMapCoordDensityWarning->setStyleSheet("color: orange; font: 10pt \"Century Gothic\";");

    }else
        if(arg1 >= 501 && arg1 <=1000){
        ui->lblMapCoordDensityWarning->setText("Рекомендуется уменьшить значение параметра");
        ui->lblMapCoordDensityWarning->setStyleSheet("color: red; font: 10pt \"Century Gothic\";");
    } else
        if(arg1>=1001) {
        ui->lblMapCoordDensityWarning->setText("Веротна возможность неверного отображения");
        ui->lblMapCoordDensityWarning->setStyleSheet("color: darkred; font: 10pt \"Century Gothic\";");
    } else{
        ui->lblMapCoordDensityWarning->setText("");

    }
}


void MainWindow::on_pbDbTableChangeDescription_clicked() {
    QString strUpdate =
        "UPDATE MainTable SET description = '%1' WHERE name == '%2'";
    strUpdate = strUpdate.arg(ui->teDBTableDescription->toPlainText())
                    .arg(fileNameShort);
    QSqlQuery *query = new QSqlQuery(db);
    query->exec(strUpdate);
    query->~QSqlQuery();
    updateTables();
}

