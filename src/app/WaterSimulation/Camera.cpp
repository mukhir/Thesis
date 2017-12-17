/*
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 */

#include "PreCompiled.h"
#include "Camera.h"


Camera::Camera()
{
	m_viewX = 0.0f;
	m_viewY = 0.0f;
	m_viewZ = 0.0f;

	m_posX = 30.0f;
	m_posY = 35.0f;
	m_posZ = 0.0f;

	m_rotX = 0.0f;

	m_speed = 0.0f;
	m_theta = -90.0f*3.14f/180.0f;
	m_changeTheta = 0.0f;
}

Camera::~Camera()
{
}

void Camera::look()
{
	gluLookAt(	m_posX, m_posY, m_posZ,
			m_viewX , m_viewY ,  m_viewZ,
			0.0f, 1.0f,  0.0f);
}

void Camera::keyPressed(int key)
{
	switch(key) 
	{
		case GLUT_KEY_LEFT :  m_changeTheta = -.02f; break; //rotate camera
		case GLUT_KEY_RIGHT :  m_changeTheta = .02f; break;
		case GLUT_KEY_UP : m_speed = 0.005f; break;   // moving forward
		case GLUT_KEY_DOWN : m_speed = -0.005f; break;
	}

}

void Camera::keyReleased(int key)
{
	switch(key)
	{
		case GLUT_KEY_LEFT : 
		case GLUT_KEY_RIGHT : m_changeTheta = 0.0f; break;
		case GLUT_KEY_UP : 
		case GLUT_KEY_DOWN : m_speed = .0f; break;  // speed = 0 when key released
	}
}

void Camera::mouseMoved(int x, int y)
{
	static bool start = true;
	if(start)
	{
		m_oldX = x;
		m_oldY = y;
		start = false;
	}

	m_rotX = float(m_oldY - y);

	m_viewY += m_rotX/4.0f;
	m_rotX = 0.0f;

	m_oldX = x;
	m_oldY = y;

}


void Camera::moveCamera()
{
	if(m_speed)
	{
		Vector3 dir(m_viewX-m_posX, m_viewY-m_posY, m_viewZ-m_posZ);

		m_posX = m_posX + dir[0]*m_speed;
		m_posY = m_posY + dir[1]*m_speed;
		m_posZ = m_posZ + dir[2]*m_speed;

		m_viewX = m_viewX + dir[0]*m_speed;
		m_viewY = m_viewY + dir[1]*m_speed;
		m_viewZ = m_viewZ + dir[2]*m_speed;
	}

}

void Camera::moveCameraWithBoat(Vector3& boatCentre)
{
	m_viewX = boatCentre[0];
	m_viewZ = boatCentre[2];

	m_theta += m_changeTheta;

	m_posX = boatCentre[0] + sin(m_theta)*60.0f;;
	m_posZ = boatCentre[2] + cos(m_theta)*60.0f;

}

