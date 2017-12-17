/** \class LogManager
 * Logging manager
 * Singleton design pattern.
 * Example:
 *   LogManager::getInstance() << "Log this!";
 *
 * @author  Christian Ammann
 * @date  26/03/04
 *
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#pragma once

#include "ILogSink.h"

class LogManager
{
protected:
    static const unsigned int MAX_LOG_SINKS = 6;
    unsigned int m_numLogSinks;
    ILogSink* m_sinks[MAX_LOG_SINKS];

    void log(const char* pMessage);

public:
    static LogManager& getInstance();

    LogManager();

    void addLogSink(ILogSink* pLogSink);
    void removeLogSink(ILogSink* pLogSink);

    LogManager& operator <<(const char* str);
    LogManager& operator <<(char c);
    LogManager& operator <<(float v);
    LogManager& operator <<(double v);
    LogManager& operator <<(int v);
    LogManager& operator <<(unsigned int v);
    LogManager& operator <<(bool v);

};
