/**************************************************************************
**
** Copyright (C) 2023 Mike Feng
** Blog: https://www.cnblogs.com/crazyghostvon/
**
** This file mainwindow.h is part of the Swan project.
**
**************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QGraphicsPixmapItem>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>

#include "stable.h"
#include "flowcontroller.h"
#include "controls/swangraphicsview.h"
#include "ui_mainwindow.h"

using namespace Cg::Swan::Controller;
using namespace Cg::Swan::Controls;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void doAction(const ActionParam &);

protected:
    void closeEvent(QCloseEvent *e);
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent*);

private slots:
    void handleNewImage(const QImage &, ImageSource);
    void handleAdapterStatus(int);
    void handleActionFinished(double);
    void handleLivePosition(double);
    void on_btnCheckBarrier_pressed();
    void on_btnCheckBarrier_released();
    void on_btnStartStop_clicked();
    void on_btnPause_clicked();
    void on_btnHome_clicked();
    void on_radScanByRect_toggled(bool checked);
    void on_radScanByDeg_toggled(bool checked);
    void on_radScanByPixel_toggled(bool checked);
    void on_btnTurnLeft_pressed();
    void on_btnTurnLeft_released();
    void on_btnTurnRight_pressed();
    void on_btnTurnRight_released();

    void mouseMoved(QPoint point);
    void mousePressed(QPoint point);
    void mouseReleased(QPoint point);
    void mouseRightPressed(QPoint point);
    void mouseRightReleased(QPoint point);
    void entered();
    void leaved();

    void on_btnPickColor_clicked();
    void on_txtBasey1_valueChanged(int arg1);
    void on_txtBasey2_valueChanged(int arg1);
    void on_txtBasex_valueChanged(int arg1);
    void on_txtDegStart_valueChanged(int arg1);
    void on_txtDegEnd_valueChanged(int arg1);
    void on_txtDegStep_valueChanged(double arg1);
    void on_txtRectStep_valueChanged(int arg1);
    void on_txtPixStep_valueChanged(int arg1);
    void on_txtPixEnd_valueChanged(int arg1);
    void on_txtPixStart_valueChanged(int arg1);
    void on_txtExpo_valueChanged(int arg1);
    void on_btnSensorVideo_clicked();
    void on_btnSensorPreview_clicked();
    void on_chkMirrorH_clicked(bool checked);
    void on_chkMirrorV_clicked(bool checked);


private:
    void setShadowArea();
    void updateStatusLabel();
    void initUI();
    void setButtonColor(QColor);
    QPoint restoreCoor(const QPoint&);
    QPoint restoreCoor(const QPointF&);
    void enableButton(QList<QPushButton*> &);
    void enableButton(QPushButton*);

private:
    Ui::MainWindow *ui;
    SwanGraphicsView * _gvCamera;
    SwanGraphicsView * _gvSensor;
    QGraphicsScene *_sceneCamera;
    QGraphicsScene *_sceneSensor;
    FlowController * controller;
    QThread * tcontroller;
    QGraphicsPixmapItem *_imageItemCamera = nullptr;
    QGraphicsPixmapItem *_imageItemSensor = nullptr;
    QGraphicsLineItem  *_xlineCamera = nullptr;
    QGraphicsLineItem  *_ylineCamera = nullptr;
    QGraphicsLineItem  *_ybase1 = nullptr;
    QGraphicsLineItem  *_ybase2 = nullptr;
    QGraphicsLineItem  *_xbase = nullptr;
    QGraphicsLineItem  *_xlineSensor = nullptr;
    QGraphicsLineItem  *_ylineSensor = nullptr;
    QList<QGraphicsRectItem*> _rects;
    QGraphicsPathItem  *_shadowArea = nullptr; // provides the shadow effect
    QGraphicsTextItem * txtMsg = nullptr;
    bool isChangeRect = false;
    QPoint mPos;
    QColor baseColor;

    double scaleRateX = 1.0;
    double scaleRateY = 1.0;

    int start;
    int end;
    double step;
    SWanScanMode mode;
    QList<ScanRange>  scanRanges;

    bool isReset = true;
};

#endif // MAINWINDOW_H
