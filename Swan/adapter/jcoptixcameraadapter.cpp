/**************************************************************************
**
** Copyright (C) 2023 Mike Feng
** Blog: https://www.cnblogs.com/crazyghostvon/
**
** This file (jcoptixcameraadapter.cpp) is part of the Swan project.
**
**************************************************************************/
#include "jcoptixcameraadapter.h"

using namespace Cg::Swan::Adapter;

//SDK
int                     g_hCamera = -1;     //设备句柄
unsigned char           * g_pRawBuffer=NULL;     //raw数据
unsigned char           * g_pRgbBuffer=NULL;     //处理后数据缓存区
tSdkFrameHead           g_tFrameHead;       //图像帧头信息
tSdkCameraCapbility     g_tCapability;      //设备描述信息

int                     g_SaveParameter_num=0;    //保存参数组
int                     g_SaveImage_type=0;         //保存图像格式

Width_Height            g_W_H_INFO;         //显示画板到大小和图像大小
BYTE                    *g_readBuf=NULL;    //画板显示数据区
int                     g_read_fps=0;       //统计读取帧率
int                     g_disply_fps=0;     //统计显示帧率

JCOptixCameraAdapter::JCOptixCameraAdapter(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<JCCameraActionParam>("JCCameraActionParam");
    qRegisterMetaType<JCCameraActionParam>("JCCameraActionParam&");
}

void JCOptixCameraAdapter::init()
{
    if(initSDK()==-1){
        logerror << "init JC camera SDK failed.";
        return;
    }

    m_thread = new CaptureThread(this);
    //start task thread
    connect(m_thread, &CaptureThread::captured,
            this, &JCOptixCameraAdapter::Image_process, Qt::BlockingQueuedConnection);

    if(initParam() == false)
        return;

    m_thread->start();
    //m_thread->pause();
}

int JCOptixCameraAdapter::initSDK()
{

    int                     iCameraCounts = 4;
    int                     iStatus=-1;
    tSdkCameraDevInfo       tCameraEnumList[4];

    //sdk初始化  0 English 1中文
    CameraSdkInit(1);

    //枚举设备，并建立设备列表
    CameraEnumerateDevice(tCameraEnumList,&iCameraCounts);

    //没有连接设备
    if(iCameraCounts==0){
        return -1;
    }

    //相机初始化。初始化成功后，才能调用任何其他相机相关的操作接口
    iStatus = CameraInit(&tCameraEnumList[0],-1,-1,&g_hCamera);

    //初始化失败
    if(iStatus!=CAMERA_STATUS_SUCCESS){
        return -1;
    }
    //获得相机的特性描述结构体。该结构体中包含了相机可设置的各种参数的范围信息。决定了相关函数的参数
    CameraGetCapability(g_hCamera,&g_tCapability);

    g_pRgbBuffer = (unsigned char*)malloc(g_tCapability.sResolutionRange.iHeightMax*g_tCapability.sResolutionRange.iWidthMax*3);
    g_readBuf = (unsigned char*)malloc(g_tCapability.sResolutionRange.iHeightMax*g_tCapability.sResolutionRange.iWidthMax*3);

    /*让SDK进入工作模式，开始接收来自相机发送的图像
    数据。如果当前相机是触发模式，则需要接收到
    触发帧以后才会更新图像。    */
    CameraPlay(g_hCamera);



    /*
        设置图像处理的输出格式，彩色黑白都支持RGB24位
    */
    if(g_tCapability.sIspCapacity.bMonoSensor){
        CameraSetIspOutFormat(g_hCamera,CAMERA_MEDIA_TYPE_MONO8);
    }else{
        CameraSetIspOutFormat(g_hCamera,CAMERA_MEDIA_TYPE_RGB8);
    }
    return 0;
}


bool JCOptixCameraAdapter::initParam()
{
    bool ret = setResolution(1920, 1080);
    ret &= setTriggerMode(1);
    ret &= setExposure(2000);

    g_SaveImage_type=3;

    return ret;
}

