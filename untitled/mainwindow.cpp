#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "chart.h"
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
    connectSignals();
    ui->lblChartOptimizationsWarning->setVisible(false);
    ui->gbMapDPISelection->setVisible(false);

    /*--Настройка баз данных--*/


    db = QSqlDatabase::addDatabase("QSQLITE","mainConnection");
    db.setDatabaseName("database.sqlite");
    db.open();

    emit update();
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

    /*--Настройка карты--*/

    ui->qwMap->setSource(QUrl("qrc:/map.qml"));
    ui->qwMap->setResizeMode(QQuickWidget::SizeRootObjectToView);
    engine = ui->qwMap->engine();
    context = engine->rootContext();
    context->setContextProperty(QStringLiteral("main"), this);
    setMapInfo(56.394, 61.9334, 12);
    emit setMapOfflineDirectory(QDir::currentPath() + "/offline_tiles/");

    /*--Настройка прогресс бара--*/

    ui->prbrDBSave->setVisible(false);
    ui->prbrMapDrawing->setVisible(false);
    ui->prbrChartDrawing->setVisible(false);
}

void MainWindow::connectSignals() {
    connect(this, SIGNAL(setMapDrawingProgress(int)), this,
            SLOT(onMapDrawingProgressChanged(int)));
    connect(this, SIGNAL(setMapDrawingProgressDisabled()), this,
            SLOT(onMapDrawingProgressBarVisibilityChanged()));
    connect(this, SIGNAL(setMapCoordCountValue(int)), this,
            SLOT(onMapCoordCountValueChanged(int)));


    connect(this, SIGNAL(setDBProgress(int)), this,
            SLOT(onDBProgressChanged(int)));
    connect(this, SIGNAL(setDBProgressDisabled()), this,
            SLOT(onDBProgressBarVisibilityChanged()));
    connect(this, SIGNAL(update()), this, SLOT(updateTables()));

    connect(this, SIGNAL(open()), this, SLOT(openTable()));

}
MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::clearMapFromPath() {
    emit clearMapCoordinates();
    ui->lblMapCoordCountValue->setText("0");
}

void MainWindow::onMapDrawingProgressChanged(int progress) {
    if (ui->prbrMapDrawing->isVisible() == false) {
        ui->prbrMapDrawing->setVisible(true);
    }
    ui->prbrMapDrawing->setValue(progress);
}

void MainWindow::onMapDrawingProgressBarVisibilityChanged() {
    ui->prbrMapDrawing->setVisible(false);
}

void MainWindow::onMapCoordCountValueChanged(int coords) {
    ui->lblMapCoordCountValue->setText(QString::number(coords));
}

int MainWindow::countSelectQueryRows(QSqlQuery *query) {
    int numberOfRows = 0;

    if (query->last()) {
        numberOfRows = query->at() + 1;
        query->first();
        query->previous();
    }
    emit setChartNumberOfRows(numberOfRows);
    return numberOfRows;
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
    int numberOfRows = countSelectQueryRows(query);

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
                emit setMapDrawingProgress(progress);
                ++trueCoords;
            }
        }
    }
    query->~QSqlQuery();
    _db.close();
    _db.~QSqlDatabase();
    _db.removeDatabase(_db.connectionName());
    emit setMapDrawingProgressDisabled();

    qDebug() << QSqlDatabase::connectionNames();

    emit setMapCenter(lat, lon);
    emit setMapCoordCountValue(trueCoords);
    ui->pbAddMap->setText("Перерисовать маршрут");
}

