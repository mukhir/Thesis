/** \class Camera
 * First person Camera and navigation control
 *
 * @author  Rahul Mukhi
 * @date 07/02/12
 *
 * Copyright (C) GIANTS Software GmbH, Confidential, All Rights Reserved.
 */

#pragma once

#include "base/math/Vector3.h"
#include <stdlib.h>
#include "glut/glut.h"

class Camera
{
public:

	Camera();
	~Camera();

	void look();
	void keyPressed(int key);
	void keyReleased(int key);

	void moveCameraWithBoat(Vector3& boatCentre);
	
	void rotateCamera();
	void moveCamera();
	void mouseMoved(int x, int y);
	inline Vector3 getCameraView()
	{
		return Vector3 (m_viewX, m_viewY, m_viewZ);
	}
	inline Vector3 getCameraPos()
	{
		return Vector3 (m_posX, m_posY, m_posZ);
	}
	inline float getCameraSpeed()
	{
		return m_speed;
	}


private:

	// camera speed
	float m_speed;

	// point where camera points to 
	float m_viewX, m_viewY, m_viewZ;

	// position of the camera
	float m_posX, m_posY, m_posZ;

	// for altitude
	float m_rotX;

	float m_theta, m_changeTheta;

	int m_oldX, m_oldY;
};