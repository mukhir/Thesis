/** \class DebugUtil
 * DebugUtil, debug macros
 *
 * @author  Christian Ammann
 * @date  18/02/03
 *
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#pragma once

#include "Config.h"

#include <stdlib.h>

#define GS_ERROR_MSG(msg) DebugUtil::message("Error", msg, NULL, __FILE__,  __LINE__)


#if defined(NDEBUG)

#define GS_ASSERT(exp)                 /* do nothing */
#define GS_ASSERT_WITH_MSG(exp, desc)  /* do nothing */
#define GS_DEBUG_MSG(msg)              /* do nothing */

#else

#define GS_ASSERT(exp) GS_ASSERT_WITH_MSG(exp, "")
#define GS_ASSERT_WITH_MSG(exp, desc) if(!(exp)) { DebugUtil::message("Assertion failed", desc, #exp, __FILE__,  __LINE__); abort(); }
#define GS_DEBUG_MSG(msg) DebugUtil::message("Debug Message", msg, NULL, __FILE__,  __LINE__)

#endif

#if defined(GS_DO_VALIDITY_CHECKS)
#define GS_CHECK(exp, action) GS_CHECK_WITH_MSG(exp, action, "")
#define GS_CHECK_WITH_MSG(exp, action, desc) if (!(exp)) { DebugUtil::message(DebugUtil::ssprintf("Validity check failed in %s", __FUNCTION__), desc, #exp, __FILE__,  __LINE__); action; /*abort();*/ }
#define GS_CHECK_WITH_MSG_FUNC(exp, action, desc, func) if (!(exp)) { DebugUtil::message(DebugUtil::ssprintf("Validity check failed in %s", func), desc, #exp, __FILE__,  __LINE__); action; /*abort();*/ }
#else
#define GS_CHECK(exp, action)
#define GS_CHECK_WITH_MSG(exp, action, desc)
#define GS_CHECK_WITH_MSG_FUNC(exp, action, desc, func)
#endif


class DebugUtil{
public:
    static void message(const char* title, const char* desc, const char *body, const char *file, int line);
    static void init();
    static void finish();

    static const char* ssprintf(const char* msg, ...);
};

