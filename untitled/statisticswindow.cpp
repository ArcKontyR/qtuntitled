#include "statisticswindow.h"
#include "mainwindow.h"
#include "ui_statisticswindow.h"

#include <QtSql>
StatisticsWindow::StatisticsWindow(QWidget *parent) :
                                                      QDialog(parent),
                                                      ui(new Ui::StatisticsWindow)
{
    ui->setupUi(this);

    QSqlDatabase db =
        QSqlDatabase::addDatabase("QSQLITE", "statisticsConnection");
    db.setDatabaseName("databases/" + fileName + ".sqlite");
    db.open();
    QFile *file = new QFile(fileName);
    file->open(QFile::ReadOnly);
    QDataStream stream(file);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setVersion(QDataStream::Qt_5_15);
    BpiData prevData;
    BpiData tmpData;
    QTreeWidgetItem *generalParentItem = new QTreeWidgetItem(ui->treeWidget);
    generalParentItem->setText(0,"Общее");
    while (stream.atEnd() == false && stream.status() == QDataStream::Ok) {
        prevData = tmpData;
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

        if (prevData.nNavCounter - tmpData.nNavCounter >1){
            QTreeWidgetItem *childItem = new QTreeWidgetItem(generalParentItem);
            childItem->setText(0, "");

        }
    }

    QSqlQuery *query = new QSqlQuery(db);
    query->exec("SELECT * FROM DataTable");
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

void StatisticsWindow::on_buttonBox_clicked(QAbstractButton *button) {
    qDebug() << fileName;
}

void StatisticsWindow::setFileName(QString _fileName) { fileName = _fileName;
}




