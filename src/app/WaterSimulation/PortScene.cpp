/*
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 */

#include "PortScene.h"

#include "iostream"

PortScene::PortScene()
{
	m_cellEdge = .0f;
	m_Xmax = .0f;
	m_Zmax = .0f;

	initialize();
}

PortScene::~PortScene()
{
	for(int j=0; j<UNIFORM_GRIDCELL; j++) {
		for(int i=0; i<UNIFORM_GRIDCELL; i++) {
			const int index = i + j*UNIFORM_GRIDCELL;
			delete[] m_uniformGrid[index].faceId;
			delete[] m_uniformGrid[index].groupId;
		}
	}

	delete[] m_textureId;
	delete[] m_uniformGrid;
}

void PortScene::initialize()
{
	std::string filename ("Data/port.obj");
	m_portScene.objectLoader(filename);

	calculateHeightForLowLevelGrid();

	unsigned char *textureBuffer;
	FILE *textureFile;
	long size;
	size_t result;
	ImageDesc imageDesc;
	short textureCount = 0;

	m_textureId = new GLuint[m_portScene.m_material.size()];

	for(int i=0; i<m_portScene.m_material.size(); i++) {
		if(!m_portScene.m_material[i].textureFileName.empty()) {
		
			textureFile = fopen(m_portScene.m_material[i].textureFileName.c_str(), "rb");
			fseek(textureFile, 0, SEEK_END);
			size = ftell(textureFile);
			rewind(textureFile);
			textureBuffer = (unsigned char*)malloc(sizeof(unsigned char)*size);
			result = fread(textureBuffer, 1, size, textureFile);
			PNGUtil::loadImage(textureBuffer, result, imageDesc);
			fclose(textureFile);
			free(textureBuffer);

			glGenTextures(1, &m_textureId[textureCount]);
			
			glBindTexture(GL_TEXTURE_2D, m_textureId[textureCount]);
			gluBuild2DMipmaps(GL_TEXTURE_2D, 3, imageDesc.m_width, imageDesc.m_height, GL_RGB, GL_UNSIGNED_BYTE, imageDesc.m_pBuffer);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

		}
		textureCount++;
	}

	createVBO();
}

void PortScene::calculateHeightForLowLevelGrid() 
{
	float GridLength;
	partitionTriangleInUniformGrid(GridLength);
	calculateMaxGroundHeight(GridLength);

}

