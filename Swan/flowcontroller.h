/**************************************************************************
**
** Copyright (C) 2023 Mike Feng
** Blog: https://www.cnblogs.com/crazyghostvon/
**
** This file (flowcontroller.h) is part of the Swan project.
**
**************************************************************************/
#ifndef FLOWCONTROLLER_H
#define FLOWCONTROLLER_H

#include "stable.h"
#include <QObject>
#include <QtConcurrent>
#include <adapter/jcoptixcameraadapter.h>
#include <adapter/usbcameraadapter.h>
#include <adapter/motoradapter.h>

using namespace Cg::Swan::Adapter;

namespace Cg { namespace Swan { namespace Controller {

enum ImageSource{
    IS_Camera,
    IS_Sensor
};

enum ActionTarget{
    MotorTarget,
    CameraTarget,
    SensorTarget,
    FlowTarget
};

enum FlowAction{
    FA_Run,
    FA_Stop,
    FA_Pause,
    FA_Resume,
    FA_Reset,
    FA_GotoReady

};

struct ScanRange
{
    int start;
    int end;
};

struct FlowActionParam
{
    FlowAction fAction;
    int start;
    int end;
    double step;
    SWanScanMode mode;
};

struct ActionParam{
    ActionTarget target;
    MotorActionParam motorParam;
    USBCameraActionParam cameraParam;
    JCCameraActionParam jcParam;
    FlowActionParam flowParam;
};

class FlowController : public QObject
{
    Q_OBJECT
public:
    explicit FlowController(QObject *parent = nullptr);
    void Quit();
    void setScanRects(const QList<ScanRange> &);

    bool isProcessRunning() {return _isProcessRunning; }
    bool isProcessPause() {return _isProcessPause; }

public slots:
    void init();
    void execute(const ActionParam &);

signals:
    void newImage(const QImage &, ImageSource);

    void callMotor(const MotorActionParam &);
    void callJCoptix(const JCCameraActionParam &);
    void callUSBCamera(const USBCameraActionParam &);

    void adapterStatus(int);
    void actionFinished(double);
    void livePosition(double);
private slots:
    void recieveNewImage(const QImage &);
    void motorStateChanged(const MotorState &);
    void handleActionFinished();
    void handleLivePosition(int);
    void statusHandler(int);

private:
    void takePhotoProcess();

private:
    JCOptixCameraAdapter * optix = nullptr;
    USBCameraAdapter * camera = nullptr;
    MotorAdapter * motor = nullptr;
    QThread * toptix = nullptr;
    QThread * tcamera = nullptr;
    QThread * tmotor = nullptr;

    double     angleStart = -30;
    double     angleEnd = 30;
    double     angleStep = 1.8;
    // 1 degree need 256 / 1.8 pulse, the rate of reduction gears is 10.
    double     anglePerPulse = 256 / 1.8 * 10;
    // the lens angle is 60°, and the width is 1920 pixels
    // 1 pixel need this pulse
    double     pixelPerPulse = SWAN_VIEWRANGE * 1.0 / SWAN_IMAGESIZE.width() / 1.8 * 256.0 * 10;
    SWanScanMode        scanmode = SM_DEG;  // scanmode: 0 scan in custom rectangle, 1 scan in degree, 2 scan in pixel

    int        pixelStart = -30; // scan range draw by user
    int        pixelEnd = 30;
    int        pixelStep = 20;
    int        middlePixel = SWAN_IMAGESIZE.width() / 2;
    QList<ScanRange> rects;

    int        stepIndex = 0;
    bool       isReset = false;
    bool       _isProcessRunning = false;
    bool       _isProcessPause = false;
    bool       _isImageSaved = false;
    bool       _isFirstSaved = false;
    qint32     targetPosition = 0;  // used in runDistance mode, indicates the position.
    int        endPosition = 0;
    MotorState mstate;


    bool  isMotorActionFinished = false;
    bool  isRaiseActionFinished = false;
    bool  isJCStream = false;

};

} } }// namespace Cg::Swan::Controller
#endif // FLOWCONTROLLER_H
