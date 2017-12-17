/*
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 */

#include "SkyBox.h"

SkyBox::SkyBox()
{
}

SkyBox::~SkyBox()
{
}

void SkyBox::initSkyBox()
{
	long size;
	size_t m_result;

	FILE *m_pSkyBoxTop, *m_pSkyBoxSide;

	m_pSkyBoxTop = fopen("Data/day_sky_TOP.png", "rb");
	fseek(m_pSkyBoxTop, 0, SEEK_END);
	size = ftell(m_pSkyBoxTop);
	rewind(m_pSkyBoxTop);
	m_pBufferTop = (unsigned char*)malloc(sizeof(unsigned char)*size);
	m_result = fread(m_pBufferTop, 1, size, m_pSkyBoxTop);
	m_pngUtil.loadImage(m_pBufferTop, m_result, m_imageDescTop);
	fclose(m_pSkyBoxTop);
	free(m_pBufferTop);


	m_pSkyBoxSide = fopen("Data/day_sky_SIDE.png", "rb");
	fseek(m_pSkyBoxSide, 0, SEEK_END);
	size = ftell(m_pSkyBoxSide);
	rewind(m_pSkyBoxSide);
	m_pBufferSide = (unsigned char*)malloc(sizeof(unsigned char)*size);
	m_result = fread(m_pBufferSide, 1, size, m_pSkyBoxSide);
	m_pngUtil.loadImage(m_pBufferSide, m_result, m_imageDescSide);
	fclose(m_pSkyBoxSide);
	free(m_pBufferSide);


	glGenTextures(1, &m_texture[0]);
	glBindTexture(GL_TEXTURE_2D, m_texture[0]);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, m_imageDescTop.m_width, m_imageDescTop.m_height, GL_RGB, GL_UNSIGNED_BYTE, m_imageDescTop.m_pBuffer);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &m_texture[1]);
	glBindTexture(GL_TEXTURE_2D, m_texture[1]);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, m_imageDescSide.m_width, m_imageDescSide.m_height, GL_RGB, GL_UNSIGNED_BYTE, m_imageDescSide.m_pBuffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	
}

void SkyBox::renderSkyBox(float x, float y, float z)
{
	const float width = 1000.0f;
	const float height = 250.0f;
	const float length = 1000.0f;

	x = x - width  / 2.0f;
	//y = y - height / 2.0f;
	y = -100.0f;
	z = z - length / 2.0f;

	glEnable(GL_TEXTURE_2D);
	
	glBindTexture(GL_TEXTURE_2D, m_texture[0]);
	glBegin (GL_QUADS);
		glTexCoord2f (0.0f, 1.0f); glVertex3f (x,		y+height, z);
		glTexCoord2f (1.0f, 1.0f); glVertex3f (x+width, y+height, z);
		glTexCoord2f (1.0f, 0.0f); glVertex3f (x+width,	y+height, z+length);
		glTexCoord2f (0.0f, 0.0f); glVertex3f (x,		y+height, z+length);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, m_texture[1]);
	glBegin(GL_QUADS);	
		glTexCoord2f(.25f, 1.0f); glVertex3f(x,		  y,		z);
		glTexCoord2f(.5f, 1.0f); glVertex3f(x+width, y,		z);
		glTexCoord2f(0.5f, .0f); glVertex3f(x+width, y+height, z); 
		glTexCoord2f(0.25f, .0f); glVertex3f(x,       y+height,	z);
	
		glTexCoord2f(.0f, 1.0f); glVertex3f(x,		  y,		z+length);
		glTexCoord2f(.25f, 1.0f); glVertex3f(x, y,		z);
		glTexCoord2f(0.25f, .0f); glVertex3f(x, y+height, z); 
		glTexCoord2f(0.0f, .0f); glVertex3f(x,       y+height,	z+length);
	
		glTexCoord2f(.5f, 1.0f); glVertex3f(x+width,		  y,		z);
		glTexCoord2f(.75f, 1.0f); glVertex3f(x+width, y,		z+length);
		glTexCoord2f(0.75f, .0f); glVertex3f(x+width, y+height, z+length); 
		glTexCoord2f(0.5f, .0f); glVertex3f(x+width,       y+height,	z);
	
		glTexCoord2f(.75f, 1.0f); glVertex3f(x+width,		  y,		z+length);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y,		z+length);
		glTexCoord2f(1.0f, .0f); glVertex3f(x, y+height, z+length); 
		glTexCoord2f(0.75f, .0f); glVertex3f(x+width,       y+height,	z+length);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}