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
#define timenow QDateTime::currentDateTime().toString("hh:mm:ss.zzz")
#define logdebug  qDebug().noquote() << timenow << "  "
#define loginfo  qInfo().noquote() << timenow << "  "
#define logerror  qCritical().noquote() << timenow << "  "

#endif // STABLE_H
