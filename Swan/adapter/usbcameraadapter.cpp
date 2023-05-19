/**************************************************************************
**
** Copyright (C) 2023 Mike Feng
** Blog: https://www.cnblogs.com/crazyghostvon/
**
** This file (usbcameraadapter.cpp) is part of the Swan project.
**
**************************************************************************/
#include "usbcameraadapter.h"
using namespace Cg::Swan::Adapter;

USBCameraAdapter::USBCameraAdapter(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<USBCameraActionParam>("USBCameraActionParam");
    qRegisterMetaType<USBCameraActionParam>("USBCameraActionParam&");
}

void USBCameraAdapter::init()
{
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    logdebug << cameras.size();

    for(int i = 0;i<cameras.size();i++)
    {
        QCameraInfo ci = cameras[i];

        logdebug << ci.description();

        if(ci.description() == "PC Camera")
        {
            _camera = new QCamera(ci, this);

            _camera->setCaptureMode(QCamera::CaptureStillImage);
            _capture = new QCameraImageCapture(_camera);
            connect(_capture, &QCameraImageCapture::imageCaptured, this, &USBCameraAdapter::imageCaptured);
            _capture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
            _camera->start();
            QCameraViewfinderSettings customSetting;
            customSetting.setResolution(1920,1080);
            _camera->setViewfinderSettings(customSetting);
            QtConcurrent::run([=]() {
                while(_isStarted)
                {
                    if(_capture->isReadyForCapture())
                    {
                        _capture->capture();
                    }
                    else
                    {
                        QThread::msleep(50);
                    }
                }
            });

            break;
        }
    }

    if(_camera == nullptr)
    {
        qCritical() << ("PC Camera not found");
    }
}


void USBCameraAdapter::execute(const USBCameraActionParam & param)
{

}

void USBCameraAdapter::imageCaptured(int id, const QImage &preview)
{
    //logdebug << id << "  " << now;
    emit newImage(preview);
}

void USBCameraAdapter::Quit()
{
    _isStarted = false;
    _camera->stop();
    QThread::msleep(100);
    logdebug << "camera quit.";
}

void USBCameraAdapter::setPreviewResolution(const QSize & size)
{
    if(_camera->status() == QCamera::ActiveStatus)
    {
        int w = size.width(); int h = size.height();
        QList<QCameraViewfinderSettings> settings = _camera->supportedViewfinderSettings();
        logdebug << "camera supported resolution:";
        QSize target(320, 240);
        foreach(QCameraViewfinderSettings s, settings)
        {
            int sw = s.resolution().width();
            int sh = s.resolution().height();
            logdebug << "width:" << sw << ", height:" << sh;
            if(w >= sw && h >= s.resolution().height()
                && target.width() <= sw && target.height() <= sh )
            {
                target = s.resolution();
            }
        }
        logdebug << "set camera resolution to  width:" << target.width() << ", height:" << target.height();
        QCameraViewfinderSettings customSetting;
        customSetting.setResolution(target);
        _camera->setViewfinderSettings(customSetting);
        _camera->start();
    }
    else
    {
        logerror << "Camera is not active, status:" << _camera->status() << ", cannot set resolution.";
    }
}
