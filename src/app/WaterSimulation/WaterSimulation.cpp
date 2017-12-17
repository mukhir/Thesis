/*
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 */

#include "WaterSimulation.h"

const float WaterSimulation::FLAT = 2.0f;
const float WaterSimulation::TOTAL_HEIGHT = 6.0f;
const float WaterSimulation::CELL_EDGE = 1.5f;
const float WaterSimulation::INV_DIST = 1.0f/CELL_EDGE;
const float WaterSimulation::TIME_STEP = 0.03f;
const float WaterSimulation::GRAVITY = -9.81f;
const float WaterSimulation::DISPLACED_HEIGHT = 0.2f;
const float WaterSimulation::UNDER_WATER = TOTAL_HEIGHT;
const float WaterSimulation::BOUNDARY_THRESHOLD = TOTAL_HEIGHT;
const float WaterSimulation::GRIDSTART_X = CELL_EDGE*NUM_CELLS/2.0f;
const float WaterSimulation::GRIDSTART_Z = GRIDSTART_X;

WaterSimulation::WaterSimulation(PortScene* portScene)
{
	m_xTranslate = m_zTranslate = 0.0f;
	m_xVelocity = m_zVelocity = .0f;
	m_newNumObjectCellIndices = 0;
	m_pPortScene = portScene;
}

WaterSimulation::~WaterSimulation()
{
	delete[] m_pGrids;
}

void WaterSimulation::initializeGrid()
{

	m_pGrids = new GridCell[NUM_GRIDS];

	for (int zc = 0; zc < NUM_CELLS; zc++) {
		for (int xc = 0; xc < NUM_CELLS; xc++) {

			const int index = xc + zc*NUM_CELLS;

			m_pGrids[index].xVelocity = m_pGrids[index].zVelocity = m_pGrids[index].temp = .0f;

			float x = GRIDSTART_X + m_xTranslate - CELL_EDGE*float(xc);
			float z = GRIDSTART_Z + m_zTranslate - CELL_EDGE*float(zc);
				
			m_pGrids[index].y = TOTAL_HEIGHT; // Initial height of grid
			m_pGrids[index].waterHeight = m_pGrids[index].y - getGroundHeight(x,z);
			//m_pGrids[index].waterHeight = m_pGrids[index].y - FLAT;

			if(m_pGrids[index].waterHeight < .0f) {

				m_pGrids[index].state = Ground;
				m_pGrids[index].y = UNDER_WATER;  // render below terrain
				m_pGrids[index].waterHeight = TOTAL_HEIGHT - FLAT;

			} else {
				m_pGrids[index].state = Water;
			}
		}	
	}

	boundaryCheck();
	
}

void WaterSimulation::update(const Vector3& cameraView)
{
	moveSWEGrid(cameraView);

	resetGrid();
	
	advectHeight(); //waterheight
	advectVelocityX(); //xVelocity
	advectVelocityZ(); //zVelocity
	
	updateHeight();
	updateVelocities();

	reflectBoundaries();
	absorbingBoundaries();
	
	bodyInteraction();
	
}

void WaterSimulation::moveSWEGrid(const Vector3& cameraView)
{
	const int midIndex = (NUM_CELLS/2 - 1)*NUM_CELLS + (NUM_CELLS/2 - 1);

	float x = GRIDSTART_X + m_xTranslate - CELL_EDGE*float(NUM_CELLS/2 - 1);
	float z = GRIDSTART_Z + m_zTranslate - CELL_EDGE*float(NUM_CELLS/2 - 1);

	Vector3 midGrid(x, m_pGrids[midIndex].y, z);
	Vector3 dir;
	dir.sub(cameraView, midGrid); // move the grid so that the camera view is always following the centre of SWE grid by copying and creating cells
	
	if(dir[0] > CELL_EDGE) {

		const int numCells = dir[0]/CELL_EDGE;

		m_xTranslate +=  numCells*CELL_EDGE;

		for(int j=NUM_CELLS-1 ; j>=0; j--) {
			for(int i=NUM_CELLS-1 ; i>=0; i--) {

				const int index = i + j*NUM_CELLS;

				if(i>numCells) {
					m_pGrids[index] = m_pGrids[index-numCells];
					m_pGrids[index] = m_pGrids[index-numCells];
				}
				else 
					createNewCell(i,j);
					
			}
		}
	} else if(dir[0] < -CELL_EDGE) {

		const int numCells = abs(dir[0]/CELL_EDGE);
		
		m_xTranslate -=  numCells*CELL_EDGE;

		for(int i=0 ; i<NUM_CELLS; i++) {
			for(int j=0 ; j<NUM_CELLS; j++) {

				const int index = i + j*NUM_CELLS;

				if(i<(NUM_CELLS-numCells)) {
					m_pGrids[index] = m_pGrids[index+numCells];
					m_pGrids[index] = m_pGrids[index+numCells];
				}
				else 
					createNewCell(i,j);
			}
		}
	}

	if(dir[2] > CELL_EDGE) {

		const int numCells = dir[2]/CELL_EDGE;

		m_zTranslate +=  numCells*CELL_EDGE;

		for(int i=NUM_CELLS-1 ; i>=0; i--) {
			for(int j=NUM_CELLS-1 ; j>=0; j--) {

				const int index = i + j*NUM_CELLS;
				const int shift = numCells*NUM_CELLS;

				if(j>numCells) {
					m_pGrids[index] = m_pGrids[index - shift];
					m_pGrids[index] = m_pGrids[index - shift];
				}
				else 
					createNewCell(i,j);
				
			}
		}
	} else if(dir[2] < -CELL_EDGE) {

		const int numCells = abs(dir[2]/CELL_EDGE);

		m_zTranslate -=  numCells*CELL_EDGE;

		for(int i=0 ; i<NUM_CELLS; i++) {
			for(int j=0 ; j<NUM_CELLS; j++){

				const int index = i + j*NUM_CELLS;
				const int shift = numCells*NUM_CELLS;

				if(j<(NUM_CELLS-numCells)) {
					m_pGrids[index] = m_pGrids[index + shift];
					m_pGrids[index] = m_pGrids[index + shift];
				}
				else {
					createNewCell(i,j);
				}
			}	
		}
	}

	boundaryCheck();  // after creating new cells

}

