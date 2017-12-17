/** \class WaterScene
 * Manages the whole water scene  (PortScene, RigidBody, Camer and Watershape)
 *
 * @author  Rahul Mukhi
 * @date 07/02/12
 *
 * Copyright (C) GIANTS Software GmbH, Confidential, All Rights Reserved.
 */

#pragma once
#define GL_GLEXT_PROTOTYPES


#include "WaterShape.h"
#include "Camera.h"
#include "SkyBox.h"
#include "RigidBody.h"

#include "base/math/Plane.h"
#include "base/math/Vector3.h"
#include <vector>
#include <iostream>
#include <fstream>


class WaterScene
{

public:
	WaterScene();
	~WaterScene();

	int m_windowWidth, m_windowHeight;

	void initWaterScene();
	void update();
	void renderScene();
	void pressKey(int key);
	void releaseKey(int key);
	void mouseButton(int button, int state, int x, int y);
	void mouseMoved(int x, int y);
	

	void pressNormalKey(unsigned char key);
	void releaseNormalKey(unsigned char key);

private:

	int m_frame, m_time, m_timebase;
	char m_fps[50];
	
	SkyBox m_skybox;
	PortScene* m_pPortScene;
	WaterShape* m_pWaterShape;
	RigidBody* m_pBoat;
	Camera m_camera;
	
	bool m_renderPort;

	PNGUtil m_pngUtil;
	ImageDesc m_imageDescReflection;
	unsigned char* m_pTexBuffer;

	void initLight();
	void createReflectionTexture();

	void renderBitmapString(void *pFont, char *pString);

	void displayFPS();
	void setOrthographicProjection();
	void restorePerspectiveProjection();

};