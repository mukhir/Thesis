/**
 * Prerequisites
 *
 * @author  Christian Ammann
 * @date  30/05/05
 *
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#pragma once

#define GS_BIT(b) (1LL<<(b))
#define GS_BIT_ADD(a, b) ((a)<<(b))

#define GS_MAX(a, b) ((a >= b)?a:b)
#define GS_MIN(a, b) ((a <= b)?a:b)
#define GS_BOUND(x, min_value, max_value) ((x) < (min_value) ? (min_value) : ((x) > (max_value) ? (max_value) : (x)))

#define GS_PI 3.14159265358979323846

#define GS_MAKEFOURCC(ch0, ch1, ch2, ch3) \
    ((unsigned int)(unsigned char)(ch0) |\
    ((unsigned int)(unsigned char)(ch1) << 8 ) |\
    ((unsigned int)(unsigned char)(ch2) << 16) |\
    ((unsigned int)(unsigned char)(ch3) << 24))