void WaterSimulation::boundaryCheck()
{

	for (int xc = 0; xc < NUM_CELLS; xc++) {
		for (int zc = 0; zc < NUM_CELLS; zc++) {

			const int index = xc + zc*NUM_CELLS;

			int index_right, index_left, index_top, index_bottom;
			float xRight, xLeft, xTop, xBottom, zRight, zLeft, zTop, zBottom;

			if( xc == 0) {
				index_right = index;
				xRight = GRIDSTART_X + m_xTranslate - CELL_EDGE*float(xc);
			} else {
				index_right = index - 1;
				xRight = GRIDSTART_X + m_xTranslate - CELL_EDGE*float(xc-1);
			}

			zRight = GRIDSTART_Z + m_zTranslate - CELL_EDGE*float(zc);

			if( xc == NUM_CELLS-1) {
				index_left = index;
				xLeft = GRIDSTART_X + m_xTranslate - CELL_EDGE*float(xc);
			} else {
				index_left = index + 1;
				xLeft = GRIDSTART_X + m_xTranslate - CELL_EDGE*float(xc+1);
			}

			zLeft = GRIDSTART_Z + m_zTranslate - CELL_EDGE*float(zc);

			if( zc == 0) {
				index_bottom = index;
				zBottom = GRIDSTART_Z + m_zTranslate - CELL_EDGE*float(zc);
			} else {
				index_bottom = index - NUM_CELLS;
				zBottom = GRIDSTART_Z + m_zTranslate - CELL_EDGE*float(zc-1);
			}

			xBottom = GRIDSTART_X + m_xTranslate - CELL_EDGE*float(xc);

			if( zc == NUM_CELLS-1) {
				index_top = index;
				zTop = GRIDSTART_Z + m_zTranslate - CELL_EDGE*float(zc);
			} else {
				index_top = index + NUM_CELLS;
				zTop = GRIDSTART_Z + m_zTranslate - CELL_EDGE*float(zc+1);
			}
				
			xTop = GRIDSTART_X + m_xTranslate - CELL_EDGE*float(xc);

			const float groundHeight = getGroundHeight(GRIDSTART_X + m_xTranslate - CELL_EDGE*float(xc), GRIDSTART_Z + m_zTranslate - CELL_EDGE*float(zc));

			if(m_pGrids[index].state == Water) { // check difference in height with the neighbouring cells

				if((m_pGrids[index_right].state == Ground) && ((getGroundHeight(xRight,zRight) - groundHeight) > BOUNDARY_THRESHOLD)) {
					m_pGrids[index].state = Boundary; 
				} else if ((m_pGrids[index_left].state == Ground) && ((getGroundHeight(xLeft,zLeft) - groundHeight) > BOUNDARY_THRESHOLD)) {
					m_pGrids[index].state = Boundary; 
				} else if ((m_pGrids[index_bottom].state == Ground) && ((getGroundHeight(xBottom,zBottom) - groundHeight) > BOUNDARY_THRESHOLD)) {
					m_pGrids[index].state = Boundary; 
				} else if((m_pGrids[index_top].state == Ground) && ((getGroundHeight(xTop,zTop) - groundHeight) > BOUNDARY_THRESHOLD)) {
					m_pGrids[index].state = Boundary;
				}
			}
		}
	}

	for (int xc = 1; xc < NUM_CELLS-1; xc++) 
		for (int zc = 1; zc < NUM_CELLS-1; zc++) 
		{
			const int index = xc + zc*NUM_CELLS;

			if(m_pGrids[index].state == Water){

				if((m_pGrids[index+1].state == Boundary) || (m_pGrids[index-1].state == Boundary) || (m_pGrids[index+NUM_CELLS].state == Boundary) || (m_pGrids[index-NUM_CELLS].state == Boundary)
					|| (m_pGrids[index+1+NUM_CELLS].state == Boundary) || (m_pGrids[index+1-NUM_CELLS].state == Boundary)
					|| (m_pGrids[index-1+NUM_CELLS].state == Boundary) || (m_pGrids[index-1-NUM_CELLS].state == Boundary))
					m_pGrids[index].state = NearBoundary;
	
			}
		}

}

void WaterSimulation::createNewCell( int i,  int j)
{
	const float random = 0.1f;
	float getRand;

	const int index = i + j*NUM_CELLS;

	const float x = GRIDSTART_X + m_xTranslate - CELL_EDGE*float(i);
	const float z = GRIDSTART_Z + m_zTranslate - CELL_EDGE*float(j);

	m_pGrids[index].y = TOTAL_HEIGHT; // Initial height of grid
	m_pGrids[index].waterHeight = m_pGrids[index].y - getGroundHeight(x,z);

	if(m_pGrids[index].waterHeight < .0f) {

		m_pGrids[index].state = Ground;
		m_pGrids[index].y = UNDER_WATER;  // render below terrain
		m_pGrids[index].waterHeight = TOTAL_HEIGHT - TOTAL_HEIGHT*0.9f;
	
	} else {

		m_pGrids[index].state = Water;
		getRand = getRandom();

		if(getRand<random) {
			m_pGrids[index].y = TOTAL_HEIGHT + getRand;
		} else {
			m_pGrids[index].y = TOTAL_HEIGHT;
		}

	}

	m_pGrids[index].xVelocity = m_pGrids[index].zVelocity = m_pGrids[index].temp = .0f;	

}