void PortScene::partitionTriangleInUniformGrid(float& GridLength)
{

	float diffx = m_portScene.m_maxX - m_portScene.m_minX;
	float diffz = m_portScene.m_maxZ - m_portScene.m_minZ;

	if(diffx>diffz) {
		GridLength = diffx;
		m_Xmax = m_portScene.m_maxX;
		m_Zmax = m_portScene.m_minZ + GridLength;
	} else {
		GridLength = diffz;
		m_Xmax = m_portScene.m_minX + GridLength;
		m_Zmax = m_portScene.m_maxZ;
	}

	m_cellEdge = GridLength/(float)NUM_GRIDCELLS;

	m_uniformGridCellEdge = GridLength/(float)UNIFORM_GRIDCELL;
	m_uniformGrid = new uniformGrid[UNIFORM_GRIDCELL*UNIFORM_GRIDCELL];

	for(int j=0; j<UNIFORM_GRIDCELL; j++) {
		for(int i=0; i<UNIFORM_GRIDCELL; i++) {

			const int index = i + j*UNIFORM_GRIDCELL;
			m_uniformGrid[index].idIndex = 0;
			m_uniformGrid[index].faceId = new int[10000];
			m_uniformGrid[index].groupId = new int[10000];
		}
	}

	const float factor = 1.0f/m_uniformGridCellEdge;

	for(int k=0; k<m_portScene.m_groups.size(); k++) { // loop through all faces to see in wichi grids they lie
		for(int l=0; l<m_portScene.m_groups[k].faces.size(); l++) {

			const int offset = 24*l;
			int minX, maxX, minZ, maxZ; // min and max indices in x and z direction, bounding box for the triangle
			float ffx[3], ffz[3], FDX[3], FDZ[3]; 

			ffx[0] = (m_Xmax - m_portScene.m_groups[k].vertexData[offset])*factor;
			ffz[0] = (m_Zmax - m_portScene.m_groups[k].vertexData[offset+2])*factor;
			
			minX = (int)ffx[0]; 
			maxX = (int)ceilf(ffx[0]);
			minZ = (int)ffz[0];
			maxZ = (int)ceilf(ffz[0]);

			ffx[1] = (m_Xmax - m_portScene.m_groups[k].vertexData[offset + 8])*factor;
			ffz[1] = (m_Zmax - m_portScene.m_groups[k].vertexData[offset + 8+2])*factor;

			ffx[2] = (m_Xmax - m_portScene.m_groups[k].vertexData[offset + 16])*factor;
			ffz[2] = (m_Zmax - m_portScene.m_groups[k].vertexData[offset + 16+2])*factor;

			minX = std::min(minX, std::min((int)ffx[1], (int)ffx[2]));
			minZ = std::min(minZ, std::min((int)ffz[1], (int)ffz[2]));

			maxX = std::max(maxX, std::max((int)ceilf(ffx[1]), (int)ceilf(ffx[2])));
			maxZ = std::max(maxZ, std::max((int)ceilf(ffz[1]), (int)ceilf(ffz[2])));

			if(minX<0) {
				minX=0;
			}

			if(minZ<0) {
				minZ=0;
			}

			if(minX>=UNIFORM_GRIDCELL) {
				minX = UNIFORM_GRIDCELL-1;
			}

			if(minZ>=UNIFORM_GRIDCELL) {
				minZ = UNIFORM_GRIDCELL-1;
			}

			if(maxX<0) {
				maxX=0;
			}

			if(maxZ<0) {
				maxZ=0;
			}

			if(maxX>=UNIFORM_GRIDCELL) {
				maxX = UNIFORM_GRIDCELL-1;
			}

			if(maxZ>=UNIFORM_GRIDCELL) {
				maxZ = UNIFORM_GRIDCELL-1;
			}

			FDX[0] = ffx[0] - ffx[1];
			FDX[1] = ffx[1] - ffx[2];
			FDX[2] = ffx[2] - ffx[0];

			FDZ[0] = ffz[0] - ffz[1];
			FDZ[1] = ffz[1] - ffz[2];
			FDZ[2] = ffz[2] - ffz[0];

			float C[3], CZ[3], CX[3];

			for(int i=0; i<3; i++) {

				C[i] = FDZ[i]*ffx[i] - FDX[i]*ffz[i]; 
				CZ[i] = C[i] + FDX[i]*minZ - FDZ[i]*minX;

			}


			for(int z=minZ; z<=maxZ; z++) {

				for(int i=0; i<3; i++) {
					CX[i] = CZ[i];
				}

				for(int x=minX; x<=maxX; x++) {

					bool Passed = 1;

					for(int i=0; i<3; i++) {
			
						if(!(CX[i]>-1.0f)) {
							Passed = 0;
							break;
						}

					}

						const int index = x + z*UNIFORM_GRIDCELL;
						
						m_uniformGrid[index].groupId[m_uniformGrid[index].idIndex] = k;
						m_uniformGrid[index].faceId[m_uniformGrid[index].idIndex] = l;

						m_uniformGrid[index].idIndex++;

						if(m_uniformGrid[index].idIndex>10000) {
							exit(0);
						}
					

					for(int i=0; i<3; i++) {
						CX[i] -= FDZ[i];
					}
				}

				for(int i=0; i<3; i++) {
					CZ[i] += FDX[i];
				}
			}

		}
	}

}

void PortScene::calculateMaxGroundHeight(float GridLength)
{
	const float factor = 1.0f/m_uniformGridCellEdge;

	for(int j=0; j<NUM_GRIDCELLS; j++) { // calculate ground height at some points
		for(int i=0; i<NUM_GRIDCELLS; i++) {
			
			const int index = i + j*NUM_GRIDCELLS;

			m_gridPoints[index].v[0] = m_Xmax - m_cellEdge*(float)i;
			m_gridPoints[index].v[2] = m_Zmax - m_cellEdge*(float)j;

			float minDistancefromSky = 500.0f;

			if(!((m_gridPoints[index].v[0] > m_portScene.m_maxX) || (m_gridPoints[index].v[0] < m_portScene.m_minX) || (m_gridPoints[index].v[2] > m_portScene.m_maxZ) || (m_gridPoints[index].v[2] < m_portScene.m_minZ))) {

				int xc = (int)((m_Xmax - m_gridPoints[index].v[0])*factor);
				int zc = (int)((m_Zmax - m_gridPoints[index].v[2])*factor);

				// uniform grid where the point (i,j) lies
				const int uniformGridIndex = xc + zc*UNIFORM_GRIDCELL;

				for(int k=0; k<m_uniformGrid[uniformGridIndex].idIndex; k++) { // loop through all triangles in the uniform grid

					Vector3 tp[3];

					for(int m=0; m<3; m++) { // all vertices of the triangle

						const int offset = 24*m_uniformGrid[uniformGridIndex].faceId[k] + m*8;

						tp[m].v[0] = m_portScene.m_groups[m_uniformGrid[uniformGridIndex].groupId[k]].vertexData[offset];
						tp[m].v[1] = m_portScene.m_groups[m_uniformGrid[uniformGridIndex].groupId[k]].vertexData[offset + 1];
						tp[m].v[2] = m_portScene.m_groups[m_uniformGrid[uniformGridIndex].groupId[k]].vertexData[offset + 2];
					}
							
					float distance;
					Vector3 rayOrigin(m_gridPoints[index].v[0], 150.0f, m_gridPoints[index].v[2]);
					Vector3 rayDir(0.0f, -1.0f, 0.0f);

					if(MathUtil::rayTriangleIntersect3D(rayOrigin, rayDir, tp[0], tp[1], tp[2], distance)) {  // shoot ray from y = 150.0f to the triangle
						minDistancefromSky = std::min(minDistancefromSky, distance);
					}
				}
					

				if(minDistancefromSky > 400.0f) { // this means ray doesnt intersect any triangle as some gridcells may not have any triangles
					m_gridPoints[index].v[1] = 200.0f;
				} else {
					m_gridPoints[index].v[1] = 150.0f - minDistancefromSky;
				}

			}
		}
	}

}

