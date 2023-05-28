/**************************************************************************
**
** Copyright (C) 2023 Mike Feng
** Blog: https://www.cnblogs.com/crazyghostvon/
**
** This file motoradapter.cpp is part of the Swan project.
**
**************************************************************************/
#include "motoradapter.h"
using namespace Cg::Swan::Adapter;

MotorAdapter::MotorAdapter(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<MotorActionParam>("MotorActionParam");
    qRegisterMetaType<MotorActionParam>("MotorActionParam&");
    qRegisterMetaType<MotorState>("MotorState");
    qRegisterMetaType<MotorState>("MotorState&");
}

void MotorAdapter::init()
{
    client = new SerialPortWorker();
    tClient = new QThread();
    client->moveToThread(tClient);
    connect(tClient, &QThread::started, client, &SerialPortWorker::init);
    connect(this, &MotorAdapter::newSerialportData, client,
            &SerialPortWorker::writeSerialport);
    connect(this, &MotorAdapter::closePort, client,
            &SerialPortWorker::handleClose);

    tClient->start(QThread::TimeCriticalPriority);
    statusChecker = new QTimer(this);
    statusChecker->setInterval(100);
    connect(statusChecker, &QTimer::timeout, this, &MotorAdapter::ReportStatus);

    while(!client->isConnected());

    initMotorDriver();

    emit motorStateChanged(_status);
}

bool MotorAdapter::initMotorDriver()
{
    bool isOk = write(0x20, 01);

    if(!isOk)
    {
        qCritical() << "change 01 to 01 failed, try 00 to 01.";
        //isOk = write(0x20, 01, 00);

        if(!isOk)
        {
            logerror << "change 00 to 01 failed.";
            return false;
        }
        else
        {
            logdebug <<  "change 00 to 01 success.";
            _status.isConnected = true;
        }
    }
    else
    {
        logdebug <<  "slave id is 01.";
        _status.isConnected = true;
    }

    setSpeed(SWAN_SPEED);

    return true;
}

void MotorAdapter::setSpeed(qint32 speed)
{
    // set both reset speed and run speed
    QVector<quint16> speedv;
    speedv.append(((speed >> 16) & 0xffff));
    speedv.append(speed & 0xffff);

    QByteArray sp24 = read(0x24, 2);
    const char *spd = sp24.constData();
    int num = qFromBigEndian<int>(spd);
    if(num != speed)
    {
        logdebug << "set motor speed24  to " << speed;
        write(0x24, speedv);
    }

    QByteArray sp26 = read(0x24, 2);
    const char *spd26 = sp26.constData();
    num = qFromBigEndian<int>(spd26);
    if(num != speed)
    {
        logdebug << "set motor speed26  to " << speed;
        write(0x26, speedv);
    }
}

void MotorAdapter::stopRun(MotorDirection dir)
{
    quint16 p = 0; // 0 indicates stop condition is flag level is high
    p |= (1 << 12);
    if(dir == MD_Clock)
    {
        p |= (1 << 1); // enable right flag
        p |= (1 << 4); // clock

        logdebug << "stop turn clock";
    }
    else
    {
        p |= (1 << 0); // enable left flag
        logdebug << "stop turn counterclock";
    }

    bool d = write(0x2a, p);
    if(d)
    {
        logdebug << "write stop success.";
    }
    else
    {
        logerror << "write stop failed.";
    }
}

void MotorAdapter::ReportStatus()
{
    readStatus();
    if((!conditions.isCheckLeftFlag || _status.isLeftFlagTriggered == conditions.leftFlagStatus)
        && (!conditions.isCheckRightFlag || _status.isRightFlagTriggered == conditions.rightFlagStatus)
        && (!conditions.isCheckRunningStatus || _status.isStopped == conditions.isStopped)
        && (!conditions.isCheckPosition || _status.position == conditions.position))
    {
        statusChecker->stop();
        logdebug << "current position:" << _status.position *1.8 / 200.0 / 10; // because the reduction-gear ratio is 10.
        emit actionFinished();
    }

    emit livePosition(_status.position);
}

void MotorAdapter::Quit()
{
    if(statusChecker->isActive())
        statusChecker->stop();

    stopRun(MotorDirection::MD_Clock);

    emit closePort();
    logdebug << "motor quit.";
}