void MainWindow::on_pbAddMap_clicked() {
    ui->pbAddMap->setEnabled(false);
    QFuture<void> future = QtConcurrent::run(this, &MainWindow::setMapPath);
    future.waitForFinished();
    if (future.isFinished()) {
        ui->pbAddMap->setEnabled(true);
    }
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

void MainWindow::on_cbMapType_currentIndexChanged(int index) {
    emit setMapType(index);
}


void MainWindow::onDBProgressChanged(int progress) {
    if (ui->prbrDBSave->isVisible() == false) {
        ui->prbrDBSave->setVisible(true);
    }
    ui->prbrDBSave->setValue(progress);
}

void MainWindow::onDBProgressBarVisibilityChanged() {
    ui->prbrDBSave->setVisible(false);
}

int MainWindow::countInsertQueryRows(QFile *file) {
    int rows = 0;
    if (file->open(QIODevice::ReadOnly)) {
        QDataStream stream(file);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.setVersion(QDataStream::Qt_5_15);
        while (stream.atEnd() != true && stream.status() == QDataStream::Ok) {
            BpiData tmpData;
            stream.readRawData(tmpData.hdr, sizeof(tmpData.hdr));
            stream >> tmpData.nNavCounter >> tmpData.sns_utc >>
                tmpData.snd_utc >> tmpData.V_X >> tmpData.V_Y >> tmpData.V_Z >>
                tmpData.Vair >> tmpData.roll >> tmpData.pitch >> tmpData.head >>
                tmpData.head_magn >> tmpData.B >> tmpData.L >> tmpData.h >>
                tmpData.Wx >> tmpData.Wy >> tmpData.Wz >> tmpData.alfa >>
                tmpData.beta >> tmpData.H_baro >> tmpData.Pst >> tmpData.Tnv >>
                tmpData.Nx >> tmpData.Ny >> tmpData.Nz >> tmpData.coarse >>
                tmpData.WindDir >> tmpData.WindV >> tmpData.op_ns >>
                tmpData.op_ver >> tmpData.np_ns >> tmpData.np_ver >>
                tmpData.np_num >> tmpData.align_time >> tmpData.nav_mode >>
                tmpData.nUDPCounter >> tmpData.unix_time >> tmpData.work_time >>
                tmpData.priority >> tmpData.gps_glo >> tmpData.sns_mode >>
                tmpData.sns_cmd >> tmpData.pni_bits >> tmpData.fix_bits >>
                tmpData.recv_status;
            ++rows;
        }
    }
    return rows;
}

void MainWindow::saveDB(int _numberOfRows, QString _fileName,
                        QString _description) {

    QFile *file = new QFile(_fileName);
    QString _fileNameShort =
        _fileName.split("/")[fileName.split("/").size() - 1];
    int currentRow = 0;
    QSqlDatabase _db = QSqlDatabase::addDatabase("QSQLITE", "DBSaving");
    _db.setDatabaseName("database.sqlite");
    _db.open();
    QSqlQuery *query = new QSqlQuery(_db);

    QString strSelect = "SELECT * FROM '%1'";
    QString strAddTable = "INSERT INTO MainTable VALUES('%1','%2','%3');";

    strAddTable = strAddTable.arg(_fileNameShort)
                      .arg(_description)
                      .arg(QDateTime::currentDateTime().toString("dd.MM.yyyy"));

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

    strCreate = strCreate.arg(_fileNameShort);
    // strCreate2 = strCreate2.arg(fileNameShort+"_");
    ui->prbrDBSave->setValue(5);
    if (!query->exec(strCreate) /*&& !query2->exec(strCreate2)*/) {
        // qDebug() << !query->exec(strCreate) << "a";
    } else {
        // qDebug() << "insert";
        // qDebug() << strCreate2;
        query->exec(strAddTable);
        //query->numRowsAffected();

        if (file->open(QIODevice::ReadOnly)) {

            QDataStream stream(file);
            stream.setByteOrder(QDataStream::LittleEndian);
            stream.setVersion(QDataStream::Qt_5_15);
            _db.transaction();
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
                ++currentRow;

                int progress = qCeil(100 * currentRow / _numberOfRows);
                emit setDBProgress(progress);

                if (tmpData.pni_bits != 0) {
                    //                    QString strInsert =
                    //                        "INSERT INTO '%1' (nNavCounter,
                    //                        sns_utc, snd_utc, " "V_X, V_Y,
                    //                        V_Z, Vair, roll, pitch, head,
                    //                        head_magn, B, " "L, h, " "Wx, Wy,
                    //                        Wz, alfa, beta, H_baro, Pst, Tnv,
                    //                        Nx, Ny, Nz, " "coarse, " "WindDir,
                    //                        WindV, op_ns, op_ver, np_ns,
                    //                        np_ver, " "np_num, align_time,
                    //                        nav_mode, nUDPCounter,unix_time, "
                    //                        "work_time, priority, gps_glo,
                    //                        sns_mode, sns_cmd, " "pni_bits,
                    //                        fix_bits, recv_status) " "VALUES "
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
                        "('%2','%3','%4','%5','%6','%7','%8','%9','%10','%11','"
                        "%12','%13','%14','%15','%16','%17','%18','%19','%20','"
                        "%21',"
                        "'%22','%23','%24','%25','%26','%27','%28','%29','%30',"
                        "'%31','%32','%33','%34','%35','%36','%37','%38','%39',"
                        "'%40',"
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
                    strInsert =
                        strInsert.arg(_fileNameShort)
                            .arg(QString::number(tmpData.nNavCounter))
                            .arg(QString::number(tmpData.sns_utc))
                            .arg(QString::number(tmpData.snd_utc))
                            .arg(QString::number(tmpData.V_X, 'g', precision))
                            .arg(QString::number(tmpData.V_Y, 'g', precision))
                            .arg(QString::number(tmpData.V_Z, 'g', precision))
                            .arg(QString::number(tmpData.Vair, 'g', precision))
                            .arg(QString::number(tmpData.roll, 'g', precision))
                            .arg(QString::number(tmpData.pitch, 'g', precision))
                            .arg(QString::number(tmpData.head, 'g', precision))
                            .arg(QString::number(tmpData.head_magn, 'g',
                                                 precision))
                            .arg(QString::number(tmpData.B, 'g', precision))
                            .arg(QString::number(tmpData.L, 'g', precision))
                            .arg(QString::number(tmpData.h, 'g', precision))
                            .arg(QString::number(tmpData.Wx, 'g', precision))
                            .arg(QString::number(tmpData.Wy, 'g', precision))
                            .arg(QString::number(tmpData.Wz, 'g', precision))
                            .arg(QString::number(tmpData.alfa, 'g', precision))
                            .arg(QString::number(tmpData.beta, 'g', precision))
                            .arg(
                                QString::number(tmpData.H_baro, 'g', precision))
                            .arg(QString::number(tmpData.Pst, 'g', precision))
                            .arg(QString::number(tmpData.Tnv, 'g', precision))
                            .arg(QString::number(tmpData.Nx, 'g', precision))
                            .arg(QString::number(tmpData.Ny, 'g', precision))
                            .arg(QString::number(tmpData.Nz, 'g', precision))
                            .arg(
                                QString::number(tmpData.coarse, 'g', precision))
                            .arg(QString::number(tmpData.WindDir, 'g',
                                                 precision))
                            .arg(QString::number(tmpData.WindV, 'g', precision))
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
                    //                        qDebug() << "Unable to make insert
                    //                        operation"
                    //                                 << query2->lastError() <<
                    //                                 strInsert2;
                    //                    }
                }
                tmpData.~BpiData();
            }
            file->close();
            stream.~QDataStream();
            file->~QFile();
        }
        _db.commit();
    }
    query->~QSqlQuery();
    _db.close();
    _db.~QSqlDatabase();
    _db.removeDatabase(_db.connectionName());

    emit update();
    emit setDBProgressDisabled();
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
    if (fileName == "") {
        return;
    }
    QFile *file = new QFile(fileName);
    fileNameShort = fileName.split("/")[fileName.split("/").size() - 1];
    dialog->close();

    int numberOfRows =
        QtConcurrent::run(this, &MainWindow::countInsertQueryRows, file)
            .result();
    file->close();
    file->~QFile();
    qDebug() << numberOfRows;

    QtConcurrent::run(this, &MainWindow::saveDB,numberOfRows,fileName,description);


}

