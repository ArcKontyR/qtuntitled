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
    //void setFileRowCount(int);
    //void setFileCorruption(bool);
private slots:
    void on_buttonBox_clicked(QAbstractButton *button);


private:
    Ui::StatisticsWindow *ui;

    QString fileName;
};

#endif // STATISTICSWINDOW_H
