/*
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#include "PreCompiled.h"

#include "PNGUtil.h"


// -- PNG Callback handler -- ------------------------
png_voidp PNGUtil::malloc_fn(png_structp png_ptr, png_size_t size)
{
    return new unsigned char[size];
}

void PNGUtil::free_fn(png_structp png_ptr, png_voidp ptr)
{
    delete[] ((unsigned char*)ptr);
}

void PNGUtil::png_error_func(png_structp png_ptr, png_const_charp message)
{
    longjmp(png_jmpbuf(png_ptr), 1);
}

void PNGUtil::user_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    PNGMemoryReader* reader = (PNGMemoryReader*)png_get_io_ptr(png_ptr);
    memcpy(data, reader->pInputBuffer+reader->currentBufferPosition, length);
    reader->currentBufferPosition+=(unsigned int)length;
}

void PNGUtil::user_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    FILE* pFile = static_cast<FILE*>(png_get_io_ptr(png_ptr));
    if (pFile != NULL) {
        fwrite(data, (size_t)length, 1, pFile);
    }
}

void PNGUtil::user_flush_data(png_structp png_ptr)
{
    FILE* pFile = static_cast<FILE*>(png_get_io_ptr(png_ptr));
    if (pFile != NULL) {
        fflush(pFile);
    }
}

// -- ------------------------------------------------

bool PNGUtil::checkHeader(unsigned char* pBuffer, unsigned int bufferSize)
{
    const unsigned int HEADER_PREFIX_SIZE = 8;
    if (bufferSize < HEADER_PREFIX_SIZE) {
        return false;
    }

    return (png_check_sig(pBuffer, HEADER_PREFIX_SIZE) != 0);
}

bool PNGUtil::loadImage(unsigned char* pBuffer, unsigned int bufferSize, ImageDesc& desc)
{
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;

    // First check the eight byte PNG signature
    if (!checkHeader(pBuffer, 8)) {
        return false;
    }
    
    // Create the two png(-info) structures
    if ((png_ptr = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL, NULL, malloc_fn, free_fn)) == NULL) {
        return false;
    }
 
    if ((info_ptr = png_create_info_struct(png_ptr)) == NULL) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return false;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return false;
    }

    // Initialize the png structure
    PNGMemoryReader memoryReader;
    memoryReader.pInputBuffer = pBuffer;
    memoryReader.inputBufferSize = bufferSize;
    memoryReader.currentBufferPosition = 8;
    png_set_read_fn(png_ptr, &memoryReader, &PNGUtil::user_read_data);

    png_set_sig_bytes(png_ptr, 8);

    // Read all PNG info up to image data
    png_read_info(png_ptr, info_ptr);

    // Get width, height, bit-depth and color-type
    int colorType;
    int bitDepth;

    png_uint_32 width, height;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bitDepth, &colorType, NULL, NULL, NULL);


    // Expand low-bit-depth grayscale images to 8 bits
    if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }

    // Expand paletted colors into true RGB triplets
    if (colorType == PNG_COLOR_TYPE_PALETTE) {
        png_set_expand(png_ptr);
    }

    // Expand RGB images with transparency to full alpha channels so the data will be available as RGBA quartets
    // But don't expand paletted images, since we want alpha palettes!
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) && !(png_get_valid(png_ptr, info_ptr, PNG_INFO_PLTE))){
        png_set_tRNS_to_alpha(png_ptr);
    }

    // Refresh information
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bitDepth, &colorType, NULL, NULL, NULL);

    desc.m_width = width;
    desc.m_height = height;

    // Expanded earlier for grayscale, now take care of palette and rgb
    if (bitDepth < 8) {    
        bitDepth = 8;
        png_set_packing(png_ptr);
    }

    // Fix endianess
#if defined(GS_LITTLE_ENDIAN)
    if (bitDepth == 16){
        png_set_swap(png_ptr);
    }
#endif

    /*// Tell libpng to strip 16 bit/color files down to 8 bits/color 
    if (desc.m_bitDepth == 16) { 
        png_set_strip_16(png_ptr);
    }*/

    switch (bitDepth) {
    case 8:
        desc.m_type = ImageDesc::TYPE_UNSIGNED_BYTE;
        break;
    case 16:
        desc.m_type = ImageDesc::TYPE_UNSIGNED_SHORT;
        break;
    default:
        return false;
    }

    png_read_update_info(png_ptr,info_ptr);

    desc.m_channels = png_get_channels(png_ptr, info_ptr);

    colorType = png_get_color_type(png_ptr, info_ptr);

    //png_get_IHDR(png_ptr, info_ptr, &width , &height, &bitDepth, &colorType, NULL, NULL, NULL);

    desc.m_channels =  png_get_channels(png_ptr, info_ptr);

    switch (desc.m_channels) {
    case 1:
        desc.m_format = ImageDesc::FORMAT_LUMINANCE;
        break;
    case 2:
    case 3:
        desc.m_format = ImageDesc::FORMAT_RGB;
        break;
    case 4:
        desc.m_format =  ImageDesc::FORMAT_RGBA;
        break;
    default:
        return false;
    }

    int rowSize = desc.m_width * desc.m_channels * bitDepth / 8;

    // Allocate memory to store the image
 
    desc.m_pBuffer = new unsigned char[rowSize * desc.m_height ];
    desc.m_bufferSize = rowSize * desc.m_height;
    
    // Set the individual row-pointers to point at the correct offsets
    png_byte** ppbRowPointers = new png_bytep[desc.m_height];
 
    for (unsigned int i=0; i<desc.m_height; i++) {
        ppbRowPointers[i] = desc.m_pBuffer + i * rowSize ;
    }
    
    // Read the whole image
    png_read_image(png_ptr, ppbRowPointers);

    // Clean up
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    delete [] ppbRowPointers;
     
    return true;
}

