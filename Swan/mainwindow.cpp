/**************************************************************************
**
** Copyright (C) 2023 Mike Feng
** Blog: https://www.cnblogs.com/crazyghostvon/
**
** This file mainwindow.cpp is part of the Swan project.
**
**************************************************************************/
#pragma execution_character_set("utf-8")

#include "mainwindow.h"
#include <QEvent>
#include <QLayout>
#include <QPen>
#include <QBrush>
#include <QColorDialog>
#include <QPalette>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
    _gvCamera(new SwanGraphicsView(this)),
    _gvSensor(new SwanGraphicsView(this)),
    _sceneCamera(new QGraphicsScene(this)),
    _sceneSensor(new QGraphicsScene(this)),
    controller(new FlowController())
{
    ui->setupUi(this);

    // replace the graphics view with the custom view
    ui->mainLayout->replaceWidget(ui->gvCamera, _gvCamera);
    ui->mainLayout->replaceWidget(ui->gvSensor, _gvSensor);
    delete ui->gvCamera;
    delete ui->gvSensor;
    _gvCamera->setScene(_sceneCamera);
    _gvSensor->setScene(_sceneSensor);

    QObject::connect(_gvCamera,SIGNAL(mouseMovePoint(QPoint)),
                     this, SLOT(mouseMoved(QPoint)));
    QObject::connect(_gvCamera,SIGNAL(mousePress(QPoint)),
                     this, SLOT(mousePressed(QPoint)));
    QObject::connect(_gvCamera,SIGNAL(mouseRelease(QPoint)),
                     this, SLOT(mouseReleased(QPoint)));
    QObject::connect(_gvCamera,SIGNAL(enter()),
                     this, SLOT(entered()));
    QObject::connect(_gvCamera,SIGNAL(leave()),
                     this, SLOT(leaved()));
    QObject::connect(_gvSensor,SIGNAL(mouseMovePoint(QPoint)),
                     this, SLOT(mouseMoved(QPoint)));
    QObject::connect(_gvSensor,SIGNAL(mousePress(QPoint)),
                     this, SLOT(mousePressed(QPoint)));
    QObject::connect(_gvSensor,SIGNAL(mouseRelease(QPoint)),
                     this, SLOT(mouseReleased(QPoint)));
    QObject::connect(_gvSensor,SIGNAL(enter()),
                     this, SLOT(entered()));
    QObject::connect(_gvSensor,SIGNAL(leave()),
                     this, SLOT(leaved()));

    QPen pen;
    pen.setColor(SWAN_MOUSECOLOR);
    pen.setWidth(3);
    pen.setStyle(Qt::DashLine);

    _xlineCamera = _sceneCamera->addLine(0,0,0,0, pen);
    _ylineCamera = _sceneCamera->addLine(0,0,0,0, pen);
    _xlineSensor = _sceneSensor->addLine(0,0,0,0, pen);
    _ylineSensor = _sceneSensor->addLine(0,0,0,0, pen);

    QPen penbase;
    penbase.setColor(SWAN_LINECOLOR);
    penbase.setWidth(2);

    _ybase1 = _sceneCamera->addLine(0,0,0,0, penbase);
    _ybase2 = _sceneCamera->addLine(0,0,0,0, penbase);
    _xbase = _sceneCamera->addLine(0,0,0,0, penbase);

    _xlineCamera->setZValue(10);
    _ylineCamera->setZValue(10);
    _xlineSensor->setZValue(10);
    _ylineSensor->setZValue(10);

    _ybase1->setZValue(10);
    _ybase2->setZValue(10);
    _xbase->setZValue(10);

    _shadowArea = _sceneCamera->addPath(QPainterPath());
    _shadowArea->setBrush(QColor(0,0,0,127));
    _shadowArea->setZValue(9);
    _shadowArea->setPen(QColor(0,0,0,0));
    _shadowArea->setVisible(false);

    QPen rpen;
    rpen.setColor(SWAN_RECTCOLOR);
    rpen.setWidth(1);
    _rect = _sceneCamera->addRect(0,0,0,0,rpen);
    _rect->setZValue(10);
    _rect->setBrush(QColor(255,255,255,0));

    tcontroller = new QThread();
    controller->moveToThread(tcontroller);
    connect(tcontroller, &QThread::started, controller, &FlowController::init);
    tcontroller->start();

    connect(controller, &FlowController::newImage, this, &MainWindow::handleNewImage);
    connect(this, &MainWindow::doAction, controller, &FlowController::execute);

    initUI();

    qDebug() << ("application started.");
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::closeEvent(QCloseEvent *e)
{
    qDebug() << "quit application";
    controller->Quit();
}
void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    QApplication::processEvents();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    QApplication::processEvents();

    _gvCamera->setSceneRect(0, 0, _gvCamera->width(), _gvCamera->height());
    _gvSensor->setSceneRect(0, 0, _gvSensor->width(), _gvSensor->height());
    setShadowArea();

    QSize imgsize = SWAN_IMAGESIZE;
    scaleRateX = imgsize.width() * 1.0 / _sceneCamera->width();
    scaleRateY = imgsize.height() * 1.0 / _sceneCamera->height();

    int y1 = SWAN_YLINE1 / scaleRateY;
    int y2 = SWAN_YLINE2 / scaleRateY;
    int x = SWAN_XCENTER / scaleRateX;
    _ybase1->setLine(0,y1,_sceneCamera->width(),y1);
    _ybase2->setLine(0,y2,_sceneCamera->width(),y2);
    _xbase->setLine(x,0,x,_sceneCamera->height());
}

