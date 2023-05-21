/**************************************************************************
**
** Copyright (C) 2023 Mike Feng
** Blog: https://www.cnblogs.com/crazyghostvon/
**
** This file (usbcameraadapter.h) is part of the Swan project.
**
**************************************************************************/
#ifndef USBCAMERAADAPTER_H
#define USBCAMERAADAPTER_H

#include <stable.h>
#include <QObject>
#include <QCamera>
#include <QCameraInfo>
#include <QCameraImageCapture>
#include <QtConcurrent>
#include <QCameraViewfinderSettings>

namespace Cg { namespace Swan { namespace Adapter {

enum USBCameraAction
{
    UA_Default,
    UA_RunOnSpeed,
    UA_RunOnDistance,
    UA_FindBarrier,
    UA_GotoReady,
    UA_StopRun,
    UA_ReadStatus
};

struct USBCameraActionParam
{
    USBCameraAction UAction = UA_Default;
    bool Direction; // run in clockwise : 1 or counterclockwise : 0,
    bool EnableLeftFlag = true;  // when has load, set it to true
    bool EnableRightFlag = true; // when has load, set it to true
    bool StopOnLeftFlagLow = true; // this is determined by sensor, should not be changed in software
    bool StopOnRightFlagLow = true;// this is determined by sensor, should not be changed in software
    bool setZeroPoint = false;  // when arrived the flag, set the position as zero.
    bool relativeFlag = false;  // when run in distance mode, indicates the target position is at the right or left side.
    qint32 data32=0;
};

class USBCameraAdapter : public QObject
{
    Q_OBJECT
public:
    explicit USBCameraAdapter(QObject *parent = nullptr);

    void Quit();
    void setPreviewResolution(const QSize &);
signals:
    void newImage(const QImage &preview);
    void status(int);

public slots:
    void init();
    void execute(const USBCameraActionParam &);

private slots:
    void imageCaptured(int id, const QImage &preview);

private:
    QCamera  *_camera  = nullptr;
    QCameraImageCapture *_capture = nullptr;
    bool _isStarted = true;

};

} } }// namespace Cg::Swan::Adapter
#endif // USBCAMERAADAPTER_H
