/**************************************************************************
**
** Copyright (C) 2017 Wuxi Speedpro Ltd.
** Contact: http://www.speedpro.cn/
**
** This file (signalappender.cpp) is part of the Speedpro Venus project.
**
**************************************************************************/
#include "signalappender.h"
#include "log4qt/layout.h"
#include <QtCore/QDebug>

namespace Log4Qt {

SignalAppender::SignalAppender(Layout *pLayout, QObject *pParent) :
    AppenderSkeleton(pParent),
    _maxLine(1000)
{
    setLayout(pLayout);
}

SignalAppender::~SignalAppender()
{}

void SignalAppender::append(const LoggingEvent & rEvent)
{
    QString message(layout()->format(rEvent));
    emit appendMessage(message);
    _currentLine++;

    if (_currentLine > _maxLine)
    {
        emit requireCleanup();
        _currentLine = 0;
    }
}

#ifndef QT_NO_DEBUG_STREAM
QDebug SignalAppender::debug(QDebug &rDebug) const
{
    rDebug.nospace() << "ListAppender("
        << "name:" << name() << " "
        << "filter:" << firstFilter() << " "
        << "isactive:" << isActive() << " "
        << "isclosed:" << isClosed() << " "
        << "referencecount:" << referenceCount() << " "
        << "threshold:" << threshold().toString()
        << ")";
    return rDebug.space();
}
#endif // QT_NO_DEBUG_STREAM

}
