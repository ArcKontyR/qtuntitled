#include "filedialog.h"

FileDialog::FileDialog() {
    this->setOption(QFileDialog::DontUseNativeDialog);
    QGridLayout *mainLayout = qobject_cast<QGridLayout *>(this->layout());
    textEdit = new QTextEdit;
    label = new QLabel("Описание");
    mainLayout->addWidget(label, 4, 0);
    mainLayout->addWidget(textEdit, 4, 1);
}

QString FileDialog::getDescription() {
    return textEdit->toPlainText();
}
