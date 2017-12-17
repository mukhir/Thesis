/** \class FileLogSink
 * File log sink
 *
 * @author  Christian Ammann
 * @date  10/04/07
 *
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#pragma once

#include <string>
#include <fstream>

#include "ILogSink.h"

class FileLogSink : public ILogSink
{
private:
    bool m_firstLogCall;
    bool m_overwrite;
    std::string m_filename;

public:
    FileLogSink(const char* pFilename, bool overwrite)
    {
        m_overwrite = overwrite;
        m_filename=pFilename;
    }
    
    void log(const char* pMessage)
    {
        std::ofstream file;
        unsigned int open_mode = std::ofstream::app;

        if (m_overwrite) {
            open_mode = std::ofstream::out;
            m_overwrite = false;
        }

        file.open(m_filename.c_str(), (std::ios_base::openmode) open_mode);
        file << pMessage;
        file.close();
    }
};

