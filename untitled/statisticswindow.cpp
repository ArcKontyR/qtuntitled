#include "statisticswindow.h"
#include "mainwindow.h"
#include "chart.h"
#include "ui_statisticswindow.h"

#include <QtSql>
#include <QtMath>
StatisticsWindow::StatisticsWindow(QWidget *parent) :
                                                      QDialog(parent),
                                                      ui(new Ui::StatisticsWindow)
{
    ui->setupUi(this);

}

StatisticsWindow::~StatisticsWindow()
{
    delete ui;
}

void StatisticsWindow::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void StatisticsWindow::setFileName(QString _fileName) { fileName = _fileName; }
void StatisticsWindow::setFilePath(QString _filePath) { filePath = _filePath; }
void StatisticsWindow::setFileRowCount(int _rowCount) { rowCount = _rowCount; }
void StatisticsWindow::setFileCorruption(bool _isCorrupted) {
    isCorrupted = _isCorrupted;
}

void StatisticsWindow::getStats() {
    qDebug() << "getting stats";
    QSqlDatabase db =
        QSqlDatabase::addDatabase("QSQLITE", "getStatisticsConnection");
    db.setDatabaseName("databases/" + fileName + ".sqlite");
    db.open();
    qDebug() << db.databaseName();
    QString selectStatTable = "SELECT * FROM StatisticsTable";
    QString selectGeneralTable = "SELECT * FROM StatisticsGeneralTable";
    QString selectErrorTable = "SELECT * FROM StatisticsErrorTable";
    QSqlQuery *statQuery = new QSqlQuery(db);
    statQuery->exec(selectStatTable);
    QStringList headers;
    headers << "Название"
            << "Информация";
    ui->twStatistics->setHeaderLabels(headers);
    statQuery->next();
    QTreeWidgetItem *generalParentItem = new QTreeWidgetItem(ui->twStatistics);
    generalParentItem->setText(0, statQuery->value(0).toString());
    QSqlQuery *generalQuery = new QSqlQuery(db);
    generalQuery->exec(selectGeneralTable);
    while (generalQuery->next()) {
        QTreeWidgetItem *generalChildItem =
            new QTreeWidgetItem(generalParentItem);
        generalChildItem->setText(0, generalQuery->value(0).toString());
        generalChildItem->setText(1, generalQuery->value(1).toString());
    }
    int errorId = 0;
    statQuery->next();
    QTreeWidgetItem *errorsParentItem = new QTreeWidgetItem(ui->twStatistics);
    errorsParentItem->setText(0, statQuery->value(0).toString());
    QSqlQuery *errorsQuery = new QSqlQuery(db);
    errorsQuery->exec(selectErrorTable);
    while (errorsQuery->next()) {
        ++errorId;
        QTreeWidgetItem *errors1GenChildItem =
            new QTreeWidgetItem(errorsParentItem);
        errors1GenChildItem->setText(0, generalQuery->value(1).toString());
        errors1GenChildItem->setText(1, generalQuery->value(2).toString());
        QString selectErrorValuesTable =
            "SELECT * FROM StatisticsErrorValuesTable WHERE errorId == %1";
        QSqlQuery *errors2GenQuery = new QSqlQuery(db);

    }

}