void WaterSimulation::resetGrid()
{
	for (int j=0; j< NUM_CELLS ;j++) {
		for (int i=0; i< NUM_CELLS ;i++) {

			const int index = i + j*NUM_CELLS;

			if((m_pGrids[index].state == Object) || (m_pGrids[index].state == ObjectBoundary) ) {
				m_pGrids[index].state = Water;
				//m_pGrids[index].waterHeight = TOTAL_HEIGHT - FLAT;
			}
		}
	}
}

void WaterSimulation::advectHeight()
{
	float x1, x2, y1, y2;
	int X, Z;

	for(int j=1; j< NUM_CELLS-1; j++) {
		for(int i=1; i< NUM_CELLS-1; i++) {

			const int index = i + j*NUM_CELLS;

			if((m_pGrids[index].state == Water) || (m_pGrids[index].state == NearBoundary)) {

				float u = 0.0f, v = 0.0f; 

				u += (m_pGrids[index].xVelocity + m_pGrids[index+1].xVelocity) *0.5f;
				v += (m_pGrids[index].zVelocity + m_pGrids[index+NUM_CELLS].zVelocity) *0.5f;

				// backtrace position
				float srcpi = (float)i - u * TIME_STEP * INV_DIST;
				float srcpj = (float)j - v * TIME_STEP * INV_DIST;

				// clamp range of accesses
				if(srcpi<0.) srcpi = .0f;
				if(srcpj<0.) srcpj = .0f;
				if(srcpi>NUM_CELLS-1.0f) srcpi = NUM_CELLS-1.;
				if(srcpj>NUM_CELLS-1.0f) srcpj = NUM_CELLS-1.;

				X = (int)srcpi;
				Z = (int)srcpj;

				x1 = m_pGrids[X+NUM_CELLS*Z].waterHeight;
				x2 = m_pGrids[X  +NUM_CELLS*(Z+1)].waterHeight;
				y1 = m_pGrids[(X+1)+NUM_CELLS*Z].waterHeight;
				y2 = m_pGrids[(X+1)+NUM_CELLS*(Z+1)].waterHeight;

				// interpolate source value
				m_pGrids[index].temp = interpolate(srcpi, srcpj, x1, x2, y1, y2);
			}
		}
	}

	for (int j=1;j<NUM_CELLS-1;j++) {
		for (int i=1;i<NUM_CELLS-1;i++) {

			const int index = i + j*NUM_CELLS;

			if((m_pGrids[index].state != Boundary)) {
				m_pGrids[index].waterHeight = m_pGrids[index].temp;

				/*if(m_pGrids[index].waterHeight < -5.0f) {
					exit(0);
				}*/

			}
					
		}
	}
}

void WaterSimulation::advectVelocityX()
{

	for(int j=1; j< NUM_CELLS-1; j++) {
		for(int i=1; i< NUM_CELLS-1; i++) {
			const int index = i + j*NUM_CELLS;

			if((m_pGrids[index].state == Water) || (m_pGrids[index].state == NearBoundary)) {

				float u = 0.0f, v = 0.0f; 
				
				u += m_pGrids[index].xVelocity;
				v += (m_pGrids[index].zVelocity + m_pGrids[index+1].zVelocity + m_pGrids[index+NUM_CELLS].zVelocity + m_pGrids[index+NUM_CELLS+1].zVelocity) *0.25f;
					

				// backtrace position
				float srcpi = (float)i - u * TIME_STEP * INV_DIST;
				float srcpj = (float)j - v * TIME_STEP * INV_DIST;

				// clamp range of accesses
				if(srcpi<0.) srcpi = .0f;
				if(srcpj<0.) srcpj = .0f;
				if(srcpi>NUM_CELLS-1.0f) srcpi = NUM_CELLS-1.;
				if(srcpj>NUM_CELLS-1.0f) srcpj = NUM_CELLS-1.;

				int X = (int)srcpi;
				int Z = (int)srcpj;

				float x1 = m_pGrids[X+NUM_CELLS*Z].xVelocity;
				float x2 = m_pGrids[X  +NUM_CELLS*(Z+1)].xVelocity;
				float y1 = m_pGrids[(X+1)+NUM_CELLS*Z].xVelocity;
				float y2 = m_pGrids[(X+1)+NUM_CELLS*(Z+1)].xVelocity;

				// interpolate source value
				m_pGrids[index].temp = interpolate(srcpi, srcpj, x1, x2, y1, y2);
			}
		}
	}

	for (int j=1;j<NUM_CELLS-1;j++) {
		for (int i=1;i<NUM_CELLS-1;i++) {

			const int index = i + j*NUM_CELLS;

			if((m_pGrids[index].state != Boundary)) 
				m_pGrids[index].xVelocity = m_pGrids[index].temp;
						
		}
	}
}

