/**************************************************************************
**
** Copyright (C) 2023 Mike Feng
** Blog: https://www.cnblogs.com/crazyghostvon/
**
** This file (globalsetting.cpp) is part of the Swan project.
**
**************************************************************************/
#include "globalsetting.h"
using namespace Cg::Swan::Comm;

SwanSetting::SwanSetting(QObject *parent) : QObject(parent)
{
    qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");
    QString settingFilePath = QDir(QApplication::applicationDirPath()).filePath("swan.ini");
    _settings = new QSettings(settingFilePath, QSettings::IniFormat);
    _settings->setIniCodec(QTextCodec::codecForName("UTF-8"));

}
SwanSetting::~SwanSetting()
{
    _settings->sync();
    delete _settings;
}

QString SwanSetting::picUSBPath(){ return _settings->value("picUSBPath").toString();}
void SwanSetting::setPicUSBPath(QString v){ _settings->setValue("picUSBPath", v);}
QString SwanSetting::picJCPath(){ return _settings->value("picJCPath").toString();}
void SwanSetting::setPicJCPath(QString v){ _settings->setValue("picJCPath", v);}

int SwanSetting::rectStep(){ return _settings->value("rectStep", 60).toInt(); }
void SwanSetting::setRectStep(int v){ _settings->setValue("rectStep", v); }

int SwanSetting::startPixel(){ return _settings->value("startPixel", 0).toInt(); }
void SwanSetting::setStartPixel(int v){ _settings->setValue("startPixel", v); }

int SwanSetting::endPixel(){ return _settings->value("endPixel", 1920).toInt(); }
void SwanSetting::setEndPixel(int v){ _settings->setValue("endPixel", v); }

int SwanSetting::pixelStep(){ return _settings->value("pixelStep", 60).toInt(); }
void SwanSetting::setPixelStep(int v){ _settings->setValue("pixelStep", v); }

int SwanSetting::startAngle(){ return _settings->value("startAngle", -30).toInt(); }
void SwanSetting::setStartAngle(int v){ _settings->setValue("startAngle", v); }

int SwanSetting::endAngle(){ return _settings->value("endAngle", 30).toInt(); }
void SwanSetting::setEndAngle(int v){ _settings->setValue("endAngle", v); }

double SwanSetting::angleStep(){ return _settings->value("angleStep", 1).toDouble(); }
void SwanSetting::setAngleStep(double v){ _settings->setValue("angleStep", v); }

QSize SwanSetting::imageSize()
{
    QStringList rgblist = _settings->value("imageSize").toString().split(",", Qt::SkipEmptyParts);
    QSize s(1920,1080);
    if(rgblist.length() == 2)
    {
        int w = rgblist[0].toInt();
        int h = rgblist[1].toInt();

        s = QSize(w, h);
    }

    return s;
}

void SwanSetting::setImageSize(QSize s)
{
    QStringList sizelist;
    sizelist.append(QString::number(s.width()));
    sizelist.append(QString::number(s.height()));

    _settings->setValue("imageSize", sizelist.join(","));
}

SWanScanMode SwanSetting::scanMode()
{
    QString m = _settings->value("scanMode", "SM_DEG").toString();

    if(m == "SM_RECT")
        return SM_RECT;
    else if(m == "SM_DEG")
        return SM_DEG;
    else if(m == "SM_PIX")
        return SM_PIX;
    else
        return SM_DEG; // default value

}
void SwanSetting::setScanMode(SWanScanMode v)
{
    switch (v) {
    case SM_RECT:
        _settings->setValue("scanMode", "SM_RECT");
        break;
    case SM_DEG:
        _settings->setValue("scanMode", "SM_DEG");
        break;
    case SM_PIX:
        _settings->setValue("scanMode", "SM_PIX");
        break;
    default:
        break;
    }
}

/*----------- Motor -----------*/
int SwanSetting::speed(){ return _settings->value("Motor/speed", 51200).toInt(); }
void SwanSetting::setSpeed(int v){ _settings->setValue("Motor/speed", v); }

