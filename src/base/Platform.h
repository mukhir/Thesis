/**
 * Platform defines (Fixes platform incompatibilities)
 *
 * @author  Christian Ammann
 * @date  30/05/05
 *
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#pragma once

// Fix Microsoft Visual C++ issues
#if defined(_MSC_VER)
    #define strcasecmp _stricmp
    #define strncasecmp _strnicmp
    #define isnan _isnan
    #define finite _finite
    
    #define usleep(microsecs) Sleep((microsecs)/1000)
    
    #define strtoll _strtoi64

    #if (_MSC_VER >= 1400) // VC8+    
        #ifndef _CRT_SECURE_NO_WARNINGS
            #define _CRT_SECURE_NO_WARNINGS
        #endif
        #ifndef _CRT_SECURE_NO_DEPRECATE
            #define _CRT_SECURE_NO_DEPRECATE
        #endif
        #ifndef _CRT_NONSTDC_NO_DEPRECATE
            #define _CRT_NONSTDC_NO_DEPRECATE
        #endif
    #endif
#endif

//#define GS_SHOW_MEMORY_LEAKS

#if defined(_WIN32) && defined(_MSC_VER) && defined(GS_SHOW_MEMORY_LEAKS)
    #include <crtdbg.h>
    void * operator new(size_t nSize, const char * lpszFileName, int nLine);
    //{ return ::operator new(nSize, 1, lpszFileName, nLine); }
    #define new new(__FILE__,__LINE__)
#endif

#if defined(_MSC_VER)
    #define GS_FORCEINLINE __forceinline
    #define GS_RESTRICT __restrict
    #define GS_NO_DEFAULT default: __assume(0); break;
#elif defined(__GNUC__)
    #define GS_FORCEINLINE inline
    #define GS_RESTRICT __restrict
    #define GS_NO_DEFAULT default: break;
#elif defined(NN_PLATFORM_CTR)
    #define GS_FORCEINLINE __forceinline
    #define GS_RESTRICT __restrict
    #define GS_NO_DEFAULT default: break;
#else
    #define GS_FORCEINLINE inline
    #define GS_RESTRICT
    #define GS_NO_DEFAULT default: break;
#endif

#if defined(_M_IX86) || defined(_M_X64) || defined(i386) || defined(__AMD64__) || defined(NN_PLATFORM_CTR)
    #define GS_LITTLE_ENDIAN
#else
    #define GS_BIG_ENDIAN
#endif

#if defined(_MSC_VER)
    #define GS_OFFSETOF(type, member) offsetof(type, member)
#elif defined(__GNUC__)
    #define GS_OFFSETOF(type, member)   __builtin_offsetof(type, member)
#else
    #define GS_OFFSETOF(type, member)   ((size_t)&(((type*)0)->member))
#endif

#if defined(_MSC_VER)
    #include <hash_map>
    #define StdHashMap stdext::hash_map
#elif defined(__APPLE__) && !defined(IPHONE)
//    #define isnan std::isnan

    // GCC tr1 sucks on OSX
    #include <ext/hash_map>
    #define StdHashMap __gnu_cxx::hash_map
#ifndef GCC_HASHMAP_WORKAROUND
#define GCC_HASHMAP_WORKAROUND
namespace __gnu_cxx
{
    template<> struct hash< std::string >
    {
        size_t operator()( const std::string& x ) const
        {
            return hash< const char* >()( x.c_str() );
        }
    };
}
#endif // GCC_HASHMAP_WORKAROUND

#else
    #include <tr1/unordered_map>
    #define StdHashMap std::tr1::unordered_map
#endif // _MSC_VER