void WaterSimulation::advectVelocityZ()
{
	
	for(int j=1; j< NUM_CELLS-1; j++) {
		for(int i=1; i< NUM_CELLS-1; i++) {

			const int index = i + j*NUM_CELLS;

			if((m_pGrids[index].state == Water) || (m_pGrids[index].state == NearBoundary)) {

				float u = 0.0f, v = 0.0f; 
				
				u += (m_pGrids[index].xVelocity + m_pGrids[index+1].xVelocity + m_pGrids[index+NUM_CELLS].xVelocity + m_pGrids[index+NUM_CELLS+1].xVelocity) *0.25f;
				v += m_pGrids[index].zVelocity;
					

				// backtrace position
				float srcpi = (float)i - u * TIME_STEP * INV_DIST;
				float srcpj = (float)j - v * TIME_STEP * INV_DIST;

				// clamp range of accesses
				if(srcpi<0.) srcpi = .0f;
				if(srcpj<0.) srcpj = .0f;
				if(srcpi>NUM_CELLS-1.0f) srcpi = NUM_CELLS-1.;
				if(srcpj>NUM_CELLS-1.0f) srcpj = NUM_CELLS-1.;

				int X = (int)srcpi;
				int Z = (int)srcpj;

				float x1 = m_pGrids[X+NUM_CELLS*Z].zVelocity;
				float x2 = m_pGrids[X  +NUM_CELLS*(Z+1)].zVelocity;
				float y1 = m_pGrids[(X+1)+NUM_CELLS*Z].zVelocity;
				float y2 = m_pGrids[(X+1)+NUM_CELLS*(Z+1)].zVelocity;

				// interpolate source value
				m_pGrids[index].temp = interpolate(srcpi, srcpj, x1, x2, y1, y2);
			}
		}
	}

	for (int j=1;j<NUM_CELLS-1;j++) {
		for (int i=1;i<NUM_CELLS-1;i++) {

			const int index = i + j*NUM_CELLS;

			if((m_pGrids[index].state != Boundary)) 
				m_pGrids[index].zVelocity = m_pGrids[index].temp;
						
		}
	}
}

void WaterSimulation::updateHeight() 
{
	// update heights as per shallow water equation
	for (int j=1;j<NUM_CELLS-1;j++) {
		for (int i=1;i<NUM_CELLS-1;i++) {

			const int index = i + j*NUM_CELLS;

			if((m_pGrids[index].state != Boundary) && (m_pGrids[index].state != Ground) ) {

					float dh = -0.5 * m_pGrids[index].waterHeight * INV_DIST * (
						(m_pGrids[index+1].xVelocity  - m_pGrids[index].xVelocity) +
						(m_pGrids[index+NUM_CELLS].zVelocity - m_pGrids[index].zVelocity) );

					m_pGrids[index].waterHeight += dh * TIME_STEP;

					const float x = GRIDSTART_X + m_xTranslate - CELL_EDGE*float(i);
					const float z = GRIDSTART_Z + m_zTranslate - CELL_EDGE*float(j);

					if (m_pGrids[index].state == Ground) {
						m_pGrids[index].y = m_pGrids[index].waterHeight + FLAT + TOTAL_HEIGHT*0.9f;
					} else {
						m_pGrids[index].y = getGroundHeight(x,z) + m_pGrids[index].waterHeight;
					}

			} 
		}	
	}

	for (int j=0;j<NUM_CELLS;j++) {
			for (int i=0;i<NUM_CELLS;i++) {
				if(((i==0)||(i==NUM_CELLS-1)||(j==0)||(j==NUM_CELLS-1))) {// Height should be 0 at SWE grid borders
					m_pGrids[i + j*NUM_CELLS].y  = TOTAL_HEIGHT; 
				}
			}
	}

}

void WaterSimulation::updateVelocities()
{ 
	// accelerate velocities as per SWE
	for (int j=1; j< NUM_CELLS-1 ;j++) {
		for (int i=1; i< NUM_CELLS-1 ;i++) {

			const int index = i + j*NUM_CELLS;

			if((m_pGrids[index].state == Water) || (m_pGrids[index].state == NearBoundary)) {

				m_pGrids[index].xVelocity += GRAVITY * TIME_STEP * INV_DIST * (m_pGrids[index].y - m_pGrids[index-1].y); 
				m_pGrids[index].zVelocity += GRAVITY * TIME_STEP * INV_DIST * (m_pGrids[index].y - m_pGrids[index-NUM_CELLS].y); 
			}
		} 
	}

}

void WaterSimulation::absorbingBoundaries()
{

	const float HeightFFT = TOTAL_HEIGHT - FLAT;
	const float inv_gridLength = 1.0f/(NUM_CELLS*CELL_EDGE);

	for(int j=0; j<NUM_CELLS; j++) {
		for(int i=0; i<NUM_CELLS; i++)  {

			const int index = i + j*NUM_CELLS;

			//if(m_pGrids[index].state != Ground ) {
			
				if((i<=NUM_BORDER_DAMPING_CELLS) || (j<=NUM_BORDER_DAMPING_CELLS) || (i>=(NUM_CELLS-NUM_BORDER_DAMPING_CELLS)) || (j>=(NUM_CELLS-NUM_BORDER_DAMPING_CELLS))) {

					float x = GRIDSTART_X - CELL_EDGE*float(i);
					float z = GRIDSTART_Z - CELL_EDGE*float(j);
					float dx, dz;

					if(x<0.0)
						dx = (0.0 - x)*inv_gridLength;
					else
						dx = (x - 0.0)*inv_gridLength;

					if(z<0.0)
						dz = (0.0 - z)*inv_gridLength;
					else
						dz = (z - 0.0)*inv_gridLength;

					float factor;

					if(dx>dz) {
						factor = dx;
					} else {
						factor = dz;
					}

					// damp height and velocities near SWE border by factor
					m_pGrids[index].y -= (m_pGrids[index].y - TOTAL_HEIGHT)*factor;
					m_pGrids[index].xVelocity -= factor*(m_pGrids[index].xVelocity);
					m_pGrids[index].zVelocity -= factor*(m_pGrids[index].zVelocity);
				
				//}
			}

		}
	}

}

