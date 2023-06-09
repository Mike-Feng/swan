/**************************************************************************
**
** Copyright (C) 2023 Mike Feng
** Blog: https://www.cnblogs.com/crazyghostvon/
**
** This file (flowcontroller.cpp) is part of the Swan project.
**
**************************************************************************/
#include "flowcontroller.h"
using namespace Cg::Swan::Controller;

FlowController::FlowController(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<ActionParam>("ActionParam");
    qRegisterMetaType<ActionParam>("ActionParam&");
    qRegisterMetaType<ImageSource>("ImageSource");
    qRegisterMetaType<ImageSource>("ImageSource&");
}

void FlowController::init()
{
    toptix = new QThread();
    optix = new JCOptixCameraAdapter();
    optix->moveToThread(toptix);
    connect(toptix, &QThread::started, optix, &JCOptixCameraAdapter::init);

    tcamera =new QThread();
    camera = new USBCameraAdapter();
    camera->moveToThread(tcamera);
    connect(tcamera, &QThread::started, camera, &USBCameraAdapter::init);

    tmotor = new QThread();
    motor = new MotorAdapter();
    motor->moveToThread(tmotor);
    connect(tmotor, &QThread::started, motor, &MotorAdapter::init);

    connect(camera, &USBCameraAdapter::newImage, this, &FlowController::recieveNewImage);
    connect(camera, &USBCameraAdapter::status, this, &FlowController::statusHandler);
    connect(optix, &JCOptixCameraAdapter::newImage, this, &FlowController::recieveNewImage);
    connect(optix, &JCOptixCameraAdapter::status, this, &FlowController::statusHandler);
    connect(motor, &MotorAdapter::motorStateChanged, this, &FlowController::motorStateChanged);
    connect(motor, &MotorAdapter::actionFinished, this, &FlowController::handleActionFinished);
    connect(motor, &MotorAdapter::livePosition, this, &FlowController::handleLivePosition);

    connect(this, &FlowController::callUSBCamera, camera, &USBCameraAdapter::execute);
    connect(this, &FlowController::callJCoptix, optix, &JCOptixCameraAdapter::execute);
    connect(this, &FlowController::callMotor, motor, &MotorAdapter::execute);

    toptix->start();
    tcamera->start();
    tmotor->start();

    QDir jdir(SWAN_PICJCPATH + QDir::separator());
    if(!jdir.exists())
        jdir.mkpath(".");

    QDir udir(SWAN_PICUSBPATH + "Preview" + QDir::separator());
    if(!udir.exists())
        udir.mkpath(".");
}

void FlowController::execute(const ActionParam & param)
{
    switch (param.target) {
    case FlowTarget:
    {
        switch (param.flowParam.fAction) {
        case FA_Reset:
        {
            MotorActionParam param;
            param.MAction = MA_FindBarrier;
            param.Direction = MD_CounterClock;
            _isProcessRunning = false;
            _isProcessPause = false;

            emit callMotor(param);
            break;
        }
        case FA_Run:
        {
            scanmode = param.flowParam.mode;
            if(scanmode == SM_DEG)
            {
                angleStart = param.flowParam.start;
                angleEnd = param.flowParam.end;
                angleStep = param.flowParam.step;
                targetPosition = param.flowParam.start * anglePerPulse;
                endPosition = param.flowParam.end * anglePerPulse;
            }
            else
            {
                pixelStart = param.flowParam.start;
                pixelEnd = param.flowParam.end;
                pixelStep = param.flowParam.step;
                targetPosition = (pixelStart - middlePixel) * pixelPerPulse;
                endPosition = (pixelEnd - middlePixel) * pixelPerPulse;
            }

            stepIndex = 0;
            _isProcessRunning = true;
            _isProcessPause = false;
            _isFirstSaved = false;
            _isImageSaved = false;

            optix->setTriggerMode(1);

            takePhotoProcess();
            break;
        }
        case FA_Pause:
        {
            _isProcessPause = true;
            break;
        }
        case FA_Resume:
        {
            _isProcessPause = false;
            break;
        }
        case FA_Stop:
        {
            _isProcessRunning = false;
            break;
        }
        case FA_GotoReady:
        {
            isRaiseActionFinished = true;
            MotorActionParam param;
            param.MAction = MA_RunOnDistance;
            param.data32 = 0;
            _isProcessRunning = false;
            _isProcessPause = false;
            emit callMotor(param);

            break;
        }
        }

        break;
    }
    case MotorTarget:
    {
        isRaiseActionFinished = true;

        emit callMotor(param.motorParam);
        break;
    }
    case CameraTarget:
    {
        emit callUSBCamera(param.cameraParam);
        break;
    }
    case SensorTarget:
    {
        if(param.jcParam.Action == JC_Preview)
        {
            isJCStream = true;
        }
        else
            isJCStream = false;

        emit callJCoptix(param.jcParam);
        break;
    }
    }

}

