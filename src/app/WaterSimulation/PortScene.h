/** \class RigidBody
 * Defines rigid body to interacting with shallow water
 *
 * @author  Rahul Mukhi
 * @date 07/05/12
 *
 * Copyright (C) GIANTS Software GmbH, Confidential, All Rights Reserved.
 */

#include "ObjReader.h"
#include "glew/glew.h"
#include "glut/glut.h"
#include "base/2d/PNGUtil.h"
#include "base/math/MathUtil.h"

class PortScene 
{
	struct uniformGrid {
		int *groupId, *faceId;
		unsigned short idIndex;
	};

public:
	PortScene();
	~PortScene();

	void renderPort();
	float getGroundHeight(float x, float z);

private:
	void initialize();

	ObjReader m_portScene;
	GLuint *m_textureId;
	GLuint *m_vertexVBOIdPort, *m_indexVBOIdPort;

	static const int NUM_GRIDCELLS = 200;
	static const int UNIFORM_GRIDCELL = 20;
	Vector3 m_gridPoints[NUM_GRIDCELLS*NUM_GRIDCELLS];
	float m_cellEdge, m_uniformGridCellEdge, m_Xmax, m_Zmax;
	uniformGrid *m_uniformGrid;

	void createVBO();
	void calculateHeightForLowLevelGrid();
	void partitionTriangleInUniformGrid(float &GridLength);
	void calculateMaxGroundHeight(float GridLength);
};
