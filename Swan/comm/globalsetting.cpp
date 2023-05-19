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
QSize SwanSetting::imageSize()
{
    return _settings->value("imageSize", QSize(1920,1080)).toSize();
}


/*----------- Camera -----------*/
int SwanSetting::viewAngle()
{
    return _settings->value("Camera/viewAngle", 60).toInt();
}
void SwanSetting::setViewAngle(int v)
{
    _settings->setValue("Camera/viewAngle", v);
}
int SwanSetting::yline1()
{
    return _settings->value("Camera/yline1", 360).toInt();
}
void SwanSetting::setYline1(int v)
{
    _settings->setValue("Camera/yline1", v);
}
int SwanSetting::yline2()
{
    return _settings->value("Camera/yline2", 720).toInt();
}
void SwanSetting::setYline2(int v)
{
    _settings->setValue("Camera/yline2", v);
}
int SwanSetting::xCenter()
{
    return _settings->value("Camera/xCenter", 960).toInt();
}
void SwanSetting::setXCenter(int v)
{
    _settings->setValue("Camera/xCenter", v);
}
QColor SwanSetting::lineColor()
{
    QList<int> rgblist = _settings->value("Camera/lineColor").value<QList<int>>();
    QColor c(0,255,127);
    if(rgblist.length() == 3)
    {
        int r = rgblist[0];
        int g = rgblist[1];
        int b = rgblist[2];

        c = QColor(r,g,b);
    }

    return c;
}
void SwanSetting::setLineColor(QColor v)
{
    QList<int> rgblist;
    rgblist.append(v.red());
    rgblist.append(v.green());
    rgblist.append(v.blue());

    _settings->setValue("Camera/lineColor", QVariant::fromValue(rgblist));
}

QColor SwanSetting::rectColor()
{
    QList<int> rgblist = _settings->value("Camera/rectColor").value<QList<int>>();
    QColor c(0,255,127);
    if(rgblist.length() == 3)
    {
        int r = rgblist[0];
        int g = rgblist[1];
        int b = rgblist[2];

        c = QColor(r,g,b);
    }

    return c;
}
void SwanSetting::setRectColor(QColor v)
{
    QList<int> rgblist;
    rgblist.append(v.red());
    rgblist.append(v.green());
    rgblist.append(v.blue());

    _settings->setValue("Camera/rectColor", QVariant::fromValue(rgblist));
}

QColor SwanSetting::mouseColor()
{
    QList<int> rgblist = _settings->value("Camera/mouseColor").value<QList<int>>();
    QColor c(0,255,127);
    if(rgblist.length() == 3)
    {
        int r = rgblist[0];
        int g = rgblist[1];
        int b = rgblist[2];

        c = QColor(r,g,b);
    }

    return c;
}
void SwanSetting::setMouseColor(QColor v)
{
    QList<int> rgblist;
    rgblist.append(v.red());
    rgblist.append(v.green());
    rgblist.append(v.blue());

    _settings->setValue("Camera/mouseColor", QVariant::fromValue(rgblist));
}
