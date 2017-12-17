/** \class SkyBox
 * Skybox for waterscene
 *
 * @author  Rahul Mukhi
 * @date 07/02/12
 *
 * Copyright (C) GIANTS Software GmbH, Confidential, All Rights Reserved.
 */

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "glut/glut.h"
#include "glext/glext.h"
#include "base/2d/ImageDesc.h"
#include "base/2d/PNGUtil.h"

using namespace std;

class SkyBox
{
public:
	SkyBox();
	~SkyBox();

	void initSkyBox();
	void renderSkyBox(float x, float y, float z);

private:

	unsigned char *m_pBufferTop, *m_pBufferSide;
	GLuint m_texture[2];

	ImageDesc m_imageDescTop, m_imageDescSide;
	PNGUtil m_pngUtil;

	
};