void StatisticsWindow::setStats() {
    qDebug() << "setting stats";
    QSqlDatabase db =
        QSqlDatabase::addDatabase("QSQLITE", "setStatisticsConnection");
    db.setDatabaseName("databases/" + fileName + ".sqlite");
    db.open();
    qDebug() << db.databaseName();

    QSqlQuery *mainQuery = new QSqlQuery(db);
    QString createStatTable =
        "CREATE TABLE StatisticsTable(statName TEXT, statValue TEXT);";
    QString insertStatTableDefaultParameter1 =
        "INSERT INTO StatisticsTable(statName, statValue) VALUES('Общая', '');";
    QString insertStatTableDefaultParameter2 =
        "INSERT INTO StatisticsTable(statName, statValue) VALUES('Ошибки', "
        "'');";
    QString createGeneralTable =
        "CREATE TABLE StatisticsGeneralTable(generalName TEXT, generalValue "
        "TEXT);";
    QString insertGeneralTableParameters =
        "INSERT INTO StatisticsGeneralTable(generalName, generalValue) "
        "VALUES('%1','%2');";
    QString createErrorTable =
        "CREATE TABLE StatisticsErrorTable(errorId INTEGER PRIMARY KEY "
        "NOT NULL, errorName TEXT, errorInfo TEXT);";
    QString createErrorValuesTable =
        "CREATE TABLE StatisticsErrorValuesTable(errorId "
        "INTEGER,errorValueName TEXT, "
        "errorValueInfo "
        "TEXT);";
    mainQuery->exec(createStatTable);
    mainQuery->exec(insertStatTableDefaultParameter1);
    mainQuery->exec(insertStatTableDefaultParameter2);

    mainQuery->exec(createGeneralTable);
    mainQuery->exec(createErrorTable);
    mainQuery->exec(createErrorValuesTable);
    QFile *file = new QFile(filePath);
    file->open(QFile::ReadOnly);
    QDataStream stream(file);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setVersion(QDataStream::Qt_5_15);
    QStringList headers;
    headers << "Название"
            << "Информация";
    ui->twStatistics->setHeaderLabels(headers);
    BpiData prevData;
    BpiData tmpData;
    QTreeWidgetItem *generalParentItem = new QTreeWidgetItem(ui->twStatistics);
    generalParentItem->setText(0, "Общее");
    QTreeWidgetItem *warningsParentItem = new QTreeWidgetItem(ui->twStatistics);
    warningsParentItem->setText(0, "Ошибки");
    int rows = 0;
    QTime flightStarted;
    QTime flightFinished;
    bool isFlightStarted = false;
    int errorId = 0;

    while (stream.atEnd() == false && stream.status() == QDataStream::Ok) {
        ++rows;
        prevData = tmpData;

        stream.readRawData(tmpData.hdr, sizeof(tmpData.hdr));
        stream >> tmpData.nNavCounter >> tmpData.sns_utc >> tmpData.snd_utc >>
            tmpData.V_X >> tmpData.V_Y >> tmpData.V_Z >> tmpData.Vair >>
            tmpData.roll >> tmpData.pitch >> tmpData.head >>
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
        if (tmpData.pni_bits != 0) {
            if (!isFlightStarted) {
                flightStarted = QTime::fromString(
                    QString::number(tmpData.sns_utc), "hmmsszzz");
                isFlightStarted = true;
                prevData = tmpData;
            }
            if (isFlightStarted) {
                flightFinished = QTime::fromString(
                    QString::number(tmpData.sns_utc), "hmmsszzz");
            }

            if ((tmpData.nNavCounter - prevData.nNavCounter) > 1 /*&&
                prevData.nNavCounter < tmpData.nNavCounter && isFlightStarted*/) {
                ++errorId;
                QString insertErrorTableParameters =
                    "INSERT INTO StatisticsErrorTable(errorId, errorName, "
                    "errorInfo) "
                    "VALUES(%1,'%2','%3');";
                QString insertErrorValuesTableParameters =
                    "INSERT INTO StatisticsErrorValuesTable(errorId, "
                    "errorValueName, "
                    "errorValueInfo) "
                    "VALUES(%1,'%2','%3');";
                //                QString insertErrorValuesTableParameter2 =
                //                    "INSERT INTO
                //                    StatisticsErrorValuesTable(errorId, "
                //                    "errorValueName, "
                //                    "errorValueInfo) "
                //                    "VALUES(%1,'%2','%3');";
                QTreeWidgetItem *childItem =
                    new QTreeWidgetItem(warningsParentItem);
                childItem->setText(0, "Пропуск");
                insertErrorTableParameters =
                    insertErrorTableParameters.arg(errorId).arg("Пропуск").arg(
                        "");
                mainQuery->exec(insertErrorTableParameters);
                QTreeWidgetItem *first2GenChildItem =
                    new QTreeWidgetItem(childItem);
                first2GenChildItem->setText(0, "Начало");
                first2GenChildItem->setText(
                    1, QString::number(prevData.nNavCounter));

                QString insertErrorValuesTableParameter1 =
                    insertErrorValuesTableParameters.arg(errorId)
                        .arg("Начало")
                        .arg(QString::number(prevData.nNavCounter));
                mainQuery->exec(insertErrorValuesTableParameter1);

                QTreeWidgetItem *sec2GenChildItem =
                    new QTreeWidgetItem(childItem);
                sec2GenChildItem->setText(0, "Конец");
                sec2GenChildItem->setText(1,
                                          QString::number(tmpData.nNavCounter));

                QString insertErrorValuesTableParameter2 =
                    insertErrorValuesTableParameters.arg(errorId)
                        .arg("Конец")
                        .arg(QString::number(tmpData.nNavCounter));
                mainQuery->exec(insertErrorValuesTableParameter2);
            }
            if (rows == rowCount && isCorrupted) {
                break;
            }
        }
    }

    QTime defaultTime = QTime(0, 0, 0);
    QTime flightTime;
    flightTime = defaultTime.addSecs(flightStarted.secsTo(flightFinished));
    QTreeWidgetItem *childItem = new QTreeWidgetItem(generalParentItem);
    childItem->setText(0, "Время в полете");
    childItem->setText(1, flightTime.toString());
    QString insertGeneralTableParameter =
        insertGeneralTableParameters.arg("Время в полете")
            .arg(flightTime.toString());
    mainQuery->exec(insertGeneralTableParameter);
    db.close();
    db.removeDatabase(db.connectionName());

}



