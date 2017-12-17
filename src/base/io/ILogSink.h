/** \class ILogSink
 * Log sink interface
 *
 * @author  Christian Ammann
 * @date  10/04/07
 *
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#pragma once

class ILogSink
{
public:
    virtual ~ILogSink() {};
    virtual void log(const char* pMessage) = 0;
};
