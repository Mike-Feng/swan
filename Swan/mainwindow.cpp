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
#include <QMessageBox>

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
    connect(controller, &FlowController::newImage, this, &MainWindow::handleNewImage);
    connect(controller, &FlowController::adapterStatus, this, &MainWindow::handleAdapterStatus);
    connect(controller, &FlowController::actionFinished, this, &MainWindow::handleActionFinished);
    connect(this, &MainWindow::doAction, controller, &FlowController::execute);

    tcontroller->start();
    initUI();

    qDebug() << ("application started.");
    QSize s(1920,1080);
    SwanINI.setImageSize(s);

}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::closeEvent(QCloseEvent *e)
{
    qDebug() << "quit application";    
    disconnect(controller, &FlowController::newImage, this, &MainWindow::handleNewImage);
    sleep(50);
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
    SWanScanMode sm = SWAN_SCANMODE;
    switch (sm) {
    case SM_RECT:
        ui->radScanByRect->setChecked(true);
        logdebug << "scan mode: SM_RECT";
        break;
    case SM_DEG:
        ui->radScanByDeg->setChecked(true);
        logdebug << "scan mode: SM_DEG";
        break;
    case SM_PIX:
        ui->radScanByPixel->setChecked(true);
        logdebug << "scan mode: SM_PIX";
        break;
    }

    ui->txtRectStart->setValue(SWAN_SCANRECT.x());
    ui->txtRectEnd->setValue(SWAN_SCANRECT.y());
    ui->txtRectStep->setValue(SWAN_RECTSTEP);
    ui->txtDegStart->setValue(SWAN_STARTANGLE);
    ui->txtDegEnd->setValue(SWAN_ENDANGLE);
    ui->txtDegStep->setValue(SWAN_ANGLESTEP);
    ui->txtPixStart->setValue(SWAN_STARTPIXEL);
    ui->txtPixEnd->setValue(SWAN_ENDPIXEL);
    ui->txtPixStep->setValue(SWAN_PIXELSTEP);

    ui->txtBasey1->setValue(SWAN_YLINE1);
    ui->txtBasey2->setValue(SWAN_YLINE2);
    ui->txtBasex->setValue(SWAN_XCENTER);
    setButtonColor(SWAN_LINECOLOR);

    ui->txtExpo->setValue(SWAN_EXPOSURE);
    ui->chkMirrorH->setChecked(SWAN_MIRRORH);
    ui->chkMirrorV->setChecked(SWAN_MIRRORV);
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

void MainWindow::enableButton(QList<QPushButton *> & buttons)
{
    ui->btnStartStop->setEnabled(buttons.contains(ui->btnStartStop));
    ui->btnPause->setEnabled(buttons.contains(ui->btnPause));
    ui->btnTurnLeft->setEnabled(buttons.contains(ui->btnTurnLeft));
    ui->btnTurnRight->setEnabled(buttons.contains(ui->btnTurnRight));
    ui->btnHome->setEnabled(buttons.contains(ui->btnHome));
    ui->btnCheckBarrier->setEnabled(buttons.contains(ui->btnCheckBarrier));
}
void MainWindow::enableButton(QPushButton * btn)
{
    QList<QPushButton *> bs;
    bs.append(btn);
    enableButton(bs);
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

void MainWindow::handleAdapterStatus(int v)
{
    switch(v)
    {
    case 1:
        QMessageBox::critical(this, "错误", "USB摄像头初始化失败。");
        ui->statusbar->showMessage("USB摄像头初始化失败。");
        ui->gboxScan->setEnabled(false);
        ui->gboxline->setEnabled(false);
        break;
    case 2:
        QMessageBox::critical(this, "错误", "工业相机初始化失败。");
        ui->statusbar->showMessage("工业相机初始化失败。");
        ui->gboxSensor->setEnabled(false);
        break;
    case 3:
        QMessageBox::critical(this, "错误", "旋转平台初始化失败。");
        ui->statusbar->showMessage("旋转平台初始化失败。");
        break;
    }

    ui->gboxOp->setEnabled(false);
}

void MainWindow::handleActionFinished(double x)
{
    logdebug << "action finished.";
    if(controller->isProcessRunning())
    {
        if(mode == SM_RECT)
        {
            //QRectF r = _rect->rect();
            //r.setRect(x / scaleRateX * 2 - SWAN_IMAGESIZE.width() / 2, r.y(), r.width(), r.height());
            //_rect->setRect(r);
            //setShadowArea();
        }
    }
    else
    {
        ui->btnStartStop->setText(("开始"));
        ui->btnPause->setText(("暂停"));
        ui->btnStartStop->setEnabled(true);
        ui->btnPause->setEnabled(false);
        ui->btnTurnLeft->setEnabled(true);
        ui->btnTurnRight->setEnabled(true);
        ui->btnHome->setEnabled(true);
        ui->btnCheckBarrier->setEnabled(true);
    }
}

void MainWindow::on_btnCheckBarrier_clicked()
{
    logdebug << "begin Check Barrier.";
    QList<QPushButton*> btns;
    enableButton(btns);

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

        switch (mode) {
        case SM_RECT:
            param.flowParam.start = ui->txtRectStart->value();
            param.flowParam.end = ui->txtRectEnd->value();
            param.flowParam.step = ui->txtRectStep->value();
            break;
        case SM_DEG:
            param.flowParam.start = ui->txtDegStart->value();
            param.flowParam.end = ui->txtDegEnd->value();
            param.flowParam.step = ui->txtDegStep->value();
            break;
        case SM_PIX:
            param.flowParam.start = ui->txtPixStart->value();
            param.flowParam.end = ui->txtPixEnd->value();
            param.flowParam.step = ui->txtPixStep->value();
            break;
        }

        if(param.flowParam.start > param.flowParam.end)
        {
            param.flowParam.start += param.flowParam.end;
            param.flowParam.end = param.flowParam.start - param.flowParam.end;
            param.flowParam.start = param.flowParam.start - param.flowParam.end;
        }
        param.flowParam.mode = mode;

        QList<QPushButton*> btns;
        btns.append(ui->btnStartStop);
        btns.append(ui->btnPause);
        enableButton(btns);
    }
    else
    {
        text = ("开始");
        param.flowParam.fAction = FA_Stop;

        QList<QPushButton*> btns;
        btns.append(ui->btnStartStop);
        btns.append(ui->btnTurnLeft);
        btns.append(ui->btnTurnRight);
        btns.append(ui->btnHome);
        btns.append(ui->btnCheckBarrier);
        enableButton(btns);
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
    logdebug << "button home is clicked.";
    ui->btnStartStop->setText(("开始"));
    ui->btnPause->setText(("暂停"));

    QList<QPushButton*> btn;
    enableButton(btn);

    //1. stop the previous task
    ActionParam param;
    param.target = FlowTarget;
    param.flowParam.fAction = FA_GotoReady;
    emit doAction(param);
}

void MainWindow::on_radScanByRect_toggled(bool checked)
{
    //scanmode: 1-> scan by degree; 2->scan by pixels
    if(checked)
    {
        _shadowArea->show();
        mode = SM_RECT;
    }
    else
    {
        _shadowArea->hide();
    }
}

void MainWindow::on_radScanByDeg_toggled(bool checked)
{
    //scanmode: 1-> scan by degree; 2->scan by pixels
    if(checked)
        mode = SM_DEG;
}

void MainWindow::on_radScanByPixel_toggled(bool checked)
{
    logdebug << "scan by pixel: " << checked;
    //scanmode: 1-> scan by degree; 2->scan by pixels
    if(checked)
        mode = SM_PIX;
}

void MainWindow::on_btnTurnLeft_pressed()
{
    logdebug << QDateTime::currentDateTime() << "turn left begin. ";
    ActionParam param;
    param.target = MotorTarget;
    param.motorParam.MAction = MA_MoveZero;
    param.motorParam.Direction = MD_CounterClock;

    emit doAction(param);
}

void MainWindow::on_btnTurnLeft_released()
{
    logdebug << QDateTime::currentDateTime() << "turn left end. ";
    ActionParam param;
    param.target = MotorTarget;
    param.motorParam.MAction = MA_StopRun;
    param.motorParam.Direction = MD_CounterClock;

    emit doAction(param);
}

void MainWindow::on_btnTurnRight_pressed()
{
    logdebug << "turn right begin. ";
    ActionParam param;
    param.target = MotorTarget;
    param.motorParam.MAction = MA_MoveZero;
    param.motorParam.Direction = MD_Clock;

    emit doAction(param);
}

void MainWindow::on_btnTurnRight_released()
{
    logdebug << "turn right end. ";
    ActionParam param;
    param.target = MotorTarget;
    param.motorParam.MAction = MA_StopRun;
    param.motorParam.Direction = MD_Clock;

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
            ui->txtRectStart->setValue(scanArea.x());
            ui->txtRectEnd->setValue(scanArea.x() + scanArea.width());
            SwanINI.setScanRect(scanArea);
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
    logdebug << "jc preview clicked. ";
    ActionParam param;
    param.target = SensorTarget;
    param.jcParam.Action = JC_TakeSnap;

    emit doAction(param);
}

void MainWindow::on_btnSensorVideo_clicked()
{
    logdebug << "jc video clicked. ";
    ActionParam param;
    param.target = SensorTarget;
    param.jcParam.Action = JC_Preview;
    if(ui->btnSensorVideo->text() == "工业相机拍摄")
    {
        param.jcParam.IntParamValue = true;
        ui->btnSensorVideo->setText("暂停拍摄");
    }
    else
    {
        param.jcParam.IntParamValue = false;
        ui->btnSensorVideo->setText("工业相机拍摄");
    }

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

void MainWindow::on_txtDegStart_valueChanged(int deg)
{
    SwanINI.setStartAngle(deg);
}

void MainWindow::on_txtDegEnd_valueChanged(int deg)
{
    SwanINI.setEndAngle(deg);
}

void MainWindow::on_txtDegStep_valueChanged(double d)
{
    SwanINI.setAngleStep(d);
}

void MainWindow::on_txtRectStep_valueChanged(int r)
{
    SwanINI.setRectStep(r);
}

void MainWindow::on_txtPixStep_valueChanged(int p)
{
    SwanINI.setPixelStep(p);
}

void MainWindow::on_txtPixEnd_valueChanged(int p)
{
    SwanINI.setEndPixel(p);
}

void MainWindow::on_txtPixStart_valueChanged(int p)
{
    SwanINI.setStartPixel(p);
}

void MainWindow::on_txtExpo_valueChanged(int expo)
{
    ActionParam param;
    param.target = SensorTarget;
    param.jcParam.Action = JC_SetExposure;
    param.jcParam.IntParamValue = ui->txtExpo->value();

    emit doAction(param);

    SwanINI.setExposure(expo);
}


void MainWindow::on_chkMirrorH_clicked(bool checked)
{
    logdebug << "set MirrorH " << checked;
    ActionParam param;
    param.target = SensorTarget;
    param.jcParam.Action = JC_SetMirror;
    int mirror = 0;
    if(checked)
        mirror = 2;
    if(ui->chkMirrorV->isChecked())
    {
        mirror +=1;
    }
    param.jcParam.IntParamValue = mirror;

    emit doAction(param);
}

void MainWindow::on_chkMirrorV_clicked(bool checked)
{
    logdebug << "set MirrorV " << checked;
    ActionParam param;
    param.target = SensorTarget;
    param.jcParam.Action = JC_SetMirror;
    int mirror = 0;
    if(ui->chkMirrorH->isChecked())
        mirror = 2;
    if(checked)
    {
        mirror +=1;
    }
    param.jcParam.IntParamValue = mirror;

    emit doAction(param);
}

void MainWindow::on_btnHome_pressed()
{
    logdebug << "on_btnHome_pressed";
}

void MainWindow::on_btnHome_released()
{
    logdebug << "on_btnHome_released";
}