void FlowController::recieveNewImage(const QImage &preview)
{
    QObject *object=sender();
    if(object == optix)
    {
        QString filename(SWAN_PICJCPATH + QDir::separator() + "Preview" + QDir::separator());
        if(_isProcessRunning)
        {
            switch (scanmode) {
            case SM_RECT:
                filename += QString("RECT_%1_%2_").arg(pixelStart).arg(pixelEnd);
                break;
            case SM_DEG:
                filename += QString("Angle_%1_%2_").arg(angleStart).arg(angleEnd);
                break;
            case SM_PIX:
                filename +=  QString("Pixel_%1_%2_").arg(pixelStart).arg(pixelEnd);
                break;
            }

            filename += QString("%1_%2.bmp").arg(stepIndex, 4, 10, QChar('0')).arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

            QString path = QDir::cleanPath(filename);
            preview.save(path, "bmp");
            _isImageSaved = true;
        }
        else
        {
            if(!isJCStream)
            {
                filename += QString("%1.bmp").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
                QString path = QDir::cleanPath(filename);
                preview.save(path, "bmp");
            }
        }

        logdebug << "saved file:" << QDir::cleanPath(filename);
        emit newImage(preview, ImageSource::IS_Sensor);
    }
    else
    {        
        if(_isProcessRunning && !_isFirstSaved)
        {
            QString filename(SWAN_PICUSBPATH + QDir::separator());
            filename += QString("GW_%1.bmp").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
            QString path = QDir::cleanPath(filename);
            preview.save(path, "bmp");
            logdebug << "saved file:" << path;
            _isFirstSaved = true;
        }
        emit newImage(preview, ImageSource::IS_Camera);
    }
}

void FlowController::Quit()
{
    camera->Quit();
    optix->Quit();
    motor->Quit();
}

void FlowController::setScanRects(const QList<ScanRange> & ranges)
{
    rects.clear();

    // put the line to the x-axis
    QList<int> endPoints;
    QList<bool> startFlag;
    foreach(ScanRange r, ranges)
    {
        int index = 0;
        // insert the start point
        for(;index < endPoints.length() && r.start > endPoints[index]; index++ );
        endPoints.insert(index, r.start);
        startFlag.insert(index, true);
        // insert the end point
        for(;index < endPoints.length() && r.end >= endPoints[index]; index++ );
        endPoints.insert(index, r.end);
        startFlag.insert(index, false);
    }

    QStack<int> stack;
    for(int index = 0; index < endPoints.length(); index++)
    {
        if(startFlag[index])
        {
            stack.push(endPoints[index]);
        }
        else
        {
            int pstart = stack.pop();
            if(stack.size() == 0)
            {
                ScanRange sr;
                sr.end = (endPoints[index] - middlePixel) * pixelPerPulse;
                sr.start = (pstart - middlePixel) * pixelPerPulse;
                rects.append(sr);
            }
        }
    }
}

void FlowController::motorStateChanged(const MotorState & status)
{
    mstate = status;
    if(!status.isConnected)
        emit adapterStatus(3);
}

void FlowController::statusHandler(int status)
{
    if(status < 0)
    {
        if(sender() == camera)
            emit adapterStatus(1);
        else
            emit adapterStatus(2);
    }
}

void FlowController::handleActionFinished()
{
    isMotorActionFinished = true;
    if(isRaiseActionFinished)
    {
        isRaiseActionFinished = false;
        emit actionFinished(targetPosition / pixelPerPulse + SWAN_IMAGESIZE.width() / 2);
    }
}

void FlowController::handleLivePosition(int p)
{
    emit livePosition(p / pixelPerPulse + SWAN_IMAGESIZE.width() / 2);
}

void FlowController::takePhotoProcess()
{
    QtConcurrent::run([=]() {
        logdebug << "begin take photo process, from " << targetPosition << " to " << endPosition;

        while(_isProcessRunning)
        {
            if(_isProcessPause)
            {
                sleep(100);
                continue;
            }

            logdebug << "Round " << stepIndex << ": " << targetPosition;
            stepIndex ++;

            isMotorActionFinished = false;
            isRaiseActionFinished = true;
            // goto start position
            MotorActionParam param;
            param.MAction = MA_RunOnDistance;
            param.data32 = targetPosition;

            emit callMotor(param);

            while(!isMotorActionFinished)
                sleep(50);

            // take an image
            _isImageSaved = false;
            JCCameraActionParam jcparam;
            jcparam.Action = JC_TriggerOnce;
            emit callJCoptix(jcparam);

            while(!_isImageSaved && _isProcessRunning)
                sleep(50);

            if(targetPosition >= endPosition)
            {
                loginfo << "scan finished, image count:" << stepIndex ;
                break;
            }

            if(scanmode == SM_DEG)
            {
                targetPosition += anglePerPulse * angleStep;
            }
            else
            {
                targetPosition += pixelPerPulse * pixelStep;
                if(scanmode == SM_RECT)
                {
                    if(!rects.isEmpty() && targetPosition > rects[0].end)
                    {
                        rects.removeFirst();
                        if(rects.length() > 0)
                            targetPosition = rects[0].start; // jump to the next range
                    }
                }
            }
        }

        _isProcessRunning = false;
        _isProcessPause = false;
        emit actionFinished(0);
        logdebug << "stop take photo process.";
    });

}