bool PNGUtil::saveImage(const char* pFileName, const ImageDesc& desc)
{
    int type;

    switch (desc.m_format) {
    case ImageDesc::FORMAT_LUMINANCE:
        type = PNG_COLOR_TYPE_GRAY;
        break;
    case ImageDesc::FORMAT_RGB:
        type = PNG_COLOR_TYPE_RGB;
        break;
    case ImageDesc::FORMAT_RGBA:
        type = PNG_COLOR_TYPE_RGBA;
        break;
    default:
        return false;
    }

    int bitDepth;
    switch (desc.m_type) {
    case ImageDesc::TYPE_UNSIGNED_BYTE:
        bitDepth = 8;
        break;
    case ImageDesc::TYPE_UNSIGNED_SHORT:
        bitDepth = 16;
        break;
    default:
        return false;
    };

    png_structp png_ptr;
    png_infop info_ptr;

    FILE* pFile = fopen(pFileName, "wb");

    if (pFile == NULL) {
        return false;
    }

    if ((png_ptr = png_create_write_struct_2(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL, NULL, malloc_fn, free_fn)) == NULL) {
        fclose(pFile);
        return false;
    }

    if ((info_ptr = png_create_info_struct(png_ptr)) == NULL) {
        png_destroy_write_struct(&png_ptr, NULL);
        fclose(pFile);
        return false;
    }

    png_set_write_fn(png_ptr, pFile, user_write_data, user_flush_data);
    png_set_IHDR(png_ptr, info_ptr, desc.m_width, desc.m_height, bitDepth, type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_text text[3];
    memset(text, 0, sizeof(png_text) * 3);
    
    // (char*)((const char*) used to prevent the warning: deprecated conversion from string constant to ‘char*’ under gcc
    text[0].key = (char*)((const char*)"Generated by");
    text[0].text = (char*)((const char*)"");
    text[0].compression = PNG_TEXT_COMPRESSION_NONE;
    text[1].key = (char*)((const char*)"Author's name");
    text[1].text = (char*)((const char*)"");
    text[1].compression = PNG_TEXT_COMPRESSION_NONE;
    text[2].key = (char*)((const char*)"Author's comments");
    text[2].text = (char*)((const char*)"");
    text[2].compression = PNG_TEXT_COMPRESSION_NONE;
    png_set_text(png_ptr, info_ptr, text, 3);

    // Write the file header information.
    png_write_info(png_ptr, info_ptr);
    //int nElements = desc.m_width * desc.m_channels;

    // swap bytes of 16-bit files to most significant byte first
#if defined(GS_LITTLE_ENDIAN)
    png_set_swap(png_ptr);
#endif

    int rowSize = desc.m_width * desc.m_channels * bitDepth / 8;

    for (unsigned int y = 0; y < desc.m_height; y++) {
        png_write_row(png_ptr, desc.m_pBuffer + y * rowSize);
    }

    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    
    fclose(pFile);

    return true;
}
