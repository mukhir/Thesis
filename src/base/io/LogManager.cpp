/*
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#include "PreCompiled.h"

#include "LogManager.h"

#include "base/util/DebugUtil.h"

/**
 * Get singleton instance of the LogManager
 *
 * @return returns singleton instance of the LogManager
 */
LogManager& LogManager::getInstance()
{
    static LogManager singletonLogManager;
    return singletonLogManager;
}

LogManager::LogManager()
{
    m_numLogSinks = 0;
    for (unsigned int i=0;i<MAX_LOG_SINKS;i++) {
        m_sinks[i] = NULL;
    }
}

void LogManager::addLogSink(ILogSink* pLogSink)
{
    m_sinks[m_numLogSinks] = pLogSink;
    m_numLogSinks++;
}

void LogManager::removeLogSink(ILogSink* pLogSink)
{
    for (unsigned int i=0;i<m_numLogSinks;i++) {
        if (m_sinks[i] == pLogSink) {
            for (unsigned int j=i;j<m_numLogSinks-1;j++) {
                m_sinks[j] = m_sinks[j+1];
            }
            m_numLogSinks--;
            break;
        }
    }
}

void LogManager::log(const char* pMessage)
{
    for (unsigned int i=0;i<m_numLogSinks;i++) {
        m_sinks[i]->log(pMessage);
    }
}

LogManager& LogManager::operator <<(const char* str)
{
    log(str);
    return *this;
}

LogManager& LogManager::operator <<(char c)
{
    char buf[2];
    buf[0] = c;
    buf[1] = 0;
    log(buf);
    return *this;
}

LogManager& LogManager::operator <<(float v)
{
    /*std::ostringstream os;
    os << v;
    log(os.str().c_str());*/
	char buffer[64]={0};
    sprintf(buffer, "%f", v);
    log(buffer);
    return *this;
}

LogManager& LogManager::operator <<(double v)
{
    /*std::ostringstream os;
    os << v;
    log(os.str().c_str());*/
	char buffer[128]={0};
    sprintf(buffer, "%f", v);
    log(buffer);
    return *this;
}

LogManager& LogManager::operator <<(int v)
{
    /*std::ostringstream os;
    os << v;
    log(os.str().c_str());*/
	char buffer[32]={0};
    sprintf(buffer, "%d", v);
    log(buffer);
    return *this;
}

LogManager& LogManager::operator <<(unsigned int v)
{
    /*std::ostringstream os;
    os << v;
    log(os.str().c_str());*/
    char buffer[32]={0};
    sprintf(buffer, "%u", v);
    log(buffer);
    return *this;
}

LogManager& LogManager::operator <<(bool v)
{
    if (v) {
        log("true");
    } else {
        log("false");
    }
    return *this;
}
