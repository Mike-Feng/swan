/**************************************************************************
**
** Copyright (C) 2023 Mike Feng
** Blog: https://www.cnblogs.com/crazyghostvon/
**
** This file motoradapter.h is part of the Swan project.
**
**************************************************************************/
#ifndef MOTORADAPTER_H
#define MOTORADAPTER_H

#include <stable.h>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QLoggingCategory>
#include <QObject>
#include <QEventLoop>
#include <QTimer>

namespace Cg { namespace Swan { namespace Adapter {

struct MotorState{
    bool isConnected = false;
    bool isStopped = true;
    bool isTemp120 = false;
    bool isTooHot = false;
    int current = 0;
    bool isRightFlagTriggered = false;
    bool isLeftFlagTriggered = false;
    int position = -1;
};

enum MotorAction
{
    MA_Default,
    MA_MoveZero,
    MA_RunOnDistance,
    MA_FindBarrier,
    MA_GotoReady,
    MA_StopRun,
    MA_SetSpeed,
    MA_ReadStatus
};

enum MotorDirection
{
    MD_Clock,
    MD_CounterClock
};

struct MotorActionParam
{
    MotorAction MAction = MA_Default;
    MotorDirection Direction; // run in clockwise : 1 or counterclockwise : 0,
    bool EnableLeftFlag = true;  // when has load, set it to true
    bool EnableRightFlag = false; // when has load, set it to true
    bool StopOnLeftFlagLow = true; // this is determined by sensor, should not be changed in software
    bool StopOnRightFlagLow = false;// this is determined by sensor, should not be changed in software
    bool setZeroPoint = false;  // when arrived the flag, set the position as zero.
    bool relativeFlag = false;  // when run in distance mode, indicates the target position is at the right or left side.
    qint32 data32=0;
};

struct MotorFinishCondition
{
    bool isCheckLeftFlag = false;
    bool leftFlagStatus = false;
    bool isCheckRightFlag = false;
    bool rightFlagStatus = false;
    bool isCheckRunningStatus = false;
    bool isStopped = false;
    bool isCheckPosition = false;
    int position = 0;

};

class MotorAdapter : public QObject {
    Q_OBJECT
public:
    explicit MotorAdapter(QObject *parent = nullptr);
    Q_PROPERTY(bool direction READ direction WRITE setDirection);
    bool direction(){ return  _direction;}
    void setDirection(bool direc){ _direction = direc; }
    bool leftFlag(){ return  _leftFlag;}
    void setLeftFlag(bool f){ _leftFlag = f; }
    bool rightFlag(){ return  _rightFlag;}
    void setRightFlag(bool f){ _rightFlag = f; }

    int leftFlagLevel(){ return  _leftFlagLevel;}
    void setLeftFlagLevel(bool l){ _leftFlagLevel = l; }
    bool rightFlagLevel(){ return  _rightFlagLevel;}
    void setRightFlagLevel(int l){ _rightFlagLevel = l; }

    int middlePosition(){ return  _middlePosition;}
    void setMiddlePostion(int position){ _middlePosition = position; }
    int pulsePerSnapshot(){ return  _pulsePerSnapshot;}
    void setPulsePerSnapshot(int pulse){ _pulsePerSnapshot = pulse; }
    int runningSpeed(){ return  _runningSpeed;} // is for take photos
    void setRunningSpeed(int speed){ _runningSpeed = speed; }
    int debugSpeed(){ return  _debugSpeed;} // is for find the left/right flag and the middle point
    void setDebugSpeed(int speed){ _debugSpeed = speed; }

    void connectDevice();
    void Quit();

    MotorState Status() { return _status;}

public slots:
    void init();
    void execute(const MotorActionParam &);

signals:
    void motorStateChanged(const MotorState &);
    void actionFinished();

private slots:
    void ReportStatus();

private:
    bool initMotorDriver();
    void setSpeed(qint32);
    bool write(qint8 addr, quint16 data, qint8 slaveid = 01);
    bool write(qint8 addr, QVector<quint16> data);
    Q_REQUIRED_RESULT QByteArray read(qint8 addr, int len);
    QByteArray SendDataSync(const QByteArray sendData, uint expectReceiveLen);
    qint16 appendModbusCRC16(QByteArray * senddata);
    void readStatus();

private:
    QSerialPort  *client = nullptr;
    QTimer       *statusChecker = nullptr;
    MotorState _status;
    bool _direction = true; // true: clockwise, false: counterclockwise
    bool _leftFlag = true;
    int _leftFlagLevel = 1; // 1:high  0:low
    bool _rightFlag = true;
    int _rightFlagLevel = 1; // 1:high  0:low
    int _middlePosition;
    int _pulsePerSnapshot = 256; // 256 pulse per step, one step is 1.8°
    int _runningSpeed = 200 * 256; // 200*256(0xC800) pulse/second, means 1 circle/second
    int _debugSpeed = 100 * 256; // 100*256(0x6400) pulse/second, means 0.5 circle/second

    int _targetPosition = 0;

    bool isConnected = false;

    MotorFinishCondition conditions;
};

}}}    // namespace Cg::Swan::Adapter
#endif // MOTORADAPTER_H
