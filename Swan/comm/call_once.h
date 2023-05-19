/**************************************************************************
**
** Copyright (C) 2023 Mike Feng
** Blog: https://www.cnblogs.com/crazyghostvon/
**
** This file (call_once.h) is part of the Swan project.
**
**************************************************************************/
#ifndef CALL_ONCE_H
#define CALL_ONCE_H

#include <QThread>
#include <QThreadStorage>

namespace CallOnce {
enum ECallOnce {
    CO_Request,
    CO_InProgress,
    CO_Finished
};

Q_GLOBAL_STATIC(QThreadStorage<QAtomicInt*>, onceFlag)
}

template <class Function>
inline static void qCallOnce(Function func, QBasicAtomicInt& flag)
{
    using namespace CallOnce;

#if QT_VERSION < 0x050000
    int protectFlag = flag.fetchAndStoreAcquire(flag);
#elif QT_VERSION >= 0x050000
    int protectFlag = flag.fetchAndStoreAcquire(flag.load());
#endif

    if (protectFlag == CO_Finished)
        return;
    if (protectFlag == CO_Request && flag.testAndSetRelaxed(protectFlag, CO_InProgress)) {
        func();
        flag.fetchAndStoreRelease(CO_Finished);
    } else {
        do {
            QThread::yieldCurrentThread();
        } while (!flag.testAndSetAcquire(CO_Finished, CO_Finished));
    }
}

template <class Function>
inline static void qCallOncePerThread(Function func)
{
    using namespace CallOnce;
    if (!onceFlag()->hasLocalData()) {
        onceFlag()->setLocalData(new QAtomicInt(CO_Request));
        qCallOnce(func, *onceFlag()->localData());
    }
}

#endif // CALL_ONCE_H
