/**************************************************************************
**
** Copyright (C) 2017 Wuxi Speedpro Ltd.
** Contact: http://www.speedpro.cn/
**
** This file (signalappender.h) is part of the Speedpro Venus project.
**
**************************************************************************/
#ifndef SIGNALAPPENDER_H
#define SIGNALAPPENDER_H

#include "log4qt/appenderskeleton.h"

#include <QtCore/QList>
#include <QtCore/QMutex>
#include "log4qt/loggingevent.h"

namespace Log4Qt
{

class SignalAppender: public AppenderSkeleton
{
    Q_OBJECT

    /*!
     * The property holds the maximum count used by the appender.
     *
     * The default maximum count is -1 for unlimited.
     *
     * \sa maxCount(), setMaxCount()
     */
    Q_PROPERTY(int maxLine READ maxLine WRITE setMaxLine)
public:
    SignalAppender(Layout *pLayout, QObject *pParent = 0);
    virtual ~SignalAppender();
private:
    SignalAppender(const SignalAppender &rOther); // Not implemented
    SignalAppender &operator=(const SignalAppender &rOther); // Not implemented

public:
    virtual bool requiresLayout() const;

    int maxLine() const;
    void setMaxLine(int n);

signals:
    void appendMessage(const QString & msg);
    void requireCleanup();

protected:
    virtual void append(const LoggingEvent &rEvent);

#ifndef QT_NO_DEBUG_STREAM
    /*!
     * Writes all object member variables to the given debug stream
     * \a rDebug and returns the stream.
     *
     * <tt>
     * %ListAppender(name:"LA" count:1 filter:0x41fa488 isactive:true
     *               isclosed:false maxcount:170 referencecount:1
     *               threshold:"TRACE_SET")
     * </tt>
     * \sa QDebug, operator<<(QDebug debug, const LogObject &rLogObject)
     */
    virtual QDebug debug(QDebug &rDebug) const;
#endif // QT_NO_DEBUG_STREAM

private:
    volatile int _maxLine;
    int _currentLine;
};

inline bool SignalAppender::requiresLayout() const { return true; }

inline int SignalAppender::maxLine() const { return _maxLine; }
inline void SignalAppender::setMaxLine(int v_) { _maxLine = v_; }

}

#endif // SIGNALAPPENDER_H
