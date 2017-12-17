/*
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#include "PreCompiled.h"

#include <stdarg.h>

#include "DebugUtil.h"

#include "base/io/LogManager.h"

//#include "base/app/ProfilerManager.h"



/**
 * Message
 * 
 * @param  title  message title.
 * @param  desc  description.
 * @param  body  message body.
 * @param  file  source code filename.
 * @param  line  line number.
 */
void DebugUtil::message(const char* title, const char* desc, const char* body, const char* file, int line)
{
    if (title != NULL) {
        LogManager::getInstance() << title << ": ";
    }

    LogManager::getInstance() << desc << "\n";

    if (file != NULL || body != NULL) {
        LogManager::getInstance() << "  ";
        if (file != NULL) {
            LogManager::getInstance() << file << "(" << line << ")";
        }
        if (body != NULL) {
            if (file != NULL) {
                LogManager::getInstance() << ": ";
            }
            LogManager::getInstance() << body << "\n";
        }
    }


}

/**
 * Initialize debug stuff
 */
void DebugUtil::init()
{
#if defined(_WIN32) && defined(_MSC_VER) && defined(SHOW_MEMORY_LEAKS)
    _CrtSetDbgFlag( 
    _CRTDBG_ALLOC_MEM_DF | // Turns on debug allocation
    _CRTDBG_LEAK_CHECK_DF
    );
#endif
}

/**
 * Finish debug stuff (eg. show memory leaks)
 */
void DebugUtil::finish()
{
#if defined(_WIN32) && defined(_MSC_VER) && defined(SHOW_MEMORY_LEAKS)
    /*ProfilerManager::getInstance().debugDelete();
    if (_CrtDumpMemoryLeaks()) {
        LogManager::getInstance() << "Mempry leaks found" << "\n";
    }*/
#endif
}

/**
 * Sprintf style string formating into a static string buffer. You may use this for
 * formating any string you may treat as volatile. The returned string is valid
 * for the next 4 calls to ssprintf.
 *
 * Caution: This method is not thread safe.
 */
const char* DebugUtil::ssprintf(const char* msg, ...)
{
    static int bufferI = 0;
    static char buffers[4][8192];

    char* buffer = buffers[bufferI];
    va_list args;
    va_start(args, msg);
    vsprintf(buffer, msg, args);
    bufferI++;
    if (bufferI >= 4) {
        bufferI = 0;
    }
    return buffer;
}
