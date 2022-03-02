#ifndef STATISTICSWINDOW_H
#define STATISTICSWINDOW_H
#include <QtGui>
#include <QtWidgets>
#include <QtSql>

namespace Ui {
class StatisticsWindow;
}

class StatisticsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit StatisticsWindow(QWidget *parent = nullptr);
    ~StatisticsWindow() override;

protected:
    void changeEvent(QEvent *e) override;

public slots:
    void setFileName(QString);
    void setFilePath(QString);
    void setFileRowCount(int);
    void setFileCorruption(bool);
    void setStats();
    void getStats();

signals:
    void statsWindowClosed();
private slots:
    void on_bbConfirm_clicked(QAbstractButton *button);

    void on_twStatistics_expanded(const QModelIndex &index);

    void on_twStatistics_collapsed(const QModelIndex &index);

    void on_StatisticsWindow_finished(int result);

private:
    Ui::StatisticsWindow *ui;
    QString fileName;
    QString filePath;
    int rowCount;
    bool isCorrupted;

    void setTreeWidgetItem(QTreeWidgetItem *, QString, QString);
    void setItemToDatabase(QSqlQuery*, QString);
};

#endif // STATISTICSWINDOW_H
