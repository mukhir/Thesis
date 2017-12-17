/** \class ImageDesc
 * ImageDesc
 *
 * @author  Thomas Brunner
 * @date  25/11/08
 *
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#pragma once

#include "base/Prerequisites.h"

class ImageDesc
{
public:

    enum TYPE {
        TYPE_UNKNOWN,
        TYPE_UNSIGNED_BYTE,
        TYPE_UNSIGNED_SHORT
    };

    enum FORMAT {
        FORMAT_UNKNOWN,
        FORMAT_LUMINANCE,
        FORMAT_RGB,
        FORMAT_BGR,
        FORMAT_RGBA,
        FORMAT_BGRA,
        FORMAT_DXT1,
        FORMAT_DXT3,
        FORMAT_DXT5
    };

    ImageDesc()
    {
       // m_pBuffer=NULL;
        m_width=0;
        m_height=0;
        m_channels=0;
        m_format = FORMAT_UNKNOWN;
        m_type = TYPE_UNKNOWN;
        m_numMipmaps = 0;
        m_bufferSize = 0;
        //m_bitDepth = 0;
    }

    unsigned int m_width;
    unsigned int m_height;
    //int m_bitDepth;
    unsigned int m_numMipmaps;
    unsigned int m_channels;
    TYPE m_type;
    unsigned int m_bufferSize;
    unsigned char* m_pBuffer;
    FORMAT m_format;

};
