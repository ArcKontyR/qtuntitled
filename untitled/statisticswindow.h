#ifndef STATISTICSWINDOW_H
#define STATISTICSWINDOW_H
#include <QtGui>
#include <QtWidgets>

namespace Ui {
class StatisticsWindow;
}

class StatisticsWindow : public QDialog
{
    Q_OBJECT

        public:
                 explicit StatisticsWindow(QWidget *parent = nullptr);
    ~StatisticsWindow();

protected:
    void changeEvent(QEvent *e);

public slots:
    void setFileName(QString);
    void setFilePath(QString);
    void setFileRowCount(int);
    void setFileCorruption(bool);
    void setStats();
    void getStats();

private slots:
private:
    Ui::StatisticsWindow *ui;
    QString fileName;
    QString filePath;
    int rowCount;
    bool isCorrupted;
};

#endif // STATISTICSWINDOW_H
