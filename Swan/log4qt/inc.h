#ifndef LOG4QT_INC_H
#define LOG4QT_INC_H

#include "log4qt/consoleappender.h"
#include "log4qt/fileappender.h"
#include "log4qt/rollingfileappender.h"
#include "log4qt/dailyrollingfileappender.h"
#include "log4qt/signalappender.h"
#include "log4qt/logger.h"
#include "log4qt/logmanager.h"
#include "log4qt/patternlayout.h"
#include "iostream"
#include <thread>
#include <QThread>

#define SP_LOG_FATAL_LEVEL 1
#define SP_LOG_ERROR_LEVEL 2
#define SP_LOG_WARN_LEVEL 3
#define SP_LOG_DEBUG_LEVEL 4
#define SP_LOG_INFO_LEVEL 5
#define SP_LOG_TRACE_LEVEL 6

extern int logLvl;

#ifdef _DISABLE_LOG
    #define INITIATE_LOG4QT(ClassName)
    #define SP_PRINT(s)
    #define SP_LOG_ERROR(s)
    #define SP_LOG_WARN(s)
    #define SP_LOG_INFO(s)
    #define SP_LOG_DEBUG(s)
    #define SP_LOG_FATAL(s)
    #define SP_LOG_TRACE(s)
#else
    #define INITIATE_LOG4QT(ClassName) LOG4QT_DECLARE_STATIC_LOGGER(LOG4QT_LOGGER, ClassName)
    #define SP_PRINT(s, lvl) if (logLvl >= lvl) std::cout << QTime::currentTime().toString("hh:mm:ss.zzz - [").toStdString() << QThread::currentThreadId() << "] " << QString(s).toStdString() << std::endl; else
    #ifdef _FILE_LOG
        #define SP_LOG_ERROR(s) LOG4QT_LOGGER()->error(QString(s))
        #define SP_LOG_WARN(s)  LOG4QT_LOGGER()->warn(QString(s))
        #define SP_LOG_INFO(s)  LOG4QT_LOGGER()->info(QString(s))
        #define SP_LOG_DEBUG(s) LOG4QT_LOGGER()->debug(QString(s))
        #define SP_LOG_FATAL(s) LOG4QT_LOGGER()->fatal(QString(s))
        #define SP_LOG_TRACE(s) LOG4QT_LOGGER()->trace(QString(s))
    #else
        #define SP_LOG_FATAL(s) SP_PRINT(QString("[FATAL] - ") + s, SP_LOG_FATAL_LEVEL)
        #define SP_LOG_ERROR(s) SP_PRINT(QString("[ERROR] - ") + s, SP_LOG_ERROR_LEVEL)
        #define SP_LOG_WARN(s)  SP_PRINT(QString("[WARNING] - ") +s, SP_LOG_WARN_LEVEL)
        #define SP_LOG_INFO(s)  SP_PRINT(QString("[INFO] - ") + s, SP_LOG_DEBUG_LEVEL)
        #define SP_LOG_DEBUG(s) SP_PRINT(QString("[DEBUG] - ") + s, SP_LOG_INFO_LEVEL)
        #define SP_LOG_TRACE(s) SP_PRINT(QString("[TRACE] - ") + s, SP_LOG_TRACE_LEVEL)
    #endif
#endif

#endif // LOG4QT_INC_H
