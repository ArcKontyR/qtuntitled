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
    setFont(QFont("Century Gothic",10));
    ui->lblErrorValue->setVisible(false);
    ui->lblError->setVisible(false);
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
    ui->lblFileNameValue->setText(fileName);
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
        setTreeWidgetItem(generalParentItem, generalQuery->value(0).toString(),
                          generalQuery->value(1).toString());
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
        errors1GenChildItem->setText(0, errorsQuery->value(1).toString());
        errors1GenChildItem->setText(1, errorsQuery->value(2).toString());

        if (errorsQuery->value(1).toString() == "Файл поврежден") {
            ui->lblStatusValue->setText("Ошибка записи");
            ui->lblStatusValue->setStyleSheet(
                "color: red; font: 10pt \"Century Gothic\";");
            ui->lblErrorValue->setText(errorsQuery->value(1).toString());
            ui->lblErrorValue->setStyleSheet(
                "color: red; font: 10pt \"Century Gothic\";");
            ui->lblError->setVisible(true);
            ui->lblErrorValue->setVisible(true);
        } else{
            ui->lblStatusValue->setText("Успешно");
            ui->lblStatusValue->setStyleSheet(
                "color: green; font: 10pt \"Century Gothic\";");
        }

        QString selectErrorValuesTable =
            "SELECT * FROM StatisticsErrorValuesTable WHERE errorId == %1";
        selectErrorValuesTable = selectErrorValuesTable.arg(errorId);
        QSqlQuery *errors2GenQuery = new QSqlQuery(db);
        errors2GenQuery->exec(selectErrorValuesTable);
        while (errors2GenQuery->next()) {
            setTreeWidgetItem(errors1GenChildItem, errors2GenQuery->value(1).toString(),
                              errors2GenQuery->value(2).toString());
        }
        errors2GenQuery->~QSqlQuery();
    }
    errorsQuery->~QSqlQuery();
    generalQuery->~QSqlQuery();
    statQuery->~QSqlQuery();
}