bool JCOptixCameraAdapter::setResolution(int w, int h)
{
    /********set resolution  ********/
    tSdkImageResolution   *pImageSizeDesc=g_tCapability.pImageSizeDesc;// 预设分辨率选择

    for(int index = 0; index < g_tCapability.iImageSizeDesc; index++)
    {
        if(pImageSizeDesc[index].iWidth == w && pImageSizeDesc[index].iHeight == h)
        {
            g_W_H_INFO.sensor_width=pImageSizeDesc[index].iWidth;
            g_W_H_INFO.sensor_height=pImageSizeDesc[index].iHeight;
            g_W_H_INFO.buffer_size= g_W_H_INFO.sensor_width*g_W_H_INFO.sensor_height;


            if(pImageSizeDesc[index].iWidthZoomSw){
                g_W_H_INFO.sensor_width=pImageSizeDesc[index].iWidthZoomSw;
                g_W_H_INFO.sensor_height=pImageSizeDesc[index].iHeightZoomSw;
            }

            if(g_W_H_INFO.sensor_width < g_W_H_INFO.display_width){
                g_W_H_INFO.xOffsetFOV=(g_W_H_INFO.display_width-g_W_H_INFO.sensor_width)/2;
            }else{
                g_W_H_INFO.xOffsetFOV=0;
            }
            if(g_W_H_INFO.sensor_height < g_W_H_INFO.display_height){
                g_W_H_INFO.yOffsetFOV=(g_W_H_INFO.display_height-g_W_H_INFO.sensor_height)/2;
            }else{
                g_W_H_INFO.yOffsetFOV=0;
            }
            m_thread->pause();
            //设置预览的分辨率。
            CameraSdkStatus ret = CameraSetImageResolution(g_hCamera,&(pImageSizeDesc[index]));

            if(ret == CAMERA_STATUS_SUCCESS)
            {
                logdebug << "set resolution success: " << "w=" << w << ", h=" << h;
                return true;
            }
            else
            {
                logerror << "set resolution failed: " << "w=" << w << ", h=" << h;
                return false;
            }
        }
    }
    logerror << "resolution is not support: " << "w=" << w << ", h=" << h;
    return false;
}

bool JCOptixCameraAdapter::setTriggerMode(int mode)
{
    //0表示连续采集模式；1表示软件触发模式；2表示硬件触发模式。
   CameraSdkStatus ret = CameraSetTriggerMode(g_hCamera, mode);

   QString modestr;
   if(mode == 0) modestr = "stream trigger";
   else if(mode == 1) modestr = "soft trigger";
   else if(mode == 2) modestr = "hardware trigger";

   if(ret == CAMERA_STATUS_SUCCESS)
   {
       logdebug << "set " << modestr << " success. ";
       return true;
   }
   else
   {
       logdebug << "set " << modestr << " failed. ";
       return false;
   }
}

bool JCOptixCameraAdapter::setExposure(int time)
{
    double          m_fExpLineTime=0;//当前的行曝光时间，单位为us
    char            buffer[16];

    /*
        获得一行的曝光时间。对于CMOS传感器，其曝光
        的单位是按照行来计算的，因此，曝光时间并不能在微秒
        级别连续可调。而是会按照整行来取舍。这个函数的
        作用就是返回CMOS相机曝光一行对应的时间。
    */
    CameraGetExposureLineTime(g_hCamera, &m_fExpLineTime);

    /*
        设置曝光时间。单位为微秒。对于CMOS传感器，其曝光
        的单位是按照行来计算的，因此，曝光时间并不能在微秒
        级别连续可调。而是会按照整行来取舍。在调用
        本函数设定曝光时间后，建议再调用CameraGetExposureTime
        来获得实际设定的值。
    */
    CameraSdkStatus ret = CameraSetExposureTime(g_hCamera,time*m_fExpLineTime);

    if(ret == CAMERA_STATUS_SUCCESS)
    {
        logdebug <<  "set JC ExposureTime:" << (time*m_fExpLineTime)/1000 << "ms  success.";
        return true;
    }
    else
    {
        logdebug <<  "set JC ExposureTime:" << (time*m_fExpLineTime)/1000 << "ms  success.";
        return false;
    }
}

void JCOptixCameraAdapter::execute(const JCCameraActionParam& param)
{
    switch (param.Action) {
    case JCCameraAction::JC_SetExposure:
    {
        setExposure(param.IntParamValue);
        break;
    }
    case JCCameraAction::JC_TriggerOnce:
    {
        CameraSoftTrigger(g_hCamera);
        break;
    }
    case JCCameraAction::JC_Preview:
    {
        setTriggerMode(0);
        m_thread->pause();
        break;
    }
    }
}

void JCOptixCameraAdapter::Image_process(QImage& img)
{
    if(_isSavedImage)
    {
        // save image
    }

    emit newImage(img);
}

void JCOptixCameraAdapter::Quit()
{
    m_thread->stop();
    while (!m_thread->wait(100) )
    {
        QCoreApplication::processEvents();
    }

    if(g_readBuf!=NULL){
        free(g_readBuf);
        g_readBuf=NULL;
    }

    if(g_pRgbBuffer!=NULL){
        free(g_pRgbBuffer);
        g_pRgbBuffer=NULL;
    }

    if(g_hCamera>0){
        //相机反初始化。释放资源。
        CameraUnInit(g_hCamera);
        g_hCamera=-1;
    }
    logdebug << "JCOptix quit.";
}
