/**************************************************************************
**
** Copyright (C) 2023 Mike Feng
** Blog: https://www.cnblogs.com/crazyghostvon/
**
** This file (serialportworker.cpp) is part of the Swan project.
**
**************************************************************************/
#include "serialportworker.h"
using namespace Cg::Swan::Adapter;

SerialPortWorker::SerialPortWorker(QObject *parent) : QObject(parent)
{
}

void SerialPortWorker::init()
{
    client = new QSerialPort();
    connect(client, &QSerialPort::readyRead, this, &SerialPortWorker::handleReady);
    client->setBaudRate(QSerialPort::Baud9600);
    client->setDataBits(QSerialPort::Data8);
    client->setStopBits(QSerialPort::OneStop);
    client->setParity(QSerialPort::NoParity);
    client->setFlowControl(QSerialPort::NoFlowControl);
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        logdebug << "portname:" << info.portName() << "description:" << info.description();
        if(info.description() == "USB-SERIAL CH340")
        {
            client->setPort(info);
            logdebug << "try serialport " << info.portName();
            break;
        }
    }


    _isConnected = client->open(QIODevice::ReadWrite);

    if(_isConnected)
    {
        logdebug << client->portName() << "is openned." ;
    }
    else
    {
        logdebug << "failed to open serialport " << client->portName();
    }
}

void SerialPortWorker::writeSerialport(const QByteArray & d)
{
    _isReadyRead = false;
    if(_isConnected)
    {
        client->clear();
        int len = client->write(d);
        if(len == d.length())
            logdebug << "W:" << d.toHex(' ');
        else
        {
            logerror << "send data failed: " << d.toHex(' ');
            logerror << client->errorString();
        }
    }
}

void SerialPortWorker::handleClose()
{
    client->close();
    logdebug << client->portName() << " is closed.";
}

void SerialPortWorker::handleReady()
{
    recivedata+=client->readAll();
    logdebug << "R:" << recivedata.toHex(' ');
    _isReadyRead = true;

}
QByteArray SerialPortWorker::readData()
{
    QByteArray temp = recivedata;
    recivedata.clear();
    return temp;
}

void SerialPortWorker::Quit()
{
    client->close();
    logdebug << "serial port is closed.";
}
