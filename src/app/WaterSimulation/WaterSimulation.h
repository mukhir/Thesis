/** \class WaterSimulation
 * Does real time water simulation for water surface
 *
 * @author  Rahul Mukhi
 * @date 27/03/12
 *
 * Copyright (C) GIANTS Software GmbH, Confidential, All Rights Reserved.
 */


#include <stdlib.h>
#include "glew/glew.h"
#include "glut/glut.h"

#include "PortScene.h"

#include "base/math/Vector3.h"
#include <fstream>

#pragma once

class WaterSimulation
{

public:
	WaterSimulation(PortScene* portScene);
	~WaterSimulation();

	static const int NUM_CELLS = 120;
	static const int NUM_GRIDS = NUM_CELLS*NUM_CELLS;
	static const int NUM_BORDER_DAMPING_CELLS = NUM_CELLS/20;
	static const float TOTAL_HEIGHT;
	static const float FLAT, CELL_EDGE, INV_DIST, TIME_STEP, GRAVITY, DISPLACED_HEIGHT, UNDER_WATER, BOUNDARY_THRESHOLD, GRIDSTART_X, GRIDSTART_Z;

	void initializeGrid();
	void update(const Vector3& cameraView);
	void addDrop(float objPosX, float objPosZ);
	void fillIndicesSWE(std::vector<GLuint>& indexVect);
	void fillIndicesFFT(std::vector<GLuint>& indexVect);
	void fillVertexBufferandUpdateNormals(float* pVertices);
	void fillFFTVertexBuffer(float* pVertices);
	float getWaterHeight(float x, float z);

	Vector3 m_convexHull[25];
	short m_convexHullSize;
	float m_xVelocity, m_zVelocity;
	float m_boatSpeed, m_rotation;
	
	inline float getgridStartX()
	{
		return GRIDSTART_X;
	}

	inline float getgridStartZ()
	{
		return GRIDSTART_Z;
	}

	inline float getTranslationX()
	{
		return m_xTranslate;
	}

	inline float getTranslationZ()
	{
		return m_zTranslate;
	}

private:

	enum State
	{
		Ground,
		Water,
		Boundary, // for reflecting only
		NearBoundary,
		Object,
		ObjectBoundary
	};

	struct GridCell
	{
		float y;  //position of gridcell
		State state;
		float xVelocity, zVelocity, temp, waterHeight;
	};

	GridCell* m_pGrids;

	PortScene* m_pPortScene;

	float m_xTranslate, m_zTranslate;
	int m_newObjectCellIndices[2000], m_newNumObjectCellIndices;

	void moveSWEGrid(const Vector3& cameraView);
	void advectHeight();
	void advectVelocityX();
	void advectVelocityZ();
	void updateHeight();
	void updateVelocities();
	void updateNormals();
	void absorbingBoundaries();
	void reflectBoundaries();
	void freeSurface();
	void createNewCell( int i,  int j);
	void bodyInteraction();
	void findObjectCellsOnGrid(int  &minX, int &maxX, int &minZ, int &maxZ);
	void findObjectBoundaryOnGrid(int minX, int maxX, int minZ, int maxZ);
	void resetGrid();
	float getGroundHeight(float x, float z);
	void boundaryCheck();


	inline float getRandom(float min=0., float max=1.)
	{
		return min+( (rand()/(RAND_MAX+1.0)) *(max-min) );  
	}


	inline float interpolate( float x, float z,  float x1,  float x2,  float y1,  float y2) {
		
		const int X = (int)x;
		const int Z = (int)z;
		const float s1 = x - X;
		const float s0 = 1.f - s1;
		const float t1 = z - Z;
		const float t0 = 1.f - t1;

		return  s0*(t0* x1 + t1*x2 )+ s1*(t0*y1  + t1*y2 );
	}
				
};