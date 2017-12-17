 /**
 * Pre compiled header file
 *
 * @author  Christian Ammann
 * @date  04/05/05
 *
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#include "base/Platform.h"

#define GS_USE_PRECOMPILED_HEADERS
#if defined(GS_USE_PRECOMPILED_HEADERS)

#if defined(_WIN32)
    // Modify the following defines if you have to target a platform prior to the ones specified below.
    // Refer to MSDN for the latest info on corresponding values for different platforms.
    #ifndef WINVER                // Allow use of features specific to Windows XP or later.
    #define WINVER 0x0501        // Change this to the appropriate value to target other versions of Windows.
    #endif

    #ifndef _WIN32_WINNT        // Allow use of features specific to Windows XP or later.                   
    #define _WIN32_WINNT 0x0501    // Change this to the appropriate value to target other versions of Windows.
    #endif    

    #ifndef _WIN32_WINDOWS        // Allow use of features specific to Windows 98 or later.
    #define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
    #endif

    #ifndef _WIN32_IE            // Allow use of features specific to IE 6.0 or later.
    #define _WIN32_IE 0x0600    // Change this to the appropriate value to target other versions of IE.
    #endif

    #define NOMINMAX

    #include <winsock2.h>
    #include <windows.h>
#endif

#if defined(__APPLE__)
    #define NX32 1
//    #define NX_DISABLE_FLUIDS 1
//    #define NX_DISABLE_SOFTBODY 1
    #define NX_PHYSICS_STATICLIB 1
    #define NX_FOUNDATION_STATICLIB 1
    #define NXCHARACTER_STATIC 1
    #define CORELIB 1
    #define NX_USE_SDK_STATICLIBS 1
    #define NX_DISABLE_HARDWARE 1
    #define NX_COOKING_STATICLIB 1
#endif

#include <math.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#include <vector>
#include <functional>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <map>
#include <list>
#include <set>

#if defined(WIN32)

#elif defined(__APPLE__) || defined(__linux__)
    #include <unistd.h> 
    #include <string.h>
    #include <stdarg.h>
#endif

#endif //USE_PRECOMPILED_HEADERS