void MotorAdapter::execute(const MotorActionParam & param)
{    
    conditions.isCheckPosition = false;
    conditions.isCheckLeftFlag = false;
    conditions.isCheckRightFlag = false;
    conditions.isCheckRunningStatus = false;

    switch (param.MAction) {
    case MotorAction::MA_Default:
    {
        logdebug << "No Motor Action";
        break;
    }
    case MotorAction::MA_GotoReady:
    case MotorAction::MA_RunOnDistance:
    {
        // run in distance mode
        QVector<quint16> dist(2);
        quint16 high = 0;
        qint32 data = param.data32;
        if(param.MAction == MotorAction::MA_GotoReady)
        {
            data = 0; // go to original position
        }

        if(data < 0)
        {
            // run in counterclockwise.
            high = (1 << 15);
            data = -1 * data; // change it to positive value
        }

        // the max distance is 0xffffff
        data = data & 0xffffff;
        // setup condition
        conditions.position = param.data32;
        conditions.isCheckPosition = true;

        high |= (data >> 16) & 0xff;
        quint16 low = data & 0xffff;

        //if(param.EnableLeftFlag) high |= (1 << 8);
        //if(param.EnableRightFlag) high |= (1 << 9);
        /*************/
        // when run in distance mode, ignore the left, right flag
        //if(param.StopOnLeftFlagLow) high |= (1 << 10);
        //if(param.StopOnRightFlagLow) high |= (1 << 11);
        /**************/

        dist[0] = high;
        dist[1] = low;

        bool isOk = write(0x2b, dist);
        if(isOk)
        {
            logdebug << "run MA_RunOnDistance success.";
        }
        else
        {
            logerror << "run MA_RunOnDistance failed.";
        }
        break;
    }
    case MotorAction::MA_MoveZero:
    {
        qint16 data = 0;
        if(param.Direction == MD_Clock)data |= (1 << 4);
        write(0x2a, data);
        break;
    }
    case MotorAction::MA_FindBarrier:
    {
        qint16 data = 0;
        if(param.EnableLeftFlag) data |= (1 << 0);
        if(param.EnableRightFlag) data |= (1 << 1);
        if(param.StopOnLeftFlagLow) data |= (1 << 2);
        if(param.StopOnRightFlagLow) data |= (1 << 3);
        if(param.Direction == MD_Clock)data |= (1 << 4);

        if(param.MAction == MotorAction::MA_FindBarrier)
        {
            // record a zero position when touch the flag
            data |= (1 << 12);

            conditions.position = 0;
            conditions.isCheckPosition = true;
        }
        else
        {
            conditions.isStopped = true;
            if(param.StopOnLeftFlagLow || param.StopOnRightFlagLow)
                conditions.isCheckRunningStatus = true;
            else
                conditions.isCheckRunningStatus = false;
        }
        write(0x2a, data);
        break;
    }
    case MotorAction::MA_StopRun:
    {
        stopRun(param.Direction);
        conditions.isStopped = true;
        conditions.isCheckRunningStatus = true;

        break;
    }
    case MotorAction::MA_SetSpeed:
    {
        setSpeed(param.data32);
        break;
    }
    case MotorAction::MA_ReadStatus:
    {
        readStatus();
        emit motorStateChanged(_status);
        break;
    }
    case MotorAction::MA_Emergency:
    {
        write(0x2d, 0);
        conditions.isStopped = true;
        conditions.isCheckRunningStatus = true;
        break;
    }

    }
    if(statusChecker->isActive() == false)
        statusChecker->start();
}

void MotorAdapter::readStatus()
{
    QByteArray data = read(0x2e, 1);
    if(data.length() == 2)
    {
        qint8 high = data[0];
        qint8 low = data[1];
        _status.isStopped = ((high >> 7) & 1) == 1;
        _status.isTemp120 = ((high >> 6) & 1) == 1;
        _status.isTooHot = ((high >> 5) & 1) == 1;
        _status.current = (high & (1 << 0)) + (low >> 4);
        _status.isRightFlagTriggered = ((low >> 1) & 1) == 1;
        _status.isLeftFlagTriggered = ((low >> 0) & 1) == 1;

        QByteArray position = read(0x2b, 2);
        if(position.length() == 4)
        {
            int p0 = (position[0] & (1 << 7));
            int p1 = (position[1] << 16) & 0xff0000;
            int p2 = (position[2] << 8 ) & 0xff00;
            int p3 = (position[3] << 0 ) & 0xff;

            // the max distance is 0xffffff
            _status.position = (p1 | p2 | p3);

            // negative direction
            if(p0 > 0)
                _status.position *= -1;
        }
    }
    else
    {
        logerror << "read status failed.";
    }
}