void WaterSimulation::reflectBoundaries()
{
	
	for (int j=0; j<NUM_CELLS; j++) {
		for (int i=0; i<NUM_CELLS; i++) {

			const int index = i + j*NUM_CELLS;

			if(m_pGrids[index].state == Boundary) {

				// copy height from NearBoundary Cell to Boundary cell
				if(m_pGrids[index+1].state == NearBoundary) {

					m_pGrids[index].y = m_pGrids[index+1].y;
					m_pGrids[index].xVelocity = .0f;

				} else if(m_pGrids[index-1].state == NearBoundary) {

					m_pGrids[index].y = m_pGrids[index-1].y;
					m_pGrids[index].xVelocity = .0f;

				} else if(m_pGrids[index+NUM_CELLS].state == NearBoundary) {

					m_pGrids[index].y = m_pGrids[index+NUM_CELLS].y;
					m_pGrids[index].zVelocity = .0f;

				} else if(m_pGrids[index-NUM_CELLS].state == NearBoundary) {

					m_pGrids[index].y = m_pGrids[index-NUM_CELLS].y;
					m_pGrids[index].zVelocity = .0f;

				} else if(m_pGrids[index -1 -NUM_CELLS].state == NearBoundary) {

					m_pGrids[index].y = m_pGrids[index- 1 - NUM_CELLS].y;
					m_pGrids[index].xVelocity = .0f;
					m_pGrids[index].zVelocity = .0f;

				} else if(m_pGrids[index +1 -NUM_CELLS].state == NearBoundary) {

					m_pGrids[index].y = m_pGrids[index +1 -NUM_CELLS].y;
					m_pGrids[index].xVelocity = .0f;
					m_pGrids[index].zVelocity = .0f;

				} else if(m_pGrids[index -1 +NUM_CELLS].state == NearBoundary) {

					m_pGrids[index].y = m_pGrids[index -1 + NUM_CELLS].y;
					m_pGrids[index].xVelocity = .0f;
					m_pGrids[index].zVelocity = .0f;

				} else if(m_pGrids[index +1 +NUM_CELLS].state == NearBoundary) {

					m_pGrids[index].y = m_pGrids[index +1 +NUM_CELLS].y; 
					m_pGrids[index].xVelocity = .0f;
					m_pGrids[index].zVelocity = .0f;
				} else {
					m_pGrids[index].y = TOTAL_HEIGHT;
				}

			}
		}
	}
}

void WaterSimulation::fillVertexBufferandUpdateNormals(float* pVertices)
{

	//calculate new normal vectors (according to grid neighbours):
	for ( int zc = 0; zc < NUM_CELLS; zc++)  {
		for (int xc = 0; xc < NUM_CELLS; xc++) {

			const int index = xc+zc*NUM_CELLS;

			float xIndex = GRIDSTART_X - CELL_EDGE*float(xc);
			float zIndex = GRIDSTART_Z - CELL_EDGE*float(zc);

			Vector3 u,v,p1,p2;	//u and v are direction vectors. p1 and p2: temporary used (storing the points)

			if ((xc > 0) &&  (m_pGrids[index].waterHeight > 0.0f)) {

				float x = GRIDSTART_X - CELL_EDGE*float(xc-1);
				float z = GRIDSTART_Z - CELL_EDGE*float(zc);
				p1 = Vector3(x, m_pGrids[index-1].y, z);

			} else
				p1 = Vector3(xIndex, m_pGrids[index].y, zIndex);	
		
		
			if ((xc < NUM_CELLS-1) &&  (m_pGrids[index].waterHeight > 0.0f)) {

				float x = GRIDSTART_X - CELL_EDGE*float(xc+1);
				float z = GRIDSTART_Z - CELL_EDGE*float(zc);
				p2 = Vector3(x, m_pGrids[index+1].y, z);

			} else 
				p2 = Vector3(xIndex, m_pGrids[index].y, zIndex);

			u = p2.sub(p1); //vector from left neighbor to right neighbor

			
			if ((zc > 0) &&  (m_pGrids[index].waterHeight > 0.0f)) {

				float x = GRIDSTART_X - CELL_EDGE*float(xc);
				float z = GRIDSTART_Z - CELL_EDGE*float(zc-1);
				p1 = Vector3(x, m_pGrids[index-NUM_CELLS].y, z);

			} else 
				p1 = Vector3(xIndex, m_pGrids[index].y, zIndex);	

			if ((zc < NUM_CELLS-1) && (m_pGrids[index].waterHeight > 0.0f)) {

				float x = GRIDSTART_X - CELL_EDGE*float(xc);
				float z = GRIDSTART_Z - CELL_EDGE*float(zc+1);
				p2 = Vector3(x, m_pGrids[index+NUM_CELLS].y, z);
			}
			else 
				p2 = Vector3(xIndex, m_pGrids[index].y, zIndex);	
				
			
			v = p2.sub(p1); //vector from upper neighbor to lower neighbor
				
			//calculate the normal:
			Vector3 normal;
			normal.crossProduct(v,u);
			normal.normalize();

			int offset = 6*index;

			*(pVertices + offset) = xIndex;
			*(pVertices + offset+1) = m_pGrids[index].y; 
			*(pVertices + offset+2) = zIndex; 

			*(pVertices + offset+3) = normal[0];
			*(pVertices + offset+4) = normal[1]; 
			*(pVertices + offset+5) = normal[2];

		}
	}
}