void MainWindow::initUI()
{
    int sm = controller->scanMode();
    switch (sm) {
    case 1:
        ui->radScanByDeg->setChecked(true);
        break;
    case 2:
        ui->radScanByPixel->setChecked(true);
        break;
    }

    setButtonColor(SWAN_LINECOLOR);
}

QPoint MainWindow::restoreCoor(const QPoint& qp)
{
    QPoint p(qp.x() * scaleRateX, qp.y() * scaleRateY );
    return p;
}
QPoint MainWindow::restoreCoor(const QPointF& qpf)
{
    QPoint p(qpf.x() * scaleRateX, qpf.y() * scaleRateY );
    return p;
}

void MainWindow::handleNewImage(const QImage & img, ImageSource imgs)
{
    QGraphicsScene * temp;
    switch (imgs) {
    case ImageSource::IS_Camera:
    {
        temp = _sceneCamera;
        if(_imageItemCamera)
        {
            temp->removeItem(_imageItemCamera);
            delete _imageItemCamera;
            _imageItemCamera = 0;
        }

        QImage newImg = img.scaled(_gvCamera->width(), _gvCamera->height());
        _imageItemCamera = temp->addPixmap(QPixmap::fromImage(newImg));

        break;
    }
    case ImageSource::IS_Sensor:
    {
        temp = _sceneSensor;
        if(_imageItemSensor)
        {
            temp->removeItem(_imageItemSensor);
            delete _imageItemSensor;
            _imageItemSensor = 0;
        }
        QImage newImg2 = img.scaled(_gvSensor->width(), _gvSensor->height());
        _imageItemSensor = temp->addPixmap(QPixmap::fromImage(newImg2));
        break;
    }
    }
}

void MainWindow::on_btnCheckBarrier_clicked()
{
    ActionParam param;
    param.target = MotorTarget;
    param.motorParam.MAction = MA_FindBarrier;
    param.motorParam.Direction = MD_CounterClock;

    emit doAction(param);
}

void MainWindow::on_btnStartStop_clicked()
{
    ActionParam param;
    param.target = FlowTarget;
    QString text;

    if(!controller->isProcessRunning())
    {
        text = ("停止");
        param.flowParam.fAction = FA_Run;
    }
    else
    {
        text = ("开始");
        param.flowParam.fAction = FA_Stop;
    }

    ui->btnStartStop->setText(text);

    emit doAction(param);
}