void StatisticsWindow::setStats() {
    qDebug() << "setting stats";
    QSqlDatabase db =
        QSqlDatabase::addDatabase("QSQLITE", "setStatisticsConnection");
    db.setDatabaseName("databases/" + fileName + ".sqlite");
    db.open();
    ui->lblFileNameValue->setText(fileName);
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
    QString insertErrorTableParameters =
        "INSERT INTO StatisticsErrorTable(errorId, errorName, "
        "errorInfo) "
        "VALUES(%1,'%2','%3');";
    QString insertErrorValuesTableParameters =
        "INSERT INTO StatisticsErrorValuesTable(errorId, "
        "errorValueName, "
        "errorValueInfo) "
        "VALUES(%1,'%2','%3');";
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
    int firstEmptyRow = 0;
    int emptyRows = 0;
    int firstEmptySNSRow = 0;
    int emptySNSs = 0;
    QTime flightStarted;
    QTime flightFinished;
    bool isFlightStarted = false;
    int errorId = 0;
    qreal maxAltitude = 0;
    qreal minAltitude = 0;

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
            /*--Подсчет временных пропусков--*/
            if (tmpData.sns_utc == 0) {
                ++emptySNSs;
                if (firstEmptySNSRow == 0) {
                    firstEmptySNSRow = rows;
                }
            }

            /*--Установка значений высоты--*/
            if (qFuzzyCompare(minAltitude, 0) && !qFuzzyCompare(tmpData.h, 0)) {
                minAltitude = tmpData.h;
            }
            maxAltitude = qMax(maxAltitude, tmpData.h);
            minAltitude = qMin(minAltitude, tmpData.h);
            /*--Проверка на пропуски--*/
            if ((tmpData.nNavCounter - prevData.nNavCounter) > 1 &&
                prevData.nNavCounter < tmpData.nNavCounter && isFlightStarted) {
                ++errorId;

                QTreeWidgetItem *childItem =
                    new QTreeWidgetItem(warningsParentItem);
                childItem->setText(0, "Пропуск");
                setItemToDatabase(
                    mainQuery,
                    insertErrorTableParameters.arg(errorId).arg("Пропуск").arg(
                        ""));
                setTreeWidgetItem(childItem, "Номер записи начала",
                                  QString::number(rows - 1));
                setItemToDatabase(mainQuery,
                                  insertErrorValuesTableParameters.arg(errorId)
                                      .arg("Номер записи начала")
                                      .arg(QString::number(rows - 1)));
                setTreeWidgetItem(childItem, "Начало",
                                  QString::number(prevData.nNavCounter));
                setItemToDatabase(
                    mainQuery,
                    insertErrorValuesTableParameters.arg(errorId)
                        .arg("Начало")
                        .arg(QString::number(prevData.nNavCounter)));

                setTreeWidgetItem(childItem, "Конец",
                                  QString::number(tmpData.nNavCounter));
                setItemToDatabase(
                    mainQuery,
                    insertErrorValuesTableParameters.arg(errorId)
                        .arg("Конец")
                        .arg(QString::number(tmpData.nNavCounter)));
            }
            /*--Остановка цикла при поврежденном файле--*/
            if (rows == rowCount) {
                break;
            }
        } else {
            /*--Подсчет пустых записей--*/
            if (firstEmptyRow == 0) {
                firstEmptyRow = rows;
            }
            ++emptyRows;
        }
    }
    /*--Проверка на поврежденность--*/
    if (isCorrupted) {
        ++errorId;
        QTreeWidgetItem *childItem = new QTreeWidgetItem(warningsParentItem);
        childItem->setText(0, "Файл поврежден");
        setItemToDatabase(mainQuery,
                          insertErrorTableParameters.arg(errorId)
                              .arg("Файл поврежден")
                              .arg(""));
        setTreeWidgetItem(
            childItem, "Записано байт",
            QString::number(sizeof(tmpData) *
                            static_cast<unsigned long long>(rowCount)));
        setItemToDatabase(mainQuery,
                          insertErrorValuesTableParameters.arg(errorId)
                              .arg("Записано байт")
                              .arg(QString::number(
                                  sizeof(tmpData) *
                                  static_cast<unsigned long long>(rowCount))));
        setTreeWidgetItem(
            childItem, "Записано байт (%)",
            QString::number(static_cast<double>(100 * sizeof(tmpData) *
                                                static_cast<double>(rows)) /
                                static_cast<double>(file->size()),
                            'f', 4));
        setItemToDatabase(
            mainQuery,
            insertErrorValuesTableParameters.arg(errorId)
                .arg("Записано байт (%)")
                .arg(QString::number(
                    static_cast<double>(100 * sizeof(tmpData) *
                                        static_cast<double>(rows)) /
                        static_cast<double>(file->size()),
                    'f', 4)));
        setTreeWidgetItem(childItem, "Всего байт",
                          QString::number(file->size()));
        setItemToDatabase(mainQuery,
                          insertErrorValuesTableParameters.arg(errorId)
                              .arg("Всего байт")
                              .arg(QString::number(file->size())));

        ui->lblStatusValue->setText("Ошибка записи");
        ui->lblStatusValue->setStyleSheet(
            "color: red; font: 10pt \"Century Gothic\";");
        ui->lblErrorValue->setText("Файл поврежден");
        ui->lblErrorValue->setStyleSheet(
            "color: red; font: 10pt \"Century Gothic\";");
        ui->lblError->setVisible(true);
        ui->lblErrorValue->setVisible(true);

    } else {
        ui->lblStatusValue->setText("Успешно");
        ui->lblStatusValue->setStyleSheet(
            "color: green; font: 10pt \"Century Gothic\";");
    }
    /*--Проверка на пустые записи--*/
    if (firstEmptyRow != 0) {
        ++errorId;
        QTreeWidgetItem *childItem =
            new QTreeWidgetItem(warningsParentItem);
        childItem->setText(0, "Пустые записи");
        setItemToDatabase(
            mainQuery,
            insertErrorTableParameters.arg(errorId).arg("Пустые записи").arg(
                ""));
        setTreeWidgetItem(childItem, "Номер первой записи",
                          QString::number(firstEmptyRow));
        setItemToDatabase(
            mainQuery,
            insertErrorValuesTableParameters.arg(errorId)
                .arg("Номер первой записи")
                .arg(QString::number(firstEmptyRow)));

        setTreeWidgetItem(childItem, "Количество",
                          QString::number(emptyRows));
        setItemToDatabase(
            mainQuery,
            insertErrorValuesTableParameters.arg(errorId)
                .arg("Количество")
                .arg(QString::number(emptyRows)));
    }
    /*--Проверка на наличие SNS--*/
    if (firstEmptySNSRow != 0) {
        ++errorId;
        QTreeWidgetItem *childItem =
            new QTreeWidgetItem(warningsParentItem);
        childItem->setText(0, "Отсутствует SNS время");
        setItemToDatabase(
            mainQuery,
            insertErrorTableParameters.arg(errorId).arg("Отсутствует SNS время").arg(
                ""));
        setTreeWidgetItem(childItem, "Номер первой записи",
                          QString::number(firstEmptySNSRow));
        setItemToDatabase(
            mainQuery,
            insertErrorValuesTableParameters.arg(errorId)
                .arg("Номер первой записи")
                .arg(QString::number(firstEmptySNSRow)));

        setTreeWidgetItem(childItem, "Количество",
                          QString::number(emptySNSs));
        setItemToDatabase(
            mainQuery,
            insertErrorValuesTableParameters.arg(errorId)
                .arg("Количество")
                .arg(QString::number(emptySNSs)));
    }

    QTime flightTime;
    flightTime = QTime(0, 0, 0).addSecs(flightStarted.secsTo(flightFinished));
    setTreeWidgetItem(generalParentItem, "Время в полете",
                      flightTime.toString());
    setItemToDatabase(mainQuery,
                      insertGeneralTableParameters.arg("Время в полете")
                          .arg(flightTime.toString()));
    setTreeWidgetItem(generalParentItem, "Максимальная высота от земли, м",
                      QString::number(maxAltitude - minAltitude, 'f', 3));
    setItemToDatabase(
        mainQuery,
        insertGeneralTableParameters.arg("Максимальная высота от земли, м")
            .arg(QString::number(maxAltitude - minAltitude, 'f', 3)));
    setTreeWidgetItem(generalParentItem, "Максимальная высота от моря, м",
                      QString::number(maxAltitude, 'f', 3));
    setItemToDatabase(
        mainQuery,
        insertGeneralTableParameters.arg("Максимальная высота от моря, м")
            .arg(QString::number(maxAltitude, 'f', 3)));
    setTreeWidgetItem(generalParentItem, "Записей в файле",
                      QString::number(rowCount));
    setItemToDatabase(
        mainQuery,
        insertGeneralTableParameters.arg("Записей в файле")
            .arg(QString::number(rowCount)));


    mainQuery->~QSqlQuery();
}

