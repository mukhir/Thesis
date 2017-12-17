/*
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 */

#include "PreCompiled.h"
#include "WaterScene.h"

using namespace std;

WaterScene::WaterScene()
{
	m_frame = m_time = m_timebase = 0;
	
	m_windowWidth = 1360;
	m_windowHeight = 768;
	
	m_pPortScene = new PortScene();
	m_pWaterShape = new WaterShape(m_pPortScene);
	m_pBoat = new RigidBody(m_pWaterShape->m_waterSimulation, m_camera);

	m_renderPort = true;

	initWaterScene();
}

WaterScene::~WaterScene()
{
	delete m_pPortScene;
	delete m_pWaterShape;
	delete m_pBoat;
}

void WaterScene::initWaterScene()
{
	/*glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);*/

	initLight();
	m_skybox.initSkyBox();
}

void WaterScene::initLight()
{
	// Create light components
	float ambientLight0[] = { 1.0f, 0.6f, 0.0f, 1.0f };
	float diffuseLight0[] = { 1.0f, 0.6f, 0.0f, 1.0f };
	float specularLight0[] = { 1.0f, 0.6f, 0.0f, 1.0f };

	// Assign created components to GL_LIGHT0
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight0);

}

void WaterScene::update()
{
	m_camera.moveCamera();

	unsigned int time1 = glutGet(GLUT_ELAPSED_TIME);

	m_pWaterShape->update(m_camera.getCameraView());
	m_pBoat->rigidBodyInteraction();

	unsigned int time2 = glutGet(GLUT_ELAPSED_TIME);

	//std::cout<<time2 - time1<<std::endl;

}


void WaterScene::renderScene()
{
	unsigned int time1 = glutGet(GLUT_ELAPSED_TIME);

	// Preparing the buffer so it sends data to GPU while doing other operations
	m_pWaterShape->updateSWEGrid();

	// for water reflection
	createReflectionTexture();

	glViewport(0,0, 1360, 768);
	glLoadIdentity();
	m_camera.look();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	m_pBoat->renderObject();

	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);

	if(m_renderPort) {
		m_pPortScene->renderPort();
	}

	// For alpha blending
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWaterShape->renderWater(m_camera.getCameraPos());	

	glDisable(GL_BLEND);

	m_skybox.renderSkyBox(m_camera.getCameraPos().v[0], m_camera.getCameraPos().v[1], m_camera.getCameraPos().v[2]);

	displayFPS();

	glutSwapBuffers();

	unsigned int time2 = glutGet(GLUT_ELAPSED_TIME);

	//std::cout<<time2 - time1<<std::endl;
}

void WaterScene::createReflectionTexture()
{
	// Clear Color and Depth Buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	// Reset transformations
	glLoadIdentity();

	glBindFramebuffer(GL_FRAMEBUFFER, m_pWaterShape->m_frameBuffer);

	glViewport(0, 0, 512, 512);
	//glClearColor(1,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
	m_camera.look();

	glPushMatrix();
	glScalef(1.0f, -1.0f, 1.0f);
	glTranslatef(0.0f,-2.0f*m_pWaterShape->getTotalHeight(),0.0f);

	double plane[4] = {0.0f, 1.0f, 0.0, -m_pWaterShape->m_waterSimulation.TOTAL_HEIGHT};
	glEnable(GL_CLIP_PLANE0);
    glClipPlane(GL_CLIP_PLANE0, plane);

	// pass inverted camera modelview and projection matrices to shader to calculate reflection coordinates 
	m_pWaterShape->passModelViewProjectionToGLSL();
	
	if(m_renderPort) {
		m_pPortScene->renderPort();
	}

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	float position0[] = { 0.0f, -10.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, position0);

	m_pBoat->renderObject();
	
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);

    m_skybox.renderSkyBox(m_camera.getCameraPos().v[0], m_camera.getCameraPos().v[1], m_camera.getCameraPos().v[2]);

    glDisable(GL_CLIP_PLANE0);
    glPopMatrix();
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Save water reflection texture as png

	/*glBindTexture(GL_TEXTURE_2D, m_pWaterShape.m_reflectionTexture);
	GLint textureWidth, textureHeight;
	
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &textureWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &textureHeight);
	int size = textureWidth*textureHeight*3;

	m_pTexBuffer = (unsigned char*)malloc(sizeof(unsigned char)*size);
	glGetTexImage(GL_TEXTURE_2D,0, GL_RGB, GL_UNSIGNED_BYTE, m_pTexBuffer);
	glBindTexture(GL_TEXTURE_2D, 0);

	m_imageDescReflection.m_pBuffer = m_pTexBuffer;
	m_imageDescReflection.m_width = textureWidth;
	m_imageDescReflection.m_height = textureHeight;
	m_imageDescReflection.m_bufferSize = sizeof(unsigned char)*size;
	m_imageDescReflection.m_channels = 3;
	m_imageDescReflection.m_format = m_imageDescReflection.FORMAT_RGB;
	//m_imageDescReflection.m_numMipmaps;
	m_imageDescReflection.m_type = m_imageDescReflection.TYPE_UNSIGNED_BYTE;


	PNGUtil::saveImage("Reflection.png", m_imageDescReflection);

	free(m_pTexBuffer);*/

}

