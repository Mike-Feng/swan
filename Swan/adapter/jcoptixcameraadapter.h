/**************************************************************************
**
** Copyright (C) 2023 Mike Feng
** Blog: https://www.cnblogs.com/crazyghostvon/
**
** This file (jcoptixcameraadapter.h) is part of the Swan project.
**
**************************************************************************/
#ifndef JCOPTIXCAMERAADAPTER_H
#define JCOPTIXCAMERAADAPTER_H

#include <stable.h>
#include <QObject>
#include <QtConcurrent>

#include <windows.h>
#include "CameraApi.h"
#include "capturethread.h"

namespace Cg { namespace Swan { namespace Adapter {

typedef struct _WIDTH_HEIGHT{
    int     display_width;
    int     display_height;
    int     xOffsetFOV;
    int     yOffsetFOV;
    int     sensor_width;
    int     sensor_height;
    int     buffer_size;
}Width_Height;

enum JCCameraAction
{
    JC_Default,
    JC_TriggerOnce,
    JC_SetExposure,
    JC_TakeSnap,
    JC_Preview
};

struct JCCameraActionParam
{
    JCCameraAction Action;
    int IntParamValue;
};

class JCOptixCameraAdapter : public QObject
{
    Q_OBJECT
public:
    explicit JCOptixCameraAdapter(QObject *parent = nullptr);

    bool setResolution(int w, int h);
    bool setTriggerMode(int mode);
    bool setExposure(int time);
    void Quit();
signals:
    void newImage(const QImage &preview);

public slots:
    void init();
    void execute(const JCCameraActionParam & param);

private slots:
    void Image_process(QImage& img);

private:
    int initSDK();
    bool initParam();


private:
    bool                 _isStarted = true;
    CaptureThread        * m_thread;
    bool                 _isSavedImage = false;

};

} } } // namespace Cg::Swan::Adapter
#endif // JCOPTIXCAMERAADAPTER_H
