/** \class SPUSampleTask
 * SPU sample task
 *
 * @author  Christian Ammann
 * @date  24/01/12
 *
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#pragma once

#include "base/util/SPUDMA.h"

void cellMainSampleTask(CellArgument args);

class SPUSampleTask {
public:
	void main(void* pArg0, void* pArg1);
};


