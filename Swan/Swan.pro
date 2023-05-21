QT       += core gui serialbus serialport concurrent multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14 \
          hide_symbols

# refer to log4qt
include($$PWD/log4qt/log4qt.pri)


INCLUDEPATH += $$PWD/jcoptix/include
LIBS += -L$$PWD/jcoptix -lMVCAMSDK_X64

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS



# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    adapter/capturethread.cpp \
    adapter/jcoptixcameraadapter.cpp \
    adapter/usbcameraadapter.cpp \
    comm/globalsetting.cpp \
    controls/swangraphicsview.cpp \
    flowcontroller.cpp \
    main.cpp \
    mainwindow.cpp \
    adapter/motoradapter.cpp

HEADERS += \
    adapter/capturethread.h \
    adapter/jcoptixcameraadapter.h \
    adapter/usbcameraadapter.h \
    comm/Singleton.h \
    comm/call_once.h \
    comm/globalsetting.h \
    controls/swangraphicsview.h \
    flowcontroller.h \
    mainwindow.h \
    adapter/motoradapter.h \
    stable.h

FORMS += \
    mainwindow.ui

DISTFILES += \
    log.ini \
    swan.ini

CONFIG(release, debug|release) {
    DEST_DIR = release
} else {
    DEST_DIR = debug
}

copy_ini.commands = $${QMAKE_COPY_FILE} $$shell_path($$PWD/*.ini) $$shell_path($$OUT_PWD/$$DEST_DIR)

first.depends += $(first) copy_ini
QMAKE_EXTRA_TARGETS += first copy_ini


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

