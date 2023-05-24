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
    this->setCursor(QCursor(QPixmap(":/c/dot.png")));
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
    QPoint point=event->pos();
    if (event->button()==Qt::LeftButton)
    {
        emit mousePress(point);
    }
    else
    {
        emit mouseRightPress(point);
    }
}

void SwanGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    QPoint point=event->pos();
    if (event->button()==Qt::LeftButton)
    {
        emit mouseRelease(point);
    }
    else
    {
        emit mouseRightRelease(point);
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
