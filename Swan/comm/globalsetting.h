/**************************************************************************
**
** Copyright (C) 2023 Mike Feng
** Blog: https://www.cnblogs.com/crazyghostvon/
**
** This file (globalsetting.h) is part of the Swan project.
**
**************************************************************************/
#ifndef GLOBALSETTING_H
#define GLOBALSETTING_H

#include <QObject>
#include <QSettings>
#include <QtCore>
#include <QColor>
#include <QApplication>
#include "Singleton.h"

namespace Cg { namespace Swan { namespace Comm {

class SwanSetting : public QObject
{
    Q_OBJECT
public:
    explicit SwanSetting(QObject* parent = nullptr);
    ~SwanSetting();
public:
    // General settings [group]

    // [r] indicator to camera size
    QSize imageSize();
#define SWAN_IMAGESIZE (SwanINI.imageSize())


    /****** Camera group begin******/
    int viewAngle(); void setViewAngle(int);
#define SWAN_VIEW_ANGLE (SwanINI.viewAngle())
    int yline1(); void setYline1(int);
#define SWAN_YLINE1 (SwanINI.yline1())
    int yline2(); void setYline2(int);
#define SWAN_YLINE2 (SwanINI.yline2())
    int xCenter(); void setXCenter(int);
#define SWAN_XCENTER (SwanINI.xCenter())
    QColor lineColor(); void setLineColor(QColor);
#define SWAN_LINECOLOR (SwanINI.lineColor())
    QColor rectColor(); void setRectColor(QColor);
#define SWAN_RECTCOLOR (SwanINI.rectColor())
    QColor mouseColor(); void setMouseColor(QColor);
#define SWAN_MOUSECOLOR (SwanINI.mouseColor())
    /****** Camera group end******/

private:

    QSettings *_settings;
};

#define SwanINI (Singleton<Cg::Swan::Comm::SwanSetting>::instance())

} } }

#endif  // GLOBALSETTING_H
