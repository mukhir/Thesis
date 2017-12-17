/** \class PNGUtil
 * PNG util, saves and loads png images
 *
 * @author  Thomas Brunner
 * @date  25/11/08
 *
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */
#pragma once

#include "ImageDesc.h"

#include "libpng/png.h"

class PNGUtil
{

    struct PNGMemoryReader {
        unsigned char* pInputBuffer;
        unsigned int inputBufferSize;
        unsigned int currentBufferPosition;
    };

    // Callbacks
    static png_voidp malloc_fn(png_structp png_ptr, png_size_t size);
    static void free_fn(png_structp png_ptr, png_voidp ptr);
    static void png_error_func(png_structp png_ptr, png_const_charp message);

    static void user_read_data(png_structp png_ptr, png_bytep data, png_size_t length);
    static void user_write_data(png_structp png_ptr, png_bytep data, png_size_t length);
    static void user_flush_data(png_structp png_ptr);

public:

    static bool checkHeader(unsigned char* pBuffer, unsigned int bufferSize);
    static bool loadImage(unsigned char* pBuffer, unsigned int bufferSize, ImageDesc& desc);
    static bool saveImage(const char* pFileName, const ImageDesc& desc);

};
