/**************************************************************************
**
** Copyright (C) 2023 Mike Feng
** Blog: https://www.cnblogs.com/crazyghostvon/
**
** This file main.cpp is part of the Swan project.
**
**************************************************************************/
#include "mainwindow.h"

#include <QApplication>
#include <QStandardPaths>
#include <QDir>

#include "log4qt/logmanager.h"
#include <log4qt/propertyconfigurator.h>
#include <log4qt/consoleappender.h>
#include <log4qt/dailyrollingfileappender.h>
#include <log4qt/patternlayout.h>
#include <QDebug>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    Log4Qt::PropertyConfigurator::configure("log.ini");
#ifdef QT_NO_DEBUG
    Log4Qt::LogManager::setHandleQtMessages(true);
#endif

    Log4Qt::Logger *logger = Log4Qt::Logger::rootLogger();
    Log4Qt::PatternLayout *layout = new Log4Qt::PatternLayout();
    layout->setName("PatternLayout");
    layout->setConversionPattern("%d{yyyy-MM-dd HH:mm:ss.zzz} [%p] %m%n");
    layout->activateOptions();

    // show log at the output window
    Log4Qt::ConsoleAppender *appender = new Log4Qt::ConsoleAppender();
    appender->setName("ConsoleAppender");
    appender->setLayout(layout);
    appender->setEncoding(QTextCodec::codecForName("UTF-8"));
    appender->setTarget(Log4Qt::ConsoleAppender::STDOUT_TARGET);  // write log to stdout
    appender->setImmediateFlush(true);
    appender->setThreshold(Log4Qt::Level::ALL_INT);  // output all log
    appender->activateOptions();

    // add appender
    logger->addAppender(appender);

    logger->setLevel(Log4Qt::Level::DEBUG_INT);

    MainWindow w;
    w.showMaximized();
    int ret = a.exec();

    return ret;
}