void MainWindow::openTable() {

    if (ui->tvSqlTable->model() != nullptr) {
        ui->tvSqlTable->model()->~QAbstractItemModel();
    }

    QSqlQuery *query = new QSqlQuery(db);
    QString strSelect = "SELECT * FROM '%1'";
    strSelect = strSelect.arg(fileNameShort);

    QSqlQueryModel *sqlModel = new QSqlQueryModel;

    if (query->exec(strSelect)) {

        sqlModel->setQuery(*query);

        if (sqlModel->lastError().isValid()) {
            qDebug() << sqlModel->lastError() << "model error";
        } else
        qDebug() << query->lastError() << "query error";
        ui->tvSqlTable->setModel(sqlModel);
    }
    query->~QSqlQuery();
}

void MainWindow::on_pbSetChart_clicked() {
    if (fileNameShort.isEmpty()) {
        return;
    }
    Chart *chart = new Chart();
    connect(this, SIGNAL(drawChart()), chart, SLOT(compute()));
    connect(this, SIGNAL(setChartTableName(QString)), chart,
            SLOT(setTableName(QString)));
    connect(this, SIGNAL(setChartDatabase(QSqlDatabase)), chart,
            SLOT(setDatabase(QSqlDatabase)));
    connect(this, SIGNAL(setChartFixXChecked(bool)), chart,
            SLOT(setFixXChecked(bool)));
    connect(this, SIGNAL(setChartFixYChecked(bool)), chart,
            SLOT(setFixYChecked(bool)));
    connect(this, SIGNAL(setChartFixYChecked(bool)), chart,
            SLOT(setFixYChecked(bool)));
    connect(this, SIGNAL(setChartUseOptimization(bool)), chart,
            SLOT(setUseOptimization(bool)));
    connect(this, SIGNAL(setChartUseAltOptimization(bool)), chart,
            SLOT(setUseAltOptimization(bool)));
    connect(this, SIGNAL(setChartAODensity(int)), chart,
            SLOT(setAODensity(int)));
    connect(this, SIGNAL(setChartDBRow(QString)), chart,
            SLOT(setDBRow(QString)));
    connect(this, SIGNAL(setChartDBColumn(QString)), chart,
            SLOT(setDBColumn(QString)));
    connect(chart, SIGNAL(getNumberOfRows(QSqlQuery *)), this,
            SLOT(countSelectQueryRows(QSqlQuery *)));
    connect(this, SIGNAL(setChartNumberOfRows(int)), chart,
            SLOT(setNumberOfRows(int)));
    connect(chart, SIGNAL(setDrawingProgress(int)), this,
            SLOT(onChartDrawingProgressChanged(int)));
    connect(chart, SIGNAL(setDrawingProgressDisabled()), this,
            SLOT(onChartDrawingProgressBarVisibilityChanged()));
    connect(chart, SIGNAL(setPointsValue(int)), this,
            SLOT(onChartPointsValueChanged(int)));


    QSqlDatabase _db = QSqlDatabase::addDatabase("QSQLITE", "chartConnection");
    _db.setDatabaseName("database.sqlite");
    _db.open();
    emit setChartDatabase(_db);
    emit setChartTableName(fileNameShort);
    emit setChartFixXChecked(ui->chbChartFixAxisX->isChecked());
    emit setChartFixYChecked(ui->chbChartFixAxisY->isChecked());
    emit setChartDBRow(structListVarX.at(ui->cbChartRow->currentIndex()));
    emit setChartDBColumn(
        structListVariables.at(ui->cbChartColumn->currentIndex()));
    emit setChartUseOptimization(ui->chbChartUseOptimization->isChecked());
    emit setChartUseAltOptimization(ui->chbChartUseAltOptimization->isChecked());
    emit setChartAODensity(qCeil(ui->sbChartAODensity->value()));
    emit clearChart();
    emit drawChart();

    ui->chartView->setChart(chart);

    disconnect(this, nullptr, chart, nullptr);
    disconnect(chart, nullptr, this, nullptr);

    connect(this, SIGNAL(clearChart()), chart, SLOT(clearChart()));
    connect(chart, SIGNAL(setPointsValue(int)), this,
            SLOT(onChartPointsValueChanged(int)));
}

