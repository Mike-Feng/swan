/**************************************************************************
**
** Copyright (C) 2023 Mike Feng
** Blog: https://www.cnblogs.com/crazyghostvon/
**
** This file (swangraphicsview.cpp) is part of the Swan project.
**
**************************************************************************/
#include "swangraphicsview.h"
#include    <QMouseEvent>
#include    <QPoint>

using namespace Cg::Swan::Controls;

SwanGraphicsView::SwanGraphicsView(QWidget *parent):QGraphicsView(parent)
{
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void SwanGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    QPoint point=event->pos();
    emit mouseMovePoint(point);
}

void SwanGraphicsView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    if (event->button()==Qt::LeftButton)
    {
        QPoint point=event->pos();
        emit mousePress(point);
    }
}

void SwanGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    if (event->button()==Qt::LeftButton)
    {
        QPoint point=event->pos();
        emit mouseRelease(point);
    }

}

void SwanGraphicsView::enterEvent(QEvent *event)
{
    QGraphicsView::enterEvent(event);
    emit enter();
}

void SwanGraphicsView::leaveEvent(QEvent *event)
{
    QGraphicsView::leaveEvent(event);
    emit leave();
}