void WaterSimulation::fillFFTVertexBuffer(float* pVertices)
{

	for(int xc=0; xc<NUM_CELLS; xc++) {
		int zc = 0;
		const int index = xc;
		const int gridIndex =  xc + zc*NUM_CELLS;
		int offset = 3*index;

		*(pVertices + offset) = GRIDSTART_X - CELL_EDGE*float(xc);
		*(pVertices + offset+1) = m_pGrids[gridIndex].y;
		*(pVertices + offset+2) = GRIDSTART_Z - CELL_EDGE*float(zc);
	}

	for(int zc=0; zc<NUM_CELLS; zc++) {
		int xc=NUM_CELLS-1;
		const int index = NUM_CELLS+zc;
		const int gridIndex =  xc + zc*NUM_CELLS;
		int offset = 3*index;

		*(pVertices + offset) = GRIDSTART_X - CELL_EDGE*float(xc);
		*(pVertices + offset+1) = m_pGrids[gridIndex].y;
		*(pVertices + offset+2) = GRIDSTART_Z - CELL_EDGE*float(zc);
	}

	for(int xc=NUM_CELLS-1; xc>=0; xc--) {
		int zc = NUM_CELLS-1;
		const int index = 2*NUM_CELLS + xc;
		const int gridIndex =  xc + zc*NUM_CELLS;
		int offset = 3*index;

		*(pVertices + offset) = GRIDSTART_X - CELL_EDGE*float(xc);
		*(pVertices + offset+1) = m_pGrids[gridIndex].y;
		*(pVertices + offset+2) = GRIDSTART_Z - CELL_EDGE*float(zc);
	}

	for(int zc=NUM_CELLS-1; zc>=0; zc--) {
		int xc = 0;
		const int index = 3*NUM_CELLS + zc;
		const int gridIndex =  xc + zc*NUM_CELLS;
		int offset = 3*index;

		*(pVertices + offset) = GRIDSTART_X - CELL_EDGE*float(xc);
		*(pVertices + offset+1) = m_pGrids[gridIndex].y;
		*(pVertices + offset+2) = GRIDSTART_Z - CELL_EDGE*float(zc);
	}

	const int index = 4*NUM_CELLS;
	int offset = 3*index;

	// 4 far corner points
	*(pVertices + offset) = GRIDSTART_X - 600.0f;
	*(pVertices + offset+1) = TOTAL_HEIGHT;
	*(pVertices + offset+2) = GRIDSTART_Z + 250.0f;

	*(pVertices + offset+3) = GRIDSTART_X - 600.0f;
	*(pVertices + offset+4) = TOTAL_HEIGHT;
	*(pVertices + offset+5) = GRIDSTART_Z - 600.0f;

	*(pVertices + offset+6) = GRIDSTART_X + 250.0f;
	*(pVertices + offset+7) = TOTAL_HEIGHT;
	*(pVertices + offset+8) = GRIDSTART_Z - 600.0f;

	*(pVertices + offset+9) = GRIDSTART_X + 250.0f;
	*(pVertices + offset+10) = TOTAL_HEIGHT;
	*(pVertices + offset+11) = GRIDSTART_Z + 250.0f;

}

void WaterSimulation::fillIndicesSWE(std::vector<GLuint>& indexVect)
{

	for (int zc = 0; zc < NUM_CELLS ; zc++) {
		for (int xc = 0; xc < NUM_CELLS ; xc++) {
			const int index = xc + zc*NUM_CELLS ;

			//create two triangles:
			if ((xc < NUM_CELLS-1) && (zc < NUM_CELLS-1)) {
			
				indexVect.push_back(index);
				indexVect.push_back(index+1);
				indexVect.push_back(index+1+NUM_CELLS);

				indexVect.push_back(index);
				indexVect.push_back(index+1+NUM_CELLS);
				indexVect.push_back(index+NUM_CELLS);

			}	
		}
	}


}

void WaterSimulation::fillIndicesFFT(std::vector<GLuint>& indexVect)
{
	const int indexCorners = 4*NUM_CELLS;

	for(int xc=0; xc<NUM_CELLS; xc++) {
		int zc = 0;
		const int index = xc;

		if(xc<NUM_CELLS-1) {
			indexVect.push_back(index);
			indexVect.push_back(index+1);
			indexVect.push_back(indexCorners);
		} else {
			indexVect.push_back(index);
			indexVect.push_back(indexCorners);
			indexVect.push_back(indexCorners+1);
		}
	}

	for(int zc=0; zc<NUM_CELLS; zc++) {
		int xc = NUM_CELLS-1;
		const int index = NUM_CELLS+zc;

		if(zc<NUM_CELLS-1) {
			indexVect.push_back(index);
			indexVect.push_back(index+1);
			indexVect.push_back(indexCorners+1);
		} else {
			indexVect.push_back(index);
			indexVect.push_back(indexCorners+1);
			indexVect.push_back(indexCorners+2);
		}
	}

	for(int xc=NUM_CELLS-1; xc>=0; xc--) {
		int zc = NUM_CELLS-1;
		const int index = 2*NUM_CELLS + xc;

		if(xc>0) {
			indexVect.push_back(index);
			indexVect.push_back(index-1);
			indexVect.push_back(indexCorners+2);
		} else {
			indexVect.push_back(index);
			indexVect.push_back(indexCorners+2);
			indexVect.push_back(indexCorners+3);
		}
	}

	for(int zc=NUM_CELLS-1; zc>=0; zc--) {
		int xc = NUM_CELLS-1;
		const int index = 3*NUM_CELLS + zc;

		if(zc>0) {
			indexVect.push_back(index);
			indexVect.push_back(index-1);
			indexVect.push_back(indexCorners+3);
		} else {
			indexVect.push_back(index);
			indexVect.push_back(indexCorners+3);
			indexVect.push_back(indexCorners);
		}
	}
}

