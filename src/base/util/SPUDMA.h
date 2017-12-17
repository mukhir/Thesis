/**
 * SPU DMA
 *
 * @author  Christian Ammann
 * @date  24/01/12
 *
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#pragma once

#ifdef __SPU__
#include <spu_printf.h>
#else
#include <stdio.h>
#define spu_printf printf
#endif

#ifdef _MSC_VER
	#define GS_ALIGN(alignment, decl) __declspec(align(alignment)) decl
#elif __GNUC__
	#define GS_ALIGN(alignment, decl) decl __attribute__ ((aligned(alignment)))
#endif

#define GS_ALIGN_SIZE_16(size) (((unsigned)(size)+15)&((unsigned)~15))

struct CellArgument {
	void* m_pArg0;
	void* m_pArg1;
};

void cellDmaGet(void* pLocal, const void* pMainMem, unsigned int size, unsigned int tag);
void cellDmaLargeGet(void* pLocal, const void* pMainMem, unsigned int size, unsigned int tag);
void cellDmaPut(const void* pLocal, void* pMainMem, unsigned int size, unsigned int tag);
void cellDmaLargePut(const void* pLocal, void* pMainMem, unsigned int size, unsigned int tag);
void cellDmaWaitTagStatusAll(unsigned int tagMask);