void WaterScene::pressKey(int key)
{
	m_camera.keyPressed(key);
}

void WaterScene::releaseKey(int key)
{
	m_camera.keyReleased(key);
}

void WaterScene::pressNormalKey(unsigned char key)
{
	m_pBoat->pressNormalKey(key);
	m_pWaterShape->pressNormalKey(key);

	if(key == 'r') {

		if(m_renderPort) {
			m_renderPort= false;
		} else {
			m_renderPort = true;
		}

	}

}

void WaterScene::releaseNormalKey(unsigned char key)
{
	m_pBoat->releaseNormalKey(key);
}

void WaterScene::mouseMoved(int x, int y)
{
	m_camera.mouseMoved(x,y);
}

void WaterScene::mouseButton(int button, int state, int x, int y)
{
	if(button == GLUT_LEFT_BUTTON) {
		if(state == GLUT_UP) {

			float z;
			GLdouble objPosX, objPosY, objPosZ;
			GLint viewport[4];                  // Where The Viewport Values Will Be Stored
			GLdouble modelview[16];                 // Where The 16 Doubles Of The Modelview Matrix Are To Be Stored
			GLdouble projection[16]; 

			glGetIntegerv(GL_VIEWPORT, viewport); 
			glGetDoublev(GL_MODELVIEW_MATRIX, modelview); 
			glGetDoublev(GL_PROJECTION_MATRIX, projection);

			y = viewport[3] - y;

			glReadPixels( x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z );
			gluUnProject( (float)x, (float)y, z, modelview, projection, viewport, &objPosX, &objPosY, &objPosZ);

			m_pWaterShape->addDrop(objPosX,objPosZ);
		}
	}
}


void WaterScene::displayFPS()
{
	m_frame++;
	m_time=glutGet(GLUT_ELAPSED_TIME);
	if (m_time - m_timebase > 1000) {
		sprintf(m_fps,"FPS:%4.2f",
			m_frame*1000.0/(m_time - m_timebase));

		m_timebase = m_time;
		m_frame = 0;
	}

	setOrthographicProjection();

	glColor3f(0.0f, 0.9f, 0.9f);
	 
	glPushMatrix();
	glLoadIdentity();
	renderBitmapString(GLUT_BITMAP_HELVETICA_18,m_fps);
	glPopMatrix();

	restorePerspectiveProjection();
}

void WaterScene::renderBitmapString( void *pFont, char *pString) {

  char *c;
  glRasterPos3f(0, 30,0);
  for (c=pString; *c != '\0'; c++) {
    glutBitmapCharacter(pFont, *c);
  }
}

void WaterScene::setOrthographicProjection() 
{

	// switch to projection mode
	glMatrixMode(GL_PROJECTION);

	// save previous matrix which contains the
	//settings for the perspective projection
	glPushMatrix();

	// reset matrix
	glLoadIdentity();

	// set a 2D orthographic projection
	gluOrtho2D(0, m_windowWidth, m_windowHeight, 0);

	// switch back to modelview mode
	glMatrixMode(GL_MODELVIEW);
}

void WaterScene::restorePerspectiveProjection() 
{

	glMatrixMode(GL_PROJECTION);
	// restore previous projection matrix
	glPopMatrix();

	// get back to modelview mode
	glMatrixMode(GL_MODELVIEW);
}