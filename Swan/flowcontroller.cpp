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
    toptix->start();

    tcamera =new QThread();
    camera = new USBCameraAdapter();
    camera->moveToThread(tcamera);
    connect(tcamera, &QThread::started, camera, &USBCameraAdapter::init);
    tcamera->start();

    tmotor = new QThread();
    motor = new MotorAdapter();
    motor->moveToThread(tmotor);
    connect(tmotor, &QThread::started, motor, &MotorAdapter::init);
    tmotor->start();

    connect(camera, &USBCameraAdapter::newImage, this, &FlowController::recieveNewImage);
    connect(optix, &JCOptixCameraAdapter::newImage, this, &FlowController::recieveNewImage);
    connect(motor, &MotorAdapter::motorStateChanged, this, &FlowController::motorStateChanged);
    connect(motor, &MotorAdapter::actionFinished, this, &FlowController::handleActionFinished);

    connect(this, &FlowController::callUSBCamera, camera, &USBCameraAdapter::execute);
    connect(this, &FlowController::callJCoptix, optix, &JCOptixCameraAdapter::execute);
    connect(this, &FlowController::callMotor, motor, &MotorAdapter::execute);
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
            emit callMotor(param);;
            break;
        }
        case FA_Run:
        {
            if(scanmode == 1)
            {
                targetPosition = angleStart * anglePerPulse;
                endPosition = angleEnd * anglePerPulse;
            }
            else
            {
                targetPosition = pixelStart * pixelPerPulse;
                endPosition = angleEnd * pixelPerPulse;
            }

            stepIndex = 0;
            _isProcessRunning = true;
            _isProcessPause = false;

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
        }

        break;
    }
    case MotorTarget:
    {
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
        QString filename("AIC231GM_");
        if(_isProcessRunning)
        {
            if(scanmode == 1)
                filename += QString("Angle_%1_%2_").arg(angleStart).arg(angleEnd);
            else
                filename +=  QString("Pixel_%1_%2_").arg(_scanRange.x()).arg(_scanRange.x() + _scanRange.width());

            filename += QString("%1_%2.bmp").arg(stepIndex, 4, 10, QChar('0')).arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

            preview.save(filename, "bmp");
            _isImageSaved = true;
        }
        else
        {
            filename += QString("%1.bmp").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
            preview.save(filename, "bmp");
        }

        logdebug << "saved file:" << filename;
        emit newImage(preview, ImageSource::IS_Sensor);
    }
    else
    {
        emit newImage(preview, ImageSource::IS_Camera);
    }
}

void FlowController::Quit()
{
    camera->Quit();
    optix->Quit();
    motor->Quit();
}

void FlowController::setPreviewResolution(const QSize & size)
{
    camera->setPreviewResolution(size);
}

void FlowController::motorStateChanged(const MotorState & status)
{
    mstate = status;
}

void FlowController::handleActionFinished()
{
    isMotorActionFinished = true;
}

void FlowController::takePhotoProcess()
{
    QtConcurrent::run([=]() {
        logdebug << "begin take photo process.";

        while(_isProcessRunning)
        {
            if(_isProcessPause)
            {
                sleep(1000);
                continue;
            }

            stepIndex ++;

            isMotorActionFinished = false;
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

            while(!_isImageSaved)
                sleep(50);

            if(targetPosition > endPosition)
            {
                loginfo << "scan finished, image count:" << stepIndex ;
                break;
            }

            if(scanmode == 1)
            {
                targetPosition += anglePerPulse * angleStep;
            }
            else
            {
                targetPosition += pixelPerPulse * pixelStep;
            }
        }

        logdebug << "stop take photo process.";
    });

}