void WaterSimulation::addDrop(float objPosX, float objPosZ)
{

	int i = (GRIDSTART_X + m_xTranslate - objPosX)/CELL_EDGE;
	int j = (GRIDSTART_Z + m_zTranslate - objPosZ)/CELL_EDGE;

	if((i>2) && (i<NUM_CELLS-2) && (j>2) && (j<NUM_CELLS-2)) {
		const int index = i + j*NUM_CELLS;

		m_pGrids[index].waterHeight -= .5f;
	}
}

float WaterSimulation::getWaterHeight( float x,  float z)
{
	float height;

	// average height
	int i = (GRIDSTART_X - x + m_xTranslate)/CELL_EDGE;
	int j = (GRIDSTART_Z - z + m_zTranslate)/CELL_EDGE;

	if((i>=0) && (i<NUM_CELLS) && (j>=0) && (j<NUM_CELLS)) {

		int imax, imin; 
		imax = imin = i;

		while(m_pGrids[imin + j*NUM_CELLS].state == Object) {
			imin--;
			if(imin < 0) {
				imin++;
				break;
			}
		}

		while(m_pGrids[imax + j*NUM_CELLS].state == Object) {
			imax++;
			if(imax > NUM_CELLS-1) {
				imax--;
				break;
			}
		}

		int jmax, jmin;
		jmax = jmin = j;

		while(m_pGrids[i + jmin*NUM_CELLS].state == Object) {
			jmin--;
			if(jmin < 0) {
				jmin++;
				break;
			}
		}

		while(m_pGrids[i + jmax*NUM_CELLS].state == Object) {
			jmax++;
			if(jmax > NUM_CELLS-1) {
				jmax--;
				break;
			}
		}

		const float h_imin = m_pGrids[imin + j*NUM_CELLS].y;
		const float h_imax = m_pGrids[imax + j*NUM_CELLS].y;
		const float h_jmin = m_pGrids[i + jmin*NUM_CELLS].y;
		const float h_jmax = m_pGrids[i + jmax*NUM_CELLS].y;

		const float x_imin = GRIDSTART_X - CELL_EDGE*float(imin);
		const float x_imax = GRIDSTART_X - CELL_EDGE*float(imax);
		const float z_jmin = GRIDSTART_Z - CELL_EDGE*float(jmin);
		const float z_jmax = GRIDSTART_Z - CELL_EDGE*float(jmax);

	
		if((x_imin - x_imax == 0) && (z_jmax - z_jmin == 0)) {
			height = TOTAL_HEIGHT;
		} else {
			height = (h_imin*(x-x_imax) + h_imax*(x_imin-x) + h_jmin*(z_jmax - z) + h_jmax*(z - z_jmin))/ ((x_imin - x_imax) + (z_jmax - z_jmin));
		}

	} else {
		height = TOTAL_HEIGHT;
	}

	height = TOTAL_HEIGHT;

	return height;
}


float WaterSimulation::getGroundHeight( float x,  float z) {

	float groundHeight = m_pPortScene->getGroundHeight(x,z);

	if(groundHeight < .0f) {
		groundHeight = FLAT;
	}

	return groundHeight;
}

void WaterSimulation::bodyInteraction()
{
	int convexMinX, convexMaxX, convexMinZ, convexMaxZ;

	if(m_convexHullSize > 0) {
		findObjectCellsOnGrid(convexMinX, convexMaxX, convexMinZ, convexMaxZ);
		findObjectBoundaryOnGrid(convexMinX, convexMaxX, convexMinZ, convexMaxZ);
	}

}

void WaterSimulation::findObjectCellsOnGrid(int &minX, int &maxX, int &minZ, int &maxZ)
{
	float ffx[25], ffz[25], FDX[25], FDZ[25]; 

	for(int i=0; i<m_convexHullSize; i++) {

		ffx[i] = (GRIDSTART_X - m_convexHull[i].v[0] + m_xTranslate)/CELL_EDGE;
		ffz[i] = (GRIDSTART_Z - m_convexHull[i].v[2] + m_zTranslate)/CELL_EDGE;;

		if(i!=0) {

			minX = std::min(minX, (int)ffx[i]);
			maxX = std::max(maxX, (int)ffx[i]);
			minZ = std::min(minZ, (int)ffz[i]);
			maxZ = std::max(maxZ, (int)ffz[i]);

			FDX[i-1] = ffx[i-1] - ffx[i];
			FDZ[i-1] = ffz[i-1] - ffz[i];

		} else {

			minX = ffx[i];
			maxX = ffx[i];
			minZ = ffz[i];
			maxZ = ffz[i];
		}

		if(i == m_convexHullSize-1) {
			FDX[i] = ffx[i] - ffx[0];
			FDZ[i] = ffz[i] - ffz[0];
		}
	}

	float  C[25], CZ[25], CX[25];

	for(int i=0; i<m_convexHullSize; i++) {
		
		C[i] = FDZ[i]*ffx[i] - FDX[i]*ffz[i]; 
		CZ[i] = C[i] + FDX[i]*minZ - FDZ[i]*minX;
		CX[i] = CZ[i];

	}

	float xVelocity, zVelocity;
	xVelocity = zVelocity =.0f;

	m_newNumObjectCellIndices = 0;

	for(int z=minZ; z<=maxZ; z++) {

		for(int i=0; i<m_convexHullSize; i++) {
				CX[i] = CZ[i];
		}

		for(int x=minX; x<=maxX; x++) {

			bool Passed = 1;

			for(int i=0; i<m_convexHullSize; i++) {
			
				if(!(CX[i]>-0.5f)) {
					Passed = 0;
					break;
				}

			}

			if(Passed) {
				
				const int index = x + z*NUM_CELLS;

				if((index < NUM_GRIDS-1) && (index>=0)) {

					if(m_pGrids[index].state == Water) {

						m_pGrids[index].state = Object;

						if (m_newNumObjectCellIndices < 2000) {
							m_newObjectCellIndices[m_newNumObjectCellIndices] = index;
							m_newNumObjectCellIndices++;
						}

						xVelocity += m_pGrids[index].xVelocity;
						zVelocity += m_pGrids[index].zVelocity;

					}
				}

			}

			for(int i=0; i<m_convexHullSize; i++) {
				CX[i] -= FDZ[i];
			}
		}

		for(int i=0; i<m_convexHullSize; i++) {
				CZ[i] += FDX[i];
		}
	}

	if(m_newNumObjectCellIndices>0) {
		m_xVelocity = xVelocity/m_newNumObjectCellIndices;
		m_zVelocity = zVelocity/m_newNumObjectCellIndices;
	} else {
		m_xVelocity = .0f;
		m_zVelocity = .0f;
	}
	
}