void MainWindow::on_btnPause_clicked()
{
    ActionParam param;
    param.target = FlowTarget;
    QString text;

    if(!controller->isProcessPause())
    {
        text = ("继续");
        param.flowParam.fAction = FA_Resume;
    }
    else
    {
        text = ("暂停");
        param.flowParam.fAction = FA_Pause;
    }

    ui->btnPause->setText(text);
    emit doAction(param);
}

void MainWindow::on_btnHome_clicked()
{
    ui->btnStartStop->setText(("开始"));
    ui->btnPause->setText(("暂停"));

    //1. stop the previous task
    ActionParam param;
    param.target = FlowTarget;
    param.flowParam.fAction = FA_Stop;

    emit doAction(param);

    //2. make the motor to go home
    param.target = MotorTarget;
    param.motorParam.MAction = MA_GotoReady;

    emit doAction(param);
}

void MainWindow::on_radScanByDeg_toggled(bool checked)
{
    //scanmode: 1-> scan by degree; 2->scan by pixels
    if(checked)
        controller->setScanMode(1);
}

void MainWindow::on_radScanByPixel_toggled(bool checked)
{
    //scanmode: 1-> scan by degree; 2->scan by pixels
    if(checked)
        controller->setScanMode(2);
}

void MainWindow::on_btnTurnLeft_pressed()
{
    ActionParam param;
    param.target = MotorTarget;
    param.motorParam.MAction = MA_RunOnSpeed;
    param.motorParam.Direction = MD_CounterClock;

    emit doAction(param);
}

void MainWindow::on_btnTurnLeft_released()
{
    ActionParam param;
    param.target = MotorTarget;
    param.motorParam.MAction = MA_StopRun;

    emit doAction(param);
}

void MainWindow::on_btnTurnRight_pressed()
{
    ActionParam param;
    param.target = MotorTarget;
    param.motorParam.MAction = MA_RunOnSpeed;
    param.motorParam.Direction = MD_Clock;

    emit doAction(param);
}

void MainWindow::on_btnTurnRight_released()
{
    ActionParam param;
    param.target = MotorTarget;
    param.motorParam.MAction = MA_StopRun;

    emit doAction(param);
}
void MainWindow::mouseMoved(QPoint point)
{
    int x = point.x();
    int y = point.y();
    x = x < 0 ? 0 : x;
    y = y < 0 ? 0 : y;

    if(sender() == _gvCamera)
    {
        x = x > _sceneCamera->width() ? _sceneCamera->width() : x;
        y = y > _sceneCamera->height() ? _sceneCamera->height() : y;
        _xlineCamera->setLine(x,0,x,_sceneCamera->height());
        _ylineCamera->setLine(0,y,_sceneCamera->width(),y);
        if(isChangeRect)
        {
            QBrush b;
            b.setColor(QColor(0,255,0,255));
            QRectF r = _rect->rect();
            r.setCoords(r.x(),r.y(),x,y);
            _rect->setRect(r);
            setShadowArea();
        }
    }
    else
    {
        x = x > _sceneSensor->width() ? _sceneSensor->width() : x;
        y = y > _sceneSensor->height() ? _sceneSensor->height() : y;
        _xlineSensor->setLine(x,0,x,_sceneSensor->height());
        _ylineSensor->setLine(0,y,_sceneSensor->width(),y);
    }
    mPos = QPoint(point.x() * scaleRateX, point.y() * scaleRateY);
    updateStatusLabel();
}
void MainWindow::mousePressed(QPoint point)
{
    if(sender() == _gvCamera)
    {
        if(_shadowArea->isVisible())
        {
            isChangeRect = true;
            _rect->setRect(point.x(),point.y(),0,0);
            setShadowArea();
        }
    }
}
void MainWindow::mouseReleased(QPoint point)
{
    if(sender() == _gvCamera)
    {
        if(_shadowArea->isVisible())
        {
            isChangeRect = false;
            controller->setScanRange(scanArea);
        }
    }
}
void MainWindow::entered()
{
    if(sender() == _gvCamera)
    {
        _xlineCamera->show();
        _ylineCamera->show();
    }
    else
    {
        _xlineSensor->show();
        _ylineSensor->show();
    }
}
void MainWindow::leaved()
{
    if(sender() == _gvCamera)
    {
        _xlineCamera->hide();
        _ylineCamera->hide();
    }
    else
    {
        _xlineSensor->hide();
        _ylineSensor->hide();
    }

    updateStatusLabel();
}