void MainWindow::onChartDrawingProgressChanged(int progress) {
    if (ui->prbrChartDrawing->isVisible() == false) {
        ui->prbrChartDrawing->setVisible(true);
    }
    ui->prbrChartDrawing->setValue(progress);
}

void MainWindow::onChartDrawingProgressBarVisibilityChanged() {
    ui->prbrChartDrawing->setVisible(false);
}

void MainWindow::onChartPointsValueChanged(int coords) {
    ui->lblChartPointsValue->setText(QString::number(coords));
}

void MainWindow::on_pbDeleteChart_clicked() { emit clearChart();
}

void MainWindow::on_pbSaveFile_clicked() {
    QSqlQuery *query = new QSqlQuery(db);
    QString strSelect = "SELECT * FROM '%1'";
    strSelect = strSelect.arg(fileNameShort);
    if (!query->exec(strSelect)) {
        //qDebug() << query->lastError();
    }
    //"	" - табуляция
    int i = 0;
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
    qDebug() << "signal received";
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
    //openTable();
    emit open();
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
    emit update();
}

void MainWindow::on_cbChartRow_currentIndexChanged(int index) {
    if (ui->chbChartFixAxisX->isEnabled()) {
        fixAxisXWasChecked = ui->chbChartFixAxisX->isChecked();
    }
    switch (index) {
    case 0:
        ui->chbChartFixAxisX->setEnabled(true);
        ui->chbChartFixAxisX->setChecked(fixAxisXWasChecked);
        break;
    default:
        ui->chbChartFixAxisX->setEnabled(false);
        ui->chbChartFixAxisX->setChecked(false);
        break;
    }
}

