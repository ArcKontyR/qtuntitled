QT       += core gui sql charts qml quickwidgets concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
lessThan(QT_MAJOR_VERSION, 6): QT += location

CONFIG += c++11
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chart.cpp \
    chartview.cpp \
    filedialog.cpp \
    main.cpp \
    mainwindow.cpp \
    statisticswindow.cpp

HEADERS += \
    chart.h \
    chartview.h \
    filedialog.h \
    mainwindow.h \
    statisticswindow.h

FORMS += \
    mainwindow.ui \
    statisticswindow.ui
DISTFILES +=

RESOURCES += \
    qml.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
