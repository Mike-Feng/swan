/**************************************************************************
**
** Copyright (C) 2023 Mike Feng
** Blog: https://www.cnblogs.com/crazyghostvon/
**
** This file (serialportworker.h) is part of the Swan project.
**
**************************************************************************/
#ifndef SERIALPORTWORKER_H
#define SERIALPORTWORKER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <stable.h>

namespace Cg { namespace Swan { namespace Adapter {

class SerialPortWorker : public QObject
{
    Q_OBJECT
public:
    explicit SerialPortWorker(QObject *parent = nullptr);
    bool isReadyRead(){ return _isReadyRead;}
    bool isConnected(){ return _isConnected;}
    QByteArray readData();
    void Quit();


    volatile bool     _isReadyRead = false;

public slots:
    void init();
    void writeSerialport(const QByteArray &);
    void handleClose();

signals:
private slots:
    void handleReady();
private:
    QSerialPort  *client = nullptr;
    bool     _isConnected = false;
    QByteArray  recivedata;
};
} } }
#endif // SERIALPORTWORKER_H
