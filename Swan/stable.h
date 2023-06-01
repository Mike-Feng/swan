 /**************************************************************************
**
** Copyright (C) 2023 Mike Feng
** Blog: https://www.cnblogs.com/crazyghostvon/
**
** This file (stable.h) is part of the Swan project.
**
**************************************************************************/
#ifndef STABLE_H
#define STABLE_H

#include <QtGlobal>
#include <QImage>
#include <QDebug>
#include <QThread>
#include <QDateTime>
#include <QString>
#include "comm/globalsetting.h"

using namespace Cg::Swan::Comm;

#define sMin(a, b) (a < b) ? a : b
#define sMax(a, b) (a > b) ? a : b
#define sleep(x) QThread::msleep(x)
#define HexStr(x) QString::number(x, 16)
#ifdef QT_NO_DEBUG
    #define timenow ""
#else
    #define timenow QDateTime::currentDateTime().toString("hh:mm:ss.zzz")
#endif

#ifdef QT_NO_DEBUG
#define logdebug  qDebug().noquote()
#define loginfo  qInfo().noquote()
#define logerror  qCritical().noquote()
#else
#define logdebug  qDebug().noquote() << timenow << "  "
#define loginfo  qInfo().noquote() << timenow << "  "
#define logerror  qCritical().noquote() << timenow << "  "
#endif

#endif // STABLE_H
