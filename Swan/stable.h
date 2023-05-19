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

#include <QImage>
#include <QDebug>
#include <QThread>
#include <QDateTime>
#include <QString>
#include "comm/globalsetting.h"

using namespace Cg::Swan::Comm;

#define sleep(x) QThread::msleep(x)
#define HexStr(x) QString::number(x, 16)
//#define now QDateTime::currentDateTime().toString("hh:mm:ss.zzz")
#define logdebug  qDebug()
#define loginfo  qInfo()
#define logerror  qCritical()

#endif // STABLE_H
