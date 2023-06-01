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

enum SWanScanMode
{
    SM_RECT,
    SM_DEG,
    SM_PIX
};

class SwanSetting : public QObject
{
    Q_OBJECT
public:
    explicit SwanSetting(QObject* parent = nullptr);
    ~SwanSetting();
public:
    /****** General group begin******/
    QString picUSBPath(); void setPicUSBPath(QString);
#define SWAN_PICUSBPATH (SwanINI.picUSBPath())
    QString picJCPath(); void setPicJCPath(QString);
#define SWAN_PICJCPATH (SwanINI.picJCPath())

    int rectStep(); void setRectStep(int);
#define SWAN_RECTSTEP (SwanINI.rectStep())
    int startPixel(); void setStartPixel(int);
#define SWAN_STARTPIXEL (SwanINI.startPixel())
    int endPixel(); void setEndPixel(int);
#define SWAN_ENDPIXEL (SwanINI.endPixel())
    int pixelStep(); void setPixelStep(int);
#define SWAN_PIXELSTEP (SwanINI.pixelStep())
    int startAngle(); void setStartAngle(int);
#define SWAN_STARTANGLE (SwanINI.startAngle())
    int endAngle(); void setEndAngle(int);
#define SWAN_ENDANGLE (SwanINI.endAngle())
    double angleStep(); void setAngleStep(double);
#define SWAN_ANGLESTEP (SwanINI.angleStep())
    QSize imageSize(); void setImageSize(QSize);
#define SWAN_IMAGESIZE (SwanINI.imageSize())
    SWanScanMode scanMode(); void setScanMode(SWanScanMode);
#define SWAN_SCANMODE (SwanINI.scanMode())
    /****** General group end******/

    /****** Motor group begin******/
    int speed(); void setSpeed(int);
#define SWAN_SPEED (SwanINI.speed())
    int lastPosition(); void setLastPosition(int);
#define SWAN_LASTPOSITION (SwanINI.lastPosition())
    int readyPosition(); void setReadyPosition(int);
#define SWAN_READYPOSITION (SwanINI.readyPosition())
    /****** Motor group end******/

    /****** Camera group begin******/
    QString cameraName();
#define SWAN_CAMERANAME (SwanINI.cameraName())
    int viewRange(); void setViewRange(int);
#define SWAN_VIEWRANGE (SwanINI.viewRange())
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

    /****** JingCui group begin******/
    int exposure(); void setExposure(int);
#define SWAN_EXPOSURE (SwanINI.exposure())
    bool mirrorv(); void setMirrorv(bool);
#define SWAN_MIRRORV (SwanINI.mirrorv())
    bool mirrorh(); void setMirrorh(bool);
#define SWAN_MIRRORH (SwanINI.mirrorh())
    /****** JingCui group end******/

private:

    QSettings *_settings;
};

#define SwanINI (Singleton<Cg::Swan::Comm::SwanSetting>::instance())

} } }

#endif  // GLOBALSETTING_H