void MainWindow::on_btnRangePreview_clicked()
{
    if(_shadowArea->isVisible())
        _shadowArea->hide();
    else
        _shadowArea->show();
}

void MainWindow::setShadowArea()
{
    QRegion outRe(_gvCamera->rect());
    QRect r = _rect->rect().toAlignedRect();
    QRegion innRe(r);
    QPainterPath painterPath;
    painterPath.addRegion(outRe.subtracted(innRe));
    _shadowArea->setPath(painterPath);
    scanArea = QRect(r.x() * scaleRateX, r.y() * scaleRateY,
                r.width() * scaleRateX, r.height() * scaleRateY);
}

void MainWindow::updateStatusLabel()
{
    QString s;
    if(_rect->isVisible())
        s += QString("区域:x:%1,y:%2,w:%3,h:%4   ").arg(scanArea.x()).arg(scanArea.y()).arg(scanArea.width()).arg(scanArea.height());

    if(_xlineCamera->isVisible())
        s += QString("坐标x:%1,y:%2").arg(mPos.x()).arg(mPos.y());

    ui->lblCameraXY->setText(s);

    if(_xlineSensor->isVisible())
        ui->lblSensorXY->setText(QString("坐标x:%1,y:%2").arg(mPos.x()).arg(mPos.y()));
    else
        ui->lblSensorXY->setText("");
}

void MainWindow::on_btnSensorPreview_clicked()
{
    ActionParam param;
    param.target = SensorTarget;
    param.jcParam.Action = JC_Preview;
    param.jcParam.IntParamValue = ui->txtExpo->value();

    emit doAction(param);
}

void MainWindow::on_btnPickColor_clicked()
{
    QColor lineColor = QColorDialog::getColor(Qt::yellow, this);
    SwanINI.setLineColor(lineColor);
    setButtonColor(lineColor);
}

void MainWindow::setButtonColor(QColor c)
{
    QPalette pal = ui->btnPickColor->palette();
    pal.setColor(QPalette::Button, c);
    ui->btnPickColor->setPalette(pal);
    ui->btnPickColor->setAutoFillBackground(true);
    ui->btnPickColor->setFlat(true);

    QPen penbase;
    penbase.setColor(c);
    penbase.setWidth(2);
    _ybase1->setPen(penbase);
    _ybase2->setPen(penbase);
    _xbase->setPen(penbase);
}

void MainWindow::on_txtBasey1_valueChanged(int y)
{
    y = y > _sceneCamera->height() ? _sceneCamera->height() : y;
    SwanINI.setYline1(y * scaleRateY);
    _ybase1->setLine(0,y,_sceneCamera->width(),y);
}

void MainWindow::on_txtBasey2_valueChanged(int y)
{
    y = y > _sceneCamera->height() ? _sceneCamera->height() : y;
    SwanINI.setYline2(y * scaleRateY);
    _ybase2->setLine(0,y,_sceneCamera->width(),y);
}

void MainWindow::on_txtBasex_valueChanged(int x)
{
    x = x > _sceneCamera->width() ? _sceneCamera->width() : x;

    SwanINI.setXCenter(x * scaleRateX);
    _xbase->setLine(x,0,x,_sceneCamera->height());
}
