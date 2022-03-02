#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QtCore/QAbstractItemModel>
#include <QtWidgets>

class FileDialog : public QFileDialog {
    Q_OBJECT
public:
    FileDialog();
    QString getDescription();

private:
    QTextEdit *textEdit;
    QLabel *label;
};

#endif // FILEDIALOG_H