void StatisticsWindow::setTreeWidgetItem(QTreeWidgetItem *parent,
                                         QString firstColumn,
                                         QString secondColumn) {
    QTreeWidgetItem *childItem = new QTreeWidgetItem(parent);
    childItem->setText(0, firstColumn);
    childItem->setText(1, secondColumn);
}

void StatisticsWindow::setItemToDatabase(QSqlQuery *query, QString parameter) {
    query->exec(parameter);
}

void StatisticsWindow::on_bbConfirm_clicked(QAbstractButton *button) {
    Q_UNUSED(button)
    QSqlDatabase::database("setStatisticsConnection").close();
    QSqlDatabase::removeDatabase("setStatisticsConnection");
    QSqlDatabase::database("getStatisticsConnection").close();
    QSqlDatabase::removeDatabase("getStatisticsConnection");
    emit statsWindowClosed();
}


void StatisticsWindow::on_twStatistics_expanded(const QModelIndex &index) {
    Q_UNUSED(index)
    ui->twStatistics->resizeColumnToContents(0);
}


void StatisticsWindow::on_twStatistics_collapsed(const QModelIndex &index){
    Q_UNUSED(index)
    ui->twStatistics->resizeColumnToContents(0);
}


void StatisticsWindow::on_StatisticsWindow_finished(int result)
{
    Q_UNUSED(result)
    QSqlDatabase::database("setStatisticsConnection").close();
    QSqlDatabase::removeDatabase("setStatisticsConnection");
    QSqlDatabase::database("getStatisticsConnection").close();
    QSqlDatabase::removeDatabase("getStatisticsConnection");
    emit statsWindowClosed();
}