float PortScene::getGroundHeight(float x, float z) {

	float groundHeight = -100.0f;

	int imax, jmax;

	int imin = (int)((m_Xmax - x)/m_cellEdge);
	int jmin = (int)((m_Zmax - z)/m_cellEdge);

	if((imin<=0) || (jmin<=0) || (imin>=(NUM_GRIDCELLS-1)) || (jmin>=(NUM_GRIDCELLS-1))) {
		groundHeight =  -1.0f;
	} else {

		if(imin == (NUM_GRIDCELLS-1)) {
			imax = imin;
		} else {
			imax = imin + 1;
		}

		if(jmin == (NUM_GRIDCELLS-1)) {
			jmax = jmin;
		} else {
			jmax = jmin + 1;
		}

		const int index22 = imin + jmin*NUM_GRIDCELLS;
		const int index12 = imax + jmin*NUM_GRIDCELLS;
		const int index21 = imin + jmax*NUM_GRIDCELLS;
		const int index11 = imax + jmax*NUM_GRIDCELLS;

		const float denom = 1.0f/((m_gridPoints[index22].v[0] - m_gridPoints[index12].v[0])*(m_gridPoints[index22].v[2] - m_gridPoints[index21].v[2]));

		const float x2_x = m_gridPoints[index22].v[0] - x;
		const float x_x1 = x - m_gridPoints[index12].v[0];
		const float z2_z = m_gridPoints[index22].v[2] - z;
		const float z_z1 = z - m_gridPoints[index21].v[2];

		// bilinear interpolation using height from 4 closest neighbours
		groundHeight = (m_gridPoints[index22].v[1]*x_x1*z_z1 + m_gridPoints[index12].v[1]*x2_x*z_z1 + m_gridPoints[index21].v[1]*x_x1*z2_z + m_gridPoints[index22].v[1]*x2_x*z2_z)*denom;

	}

	return groundHeight;
	
}

void PortScene::createVBO()
{
	m_vertexVBOIdPort = new GLuint[m_portScene.m_groups.size()];
	m_indexVBOIdPort = new GLuint[m_portScene.m_groups.size()];

	for(int i=0; i<m_portScene.m_groups.size(); i++) {
		glGenBuffers(1, &(m_vertexVBOIdPort[i]));
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBOIdPort[i]);
		glBufferData(GL_ARRAY_BUFFER, m_portScene.m_groups[i].faces.size()*24*sizeof(float), m_portScene.m_groups[i].vertexData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &(m_indexVBOIdPort[i]));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBOIdPort[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_portScene.m_groups[i].faces.size()*3*sizeof(GLuint), m_portScene.m_groups[i].indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	}
}

void PortScene::renderPort()
{
	glPushMatrix();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glEnable(GL_TEXTURE_2D);  

	for(int i=0; i<m_portScene.m_groups.size(); i++) {

		glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBOIdPort[i]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBOIdPort[i]);

		
		glBindTexture(GL_TEXTURE_2D, m_textureId[m_portScene.m_groups[i].materialIndex]);

		glVertexPointer( 3,   //3 components per vertex (x,y,z)
						GL_FLOAT,
						8*sizeof(float),
						NULL);
		glNormalPointer( GL_FLOAT,
						8*sizeof(float),
						(void*)(3*sizeof(float)));
		glTexCoordPointer( 2,
						  GL_FLOAT,
						  8*sizeof(float),
						  (void*)(6*sizeof(float)));

		glDrawElements(	GL_TRIANGLES, //mode
						m_portScene.m_groups[i].faces.size()*3,  //count, ie. how many indices
						GL_UNSIGNED_INT, //type of the index array
						NULL);
	
	}

	glDisable(GL_TEXTURE_2D);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glEnd();

	glPopMatrix();
}
