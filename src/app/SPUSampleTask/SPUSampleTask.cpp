/*
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#include "SPUSampleTask.h"


SPUSampleTask CELL_ALIGN(128, gSPUSampleTask);

void cellMainSampleTask(CellArgument args) 
{
	gSPUSampleTask.main(args.m_pArg0, args.m_pArg1);
	cellExit();
}

void SPUSampleTask::main(void* pArg0, void* pArg1)
{

	spu_printf("Hello World from SPU!!!\n");

}