int SwanSetting::lastPosition(){ return _settings->value("Motor/lastPosition", 0).toInt(); }
void SwanSetting::setLastPosition(int v){ _settings->setValue("Motor/lastPosition", v); }

int SwanSetting::readyPosition(){ return _settings->value("Motor/readyPosition", 0).toInt(); }
void SwanSetting::setReadyPosition(int v){ _settings->setValue("Motor/readyPosition", v); }


/*----------- Camera -----------*/
int SwanSetting::viewRange(){ return _settings->value("Camera/viewAngle", 60).toInt();}
void SwanSetting::setViewRange(int v){ _settings->setValue("Camera/viewAngle", v);}

int SwanSetting::yline1(){ return _settings->value("Camera/yline1", 360).toInt();}
void SwanSetting::setYline1(int v){ _settings->setValue("Camera/yline1", v);}

int SwanSetting::yline2(){ return _settings->value("Camera/yline2", 720).toInt();}
void SwanSetting::setYline2(int v){ _settings->setValue("Camera/yline2", v);}

int SwanSetting::xCenter(){ return _settings->value("Camera/xCenter", 960).toInt();}
void SwanSetting::setXCenter(int v){ _settings->setValue("Camera/xCenter", v);}

QColor SwanSetting::lineColor()
{
    QStringList rgblist = _settings->value("Camera/lineColor").toString().split(",", Qt::SkipEmptyParts);
    QColor c(0,255,127);
    if(rgblist.length() == 3)
    {
        int r = rgblist[0].toInt();
        int g = rgblist[1].toInt();
        int b = rgblist[2].toInt();

        c = QColor(r,g,b);
    }

    return c;
}
void SwanSetting::setLineColor(QColor v)
{
    QStringList rgblist;
    rgblist.append(QString::number(v.red()));
    rgblist.append(QString::number(v.green()));
    rgblist.append(QString::number(v.blue()));

    _settings->setValue("Camera/lineColor", rgblist.join(","));
}

QColor SwanSetting::rectColor()
{
    QStringList rgblist = _settings->value("Camera/rectColor").toString().split(",", Qt::SkipEmptyParts);
    QColor c(0,255,127);
    if(rgblist.length() == 3)
    {
        int r = rgblist[0].toInt();
        int g = rgblist[1].toInt();
        int b = rgblist[2].toInt();

        c = QColor(r,g,b);
    }

    return c;
}
void SwanSetting::setRectColor(QColor v)
{
    QStringList rgblist;
    rgblist.append(QString::number(v.red()));
    rgblist.append(QString::number(v.green()));
    rgblist.append(QString::number(v.blue()));

    _settings->setValue("Camera/rectColor", rgblist.join(","));
}

QColor SwanSetting::mouseColor()
{
    QStringList rgblist = _settings->value("Camera/mouseColor").toString().split(",", Qt::SkipEmptyParts);
    QColor c(0,255,127);
    if(rgblist.length() == 3)
    {
        int r = rgblist[0].toInt();
        int g = rgblist[1].toInt();
        int b = rgblist[2].toInt();

        c = QColor(r,g,b);
    }

    return c;
}
void SwanSetting::setMouseColor(QColor v)
{
    QStringList rgblist;
    rgblist.append(QString::number(v.red()));
    rgblist.append(QString::number(v.green()));
    rgblist.append(QString::number(v.blue()));

    _settings->setValue("Camera/mouseColor", rgblist.join(","));
}

/*----------- JingCui -----------*/
int SwanSetting::exposure(){ return _settings->value("JingCui/exposure", 2000).toInt(); }
void SwanSetting::setExposure(int v){ _settings->setValue("JingCui/exposure", v); }

bool SwanSetting::mirrorv(){return _settings->value("JingCui/mirrorv", false).toBool(); }
void SwanSetting::setMirrorv(bool v){ _settings->setValue("JingCui/mirrorv", v); }

bool SwanSetting::mirrorh(){return _settings->value("JingCui/mirrorh", false).toBool(); }
void SwanSetting::setMirrorh(bool v){ _settings->setValue("JingCui/mirrorh", v); }