void WaterSimulation::findObjectBoundaryOnGrid(int minX,  int maxX, int minZ, int maxZ)
{

	int imin = (int)minX;
	int imax = (int)maxX;
	int jmin = (int)minZ;
	int jmax = (int)maxZ;

	if(imin <= 1) {
		imin = 1; 
	} else if(imin >= NUM_CELLS-2) {
		imin = NUM_CELLS-2;
	}

	if(imax <= 1) {
		imax = 1; 
	} else if (imax >= NUM_CELLS-2) {
		imax = NUM_CELLS-2;
	}

	if(jmin <= 1) {
		jmin = 1;
	} else if(jmin >= NUM_CELLS-2) {
		jmin = NUM_CELLS-2;
	}
		
	if(jmax <= 1) {
		jmax = 1;
	} else if (jmax >= NUM_CELLS-2) {
		jmax = NUM_CELLS-2;
	}

	int numCells = 0;

	int objectBoundaryIndices[500];

	for(int j=jmin; j<jmax; j++) {
		for(int i=imin; i<imax; i++) {

			const int index = i + j*NUM_CELLS;

			/*if((m_pGrids[index].state!=Ground) && (m_pGrids[index].state!=Object)) {*/
			if((m_pGrids[index].state == Water)) {

				if((m_pGrids[index+1].state == Object) || (m_pGrids[index-1].state == Object) || (m_pGrids[index+NUM_CELLS].state == Object) || (m_pGrids[index-NUM_CELLS].state == Object)
					|| (m_pGrids[index+1+NUM_CELLS].state == Object) || (m_pGrids[index+1-NUM_CELLS].state == Object)
					|| (m_pGrids[index-1+NUM_CELLS].state == Object) || (m_pGrids[index-1-NUM_CELLS].state == Object)) {

						m_pGrids[index].state = ObjectBoundary;
						if (numCells < 500) {
							objectBoundaryIndices[numCells] = index;
							numCells++;
						}
				}
			}
		}
	}

	const float height = TOTAL_HEIGHT - FLAT - DISPLACED_HEIGHT;
	const float displaceHeight = DISPLACED_HEIGHT/4.0f;

	if((m_boatSpeed > 0.001f) || (m_boatSpeed < -0.001f) || (m_rotation > 0.001f) || (m_rotation < -0.001f)) {

		for (int i=0; i<m_newNumObjectCellIndices; i++) {

			const int index = m_newObjectCellIndices[i];
			const int x = m_newObjectCellIndices[i]%NUM_CELLS;
			const int z = m_newObjectCellIndices[i]/NUM_CELLS;
	
			bool loop = true;
			int offset = 1;

			if(m_pGrids[index].waterHeight > height) {

				while(loop) {

					if((x + offset) <= (NUM_CELLS-1)) {
						if(m_pGrids[index+offset].state == ObjectBoundary) {
							m_pGrids[index].waterHeight -= displaceHeight;
							m_pGrids[index+offset].waterHeight += displaceHeight;
							loop = false;
							break;
						}
					}
			
					if((x - offset) >= 1) {
						if(m_pGrids[index-offset].state == ObjectBoundary) {
							m_pGrids[index].waterHeight -= displaceHeight;
							m_pGrids[index-offset].waterHeight += displaceHeight;
							loop = false;
							break;
						}
					} 
			
					if((z + offset) <= (NUM_CELLS-1)) {
						if(m_pGrids[index + offset*NUM_CELLS].state == ObjectBoundary) {
							m_pGrids[index].waterHeight -= displaceHeight;
							m_pGrids[index + offset*NUM_CELLS].waterHeight += displaceHeight;
							loop = false;
							break;
						}
					} 
				
					if((z - offset) >= 1) {
						if(m_pGrids[index - offset*NUM_CELLS].state == ObjectBoundary) {
							m_pGrids[index].waterHeight -= displaceHeight;
							m_pGrids[index - offset*NUM_CELLS].waterHeight += displaceHeight;
							loop = false;
							break;
						}
					} 
			
					offset++;

					if(offset>5)
						break;
			
				}
			}

		}
	}
}