bool MotorAdapter::write(qint8 addr, quint16 data, qint8 slaveid)
{
    QByteArray raw(8, 0); // initialized with 8 zeros, with crc
    raw[0] = slaveid;
    raw[1] = 0x06;
    raw[2] = 0;
    raw[3] = addr;
    raw[4] = data >> 8;
    raw[5] = data & 0xff;
    appendModbusCRC16(&raw);

    QByteArray ret = SendDataSync(raw, 8);

    if(ret.length() == 0)
        return false;
    int dh = (ret[4] << 8);
    int dl = ret[5];
    if((dh + dl) == data)
    {
        return true;
    }
    else
        return false;

}

bool MotorAdapter::write(qint8 addr, QVector<quint16> data)
{
    int len = data.length();

    QByteArray raw(7 + len * 2 + 2, 0); // initialized with 8 zeros
    raw[0] = 1;
    raw[1] = 0x10;
    raw[2] = 0;
    raw[3] = addr;

    raw[4] = 0; // by default, the data length is less than 0xff
    raw[5] = (len & 0xff); // data size in qint16
    raw[6] = (len << 1); // data size in bytes
    // append data in the array
    int i = 0;
    foreach(qint16 d16, data)
    {
        raw[7 + i] = (d16 >> 8);
        raw[8 + i] = (d16 & 0xff);
        i+=2;
    }

    // calculate CRC
    appendModbusCRC16(&raw);

    QByteArray ret = SendDataSync(raw, 8);

    if(ret.length() == 0)
        return false;

    int dh = (ret[4] << 8);
    int dl = ret[5];
    if((dh + dl) == len)
    {
        return true;
    }
    else
        return false;
}

QByteArray MotorAdapter::read(qint8 addr, int len)
{
    len &= 0xff; //by default, read data is not more than 0xff
    QByteArray raw(8, 0); // initialized with 8 zeros, with CRC
    raw[0] = 1;
    raw[1] = 0x03;
    raw[2] = 0;
    raw[3] = addr;
    raw[4] = 0; // by default, read data is not more than 0xff
    raw[5] = len & 0xff;
    appendModbusCRC16(&raw);

    QByteArray ret = SendDataSync(raw, len * 2 + 5);
    int rLen = 0;
    if(ret.length() > 2)
        rLen = ret[2];

    if( rLen == len * 2)
    {
        QByteArray data = ret.mid(3, rLen);
        return data;
    }
    else
        return QByteArray(0);
}

QByteArray MotorAdapter::SendDataSync(const QByteArray sendData, uint expectReceiveLen)
{
    sleep(20);
    emit newSerialportData(sendData);
    sleep(20);
    QDateTime starttime = QDateTime::currentDateTime();

    QByteArray ba;

    while (true) {
        QDateTime endtime = QDateTime::currentDateTime();
        int msecsTo = starttime.msecsTo(endtime);
        if(msecsTo > 200 && ba.length() < expectReceiveLen)
        {
            logerror << "modbus timeout, no enough data. received data:" << ba.toHex();
            ba.clear();
            break;
        }
        if(client->_isReadyRead)
        {
            ba += client->readData();
        }
        if(ba.length() >= expectReceiveLen)
        {
            client->_isReadyRead = false;
            break;
        }
        sleep(10);
    }

    return ba;
}


qint16 MotorAdapter::appendModbusCRC16(QByteArray * senddata)
{
    int len=senddata->size() - 2;
    uint16_t wcrc=0xFFFF;//预置16位crc寄存器，初值全部为1
    uint8_t temp;//定义中间变量
    int i=0,j=0;//定义计数
    for(i=0;i<len;i++)//循环计算每个数据
    {
        temp=senddata->at(i);
        wcrc^=temp;
        for(j=0;j<8;j++){
            //判断右移出的是不是1，如果是1则与多项式进行异或。
            if(wcrc&0x0001){
                wcrc>>=1;//先将数据右移一位
                wcrc^=0xA001;//与上面的多项式进行异或
            }
            else//如果不是1，则直接移出
                wcrc>>=1;//直接移出
        }
    }
    temp=wcrc;//crc的值
    senddata->data()[len + 0] = wcrc & 0xff;
    senddata->data()[len + 1] = (wcrc >> 8);
    return wcrc;
}
