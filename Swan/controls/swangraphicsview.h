/**************************************************************************
**
** Copyright (C) 2023 Mike Feng
** Blog: https://www.cnblogs.com/crazyghostvon/
**
** This file (swangraphicsview.h) is part of the Swan project.
**
**************************************************************************/
#ifndef SWANGRAPHICSVIEW_H
#define SWANGRAPHICSVIEW_H

#include "stable.h"
#include <QObject>
#include <QGraphicsView>

namespace Cg { namespace Swan { namespace Controls {


class SwanGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    SwanGraphicsView(QWidget *parent = nullptr);


signals:
    void mouseMovePoint(QPoint point);
    void mousePress(QPoint point);
    void mouseRelease(QPoint point);
    void mouseRightPress(QPoint point);
    void mouseRightRelease(QPoint point);
    void enter();
    void leave();

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

};

} } }

#endif // SWANGRAPHICSVIEW_H