void MainWindow::on_pbMapHighDPI_clicked() {
    emit setMapHighDPI(true);
    ui->gbMapDPISelection->setVisible(false);
    ui->pbAddMap->setEnabled(true);
    ui->pbClearMap->setEnabled(true);
    ui->cbMapType->setEnabled(true);
}

void MainWindow::on_pbMapLowDPI_clicked() {
    emit setMapHighDPI(false);
    ui->gbMapDPISelection->setVisible(false);
    ui->pbAddMap->setEnabled(true);
    ui->pbClearMap->setEnabled(true);
    ui->cbMapType->setEnabled(true);
}


void MainWindow::on_sbChartAODensity_valueChanged(double arg1) {
    emit setChartAODensity(qCeil(arg1));
}


void MainWindow::on_chbChartUseAltOptimization_stateChanged(int arg1) {
    ui->sbChartAODensity->setEnabled(arg1);
    if (ui->chbChartUseAltOptimization->isChecked() &&
        ui->chbChartUseOptimization->isChecked()) {
        ui->lblChartOptimizationsWarning->setVisible(true);
    } else {
        ui->lblChartOptimizationsWarning->setVisible(false);
    }
}


void MainWindow::on_chbChartUseOptimization_stateChanged(int arg1)
{
    if (ui->chbChartUseAltOptimization->isChecked() &&
        ui->chbChartUseOptimization->isChecked()) {
        ui->lblChartOptimizationsWarning->setVisible(true);
    } else {
        ui->lblChartOptimizationsWarning->setVisible(false);
    }
}


void MainWindow::on_pbMapOSM_clicked() {
    QStringList list;
    list << "Стандартный"
         << "Велосипед"
         << "Обществ. транспорт"
         << "Ночной транспорт"
         << "Местность"
         << "Туризм";
    emit setMapPlugin("osm");
    ui->gbMapDPISelection->setVisible(true);
    ui->cbMapType->addItems(list);
    ui->gbMapSelection->setVisible(false);
}


void MainWindow::on_pbMapESRI_clicked() {
    QStringList list;
    list << "Стандартный"
         << "Спутник"
         << "Местность"
         << "Топография"
         << "Топография США"
         << "Национальная"
         << "Светло-серый холст"
         << "Физическая"
         << "Затененный рельеф"
         << "Мировой океан"
         << "Темно-серый холст"
         << "Карта Делорм";
    emit setMapPlugin("esri");
    ui->cbMapType->addItems(list);
    ui->gbMapSelection->setVisible(false);
    ui->gbMapDPISelection->setVisible(false);
}

