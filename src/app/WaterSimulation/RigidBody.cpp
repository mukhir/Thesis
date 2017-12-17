/*
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 */

#include "RigidBody.h"

const float RigidBody::MASS = 1.5f;
const float RigidBody::LINEAR_CONSTANT = 2.0f;
const float RigidBody::SCALE = 0.05f;
const float RigidBody::WATER_DENSITY = 1.0f;
const float RigidBody::PI_BY_180 = 3.14159265f/180.0f;

RigidBody::RigidBody(WaterSimulation& waterSimulation, Camera& camera)
{
	m_pWaterSimulation = &waterSimulation;
	m_pCamera = &camera;
	initialize();
}

RigidBody::~RigidBody()
{
}

void RigidBody::initialize()
{
	std::string filename ("Data/boatHull01.obj");
	m_rigidBody.objectLoader(filename);

	float minY, maxY;

	m_rigidBody.m_vertices[0].v[0]*= SCALE;
	m_rigidBody.m_vertices[0].v[1]*= SCALE;
	m_rigidBody.m_vertices[0].v[2]*= SCALE;

	minY = maxY = m_rigidBody.m_vertices[0].v[1];

	for(int i=1; i<m_rigidBody.m_vertices.size(); i++) {

		m_rigidBody.m_vertices[i].v[0]*= SCALE;
		m_rigidBody.m_vertices[i].v[1]*= SCALE;
		m_rigidBody.m_vertices[i].v[2]*= SCALE;

		minY = std::min(minY, m_rigidBody.m_vertices[i].v[1]);
		maxY = std::max(maxY, m_rigidBody.m_vertices[i].v[1]);

	}
	
	m_bottom = minY;
	m_top = maxY;

	m_changeRotAngle = 0.0f;
	m_rotationAngle = 180.0f;

	m_translate[0] = m_translate[2] = 0.0f;
	m_translate[1] = m_pWaterSimulation->TOTAL_HEIGHT - 2.0f;
	m_speed = m_yVelocity = 0.0f;

	// create triangular mesh for object if not triangulated
	for (int i = 0; i < m_rigidBody.m_faces.size(); i++) {
		if(m_rigidBody.m_faces[i].numVertices>3) {
			for(int j=1; j<m_rigidBody.m_faces[i].numVertices-1; j++) 
			{
				triangle tri;
				tri.v0Index = m_rigidBody.m_faces[i].vertex[0];
				tri.v1Index = m_rigidBody.m_faces[i].vertex[j];
				tri.v2Index = m_rigidBody.m_faces[i].vertex[j+1];
				m_triangles.push_back(tri);
			}

		} else {

			triangle tri;
			tri.v0Index = m_rigidBody.m_faces[i].vertex[0];
			tri.v1Index = m_rigidBody.m_faces[i].vertex[1];
			tri.v2Index = m_rigidBody.m_faces[i].vertex[2];

			m_triangles.push_back(tri);
		}
	}

	m_triangles.size();
}

void RigidBody::rigidBodyInteraction()
{
	setPosition();
	calculateBuoyantForce();
	calculateConvexHull();
	passConvexHulltoSimulation();
}

void RigidBody::setPosition()
{
	m_rotationAngle += m_changeRotAngle;
	m_translate[0] -= m_speed*sin(m_rotationAngle*PI_BY_180);
	m_translate[2] -= m_speed*cos(m_rotationAngle*PI_BY_180);

	m_translate[0] += 0.05f*m_pWaterSimulation->m_xVelocity;
	m_translate[2] += 0.05f*m_pWaterSimulation->m_zVelocity;

	m_pCamera->moveCameraWithBoat(m_translate);
}

void RigidBody::calculateBuoyantForce()
{
	float hDiff;

	float gravity = m_pWaterSimulation->GRAVITY;
	const float waterHeight = m_pWaterSimulation->TOTAL_HEIGHT;
	float dt = m_pWaterSimulation->TIME_STEP;

	float volumeSubmerged = calculateVolumeSubmerged();

	if(waterHeight > m_top)
		hDiff = m_top - m_bottom;
	else if (waterHeight<m_bottom)
		hDiff = 0.0f;
	else
		hDiff = waterHeight - m_bottom;

	float force = MASS*gravity - WATER_DENSITY*volumeSubmerged*gravity/1000.0f;

	m_yVelocity += force*dt/MASS;
	m_yVelocity *= 0.95f; // damping
	m_translate.v[1] += dt*m_yVelocity;

	m_bottom += dt*m_yVelocity;
	m_top += dt*m_yVelocity;

	//std::cout<<m_translate.v[1] <<std::endl;
}

float RigidBody::calculateVolumeSubmerged()
{ 
	float volume = 0.0f;
	float d0, d1, d2;

	const float waterHeight = m_pWaterSimulation->TOTAL_HEIGHT;
	 
	// points on boat intersecting water surface
	m_waterPlaneIntersection.clear();

	for (int i = 0; i < m_triangles.size(); i++) {

		// depth of each vertex of triangle
		float d0 = m_rigidBody.m_vertices[m_triangles[i].v0Index].v[1] + m_translate[1] - waterHeight;
		float d1 = m_rigidBody.m_vertices[m_triangles[i].v1Index].v[1] + m_translate[1] - waterHeight;
		float d2 = m_rigidBody.m_vertices[m_triangles[i].v2Index].v[1] + m_translate[1] - waterHeight;
			
		if((d0<0.0f) && (d1<0.0f) && (d2<0.0f)) { // if all three vertices inside water calculate volume of tatrahedron
			float x = (m_rigidBody.m_vertices[m_triangles[i].v0Index].v[0] + m_rigidBody.m_vertices[m_triangles[i].v1Index].v[0] + m_rigidBody.m_vertices[m_triangles[i].v2Index].v[0])/3.0f;
			float z = (m_rigidBody.m_vertices[m_triangles[i].v0Index].v[2] + m_rigidBody.m_vertices[m_triangles[i].v1Index].v[2] + m_rigidBody.m_vertices[m_triangles[i].v2Index].v[2])/3.0f;
			volume += tetrahedronVolume(m_rigidBody.m_vertices[m_triangles[i].v0Index], m_rigidBody.m_vertices[m_triangles[i].v1Index], m_rigidBody.m_vertices[m_triangles[i].v2Index], m_pWaterSimulation->getWaterHeight(x,z));
		}
		else if((d0>0.0f) && (d1>0.0f) && (d2>0.0f)) { // clip triangle and calculate tetrahedron volume for clipped triangle
		} else {
			float x = (m_rigidBody.m_vertices[m_triangles[i].v0Index].v[0] + m_rigidBody.m_vertices[m_triangles[i].v1Index].v[0] + m_rigidBody.m_vertices[m_triangles[i].v2Index].v[0])/3.0f;
			float z = (m_rigidBody.m_vertices[m_triangles[i].v0Index].v[2] + m_rigidBody.m_vertices[m_triangles[i].v1Index].v[2] + m_rigidBody.m_vertices[m_triangles[i].v2Index].v[2])/3.0f;
			volume += clipTriangle(m_rigidBody.m_vertices[m_triangles[i].v0Index], m_rigidBody.m_vertices[m_triangles[i].v1Index], m_rigidBody.m_vertices[m_triangles[i].v2Index], d0,d1,d2, m_pWaterSimulation->getWaterHeight(x,z));
		}
	}

	return volume;
}

float RigidBody::tetrahedronVolume(const Vector3& v0, const Vector3& v1, const Vector3& v2, float waterHeight)
{
	Vector3 a, b, r;
	float tetrahedronVolume;
	a.sub(v0, v1);
	b.sub(v2, v1);
	r.sub(Vector3(0.0f, waterHeight - m_translate[1], 0.0f), v1);

	Vector3 cross;
	cross.crossProduct(b,a);
	tetrahedronVolume = (1.0f/6.0f)*Vector3::dotProduct(cross, r);
	return -tetrahedronVolume;
}

float RigidBody::clipTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, const float& d0, const float& d1, const float& d2, float waterHeight)
{ 
	float volume = .0f;
	Vector3 clipped1, clipped2, v;

	if((d0<0.0f) && (d1>0.0f) && (d2>0.0f)) {

		clipped1.add(v0, v.sub(v2, v0).scale(d0/(d0-d2))); // clipped points
		clipped2.add(v0, v.sub(v1, v0).scale(d0/(d0-d1)));
		m_waterPlaneIntersection.push_back(clipped1);
		m_waterPlaneIntersection.push_back(clipped2);
		volume += tetrahedronVolume(v0, clipped1, clipped2, waterHeight);

	} else if((d0>0.0f) && (d1<0.0f) && (d2>0.0f)) {

		clipped1.add(v1, v.sub(v2, v1).scale(d1/(d1-d2)));
		clipped2.add(v1, v.sub(v0, v1).scale(d1/(d1-d0)));
		m_waterPlaneIntersection.push_back(clipped1);
		m_waterPlaneIntersection.push_back(clipped2);
		volume += tetrahedronVolume(v1, clipped1, clipped2, waterHeight);

	} else if((d0>0.0f) && (d1>0.0f) && (d2<0.0f)) {

		clipped1.add(v2, v.sub(v1, v2).scale(d2/(d2-d1)));
		clipped2.add(v2, v.sub(v0, v2).scale(d2/(d2-d0)));
		m_waterPlaneIntersection.push_back(clipped1);
		m_waterPlaneIntersection.push_back(clipped2);
		volume += tetrahedronVolume(v2, clipped1, clipped2, waterHeight);

	} else if((d0<0.0f) && (d1<0.0f) && (d2>0.0f)) {

		clipped1.add(v0, v.sub(v2, v0).scale(d0/(d0-d2)));
		clipped2.add(v1, v.sub(v2, v1).scale(d1/(d1-d2)));
		m_waterPlaneIntersection.push_back(clipped1);
		m_waterPlaneIntersection.push_back(clipped2);
		volume += tetrahedronVolume(v0, clipped1, clipped2, waterHeight);
		volume += tetrahedronVolume(v0, v1, clipped2, waterHeight);

	} else if((d0<0.0f) && (d1>0.0f) && (d2<0.0f)) {

		clipped1.add(v0, v.sub(v1, v0).scale(d0/(d0-d1)));
		clipped2.add(v2, v.sub(v1, v2).scale(d2/(d2-d1)));
		m_waterPlaneIntersection.push_back(clipped1);
		m_waterPlaneIntersection.push_back(clipped2);
		volume += tetrahedronVolume(v0, clipped1, clipped2, waterHeight);
		volume += tetrahedronVolume(v0, v2, clipped2, waterHeight);

	} else if((d0>0.0f) && (d1<0.0f) && (d2<0.0f)){

		clipped1.add(v1, v.sub(v0, v1).scale(d1/(d1-d0)));
		clipped2.add(v2, v.sub(v0, v2).scale(d2/(d2-d0)));
		m_waterPlaneIntersection.push_back(clipped1);
		m_waterPlaneIntersection.push_back(clipped2);
		volume += tetrahedronVolume(v1, clipped1, clipped2, waterHeight);
		volume += tetrahedronVolume(v1, v2, clipped2, waterHeight);

	} else {
	}

	return volume;
}

void RigidBody::calculateConvexHull() // using Graham Scan
{
	Vector3 P0;
	int index = 0;

	if(m_waterPlaneIntersection.size() > 0) {

		P0 = m_waterPlaneIntersection[0];

		for(int i=1; i<m_waterPlaneIntersection.size(); i++) { // locate point P0 with max x and z coordinates
	
			if(m_waterPlaneIntersection[i].v[2] > P0.v[2]) {

				P0 = m_waterPlaneIntersection[i];
				index = i;
			}

			if(m_waterPlaneIntersection[i].v[2] == P0.v[2]) {
				if(m_waterPlaneIntersection[i].v[0] > P0.v[0]) {

					P0 = m_waterPlaneIntersection[i];
					index = i;
				}
			}
		}

		std::vector<convexHull> sortedPointsInitial;

		for(int i=0; i<m_waterPlaneIntersection.size(); i++) { // sort points from right to left
			
			if(i != index) {

				convexHull cH;
				cH.point = m_waterPlaneIntersection[i];

				float d = (m_waterPlaneIntersection[i].v[0] - P0.v[0])*(m_waterPlaneIntersection[i].v[0] - P0.v[0]) + (m_waterPlaneIntersection[i].v[2] - P0.v[2])*(m_waterPlaneIntersection[i].v[2] - P0.v[2]);

				if(d!=0) {
					cH.cos = (m_waterPlaneIntersection[i].v[0] - P0.v[0])/sqrt(d);
					cH.distancefromP0 = d;
					sortedPointsInitial.push_back(cH);
				}
			}
		}

		std::sort(sortedPointsInitial.begin(), sortedPointsInitial.end(), sortAngle);

		std::vector<Vector3> sortedPoints;
		std::vector<convexHull>::iterator it, temp;

		temp = sortedPointsInitial.begin();

		// remove duplicate points or chose farthest point if angle is same 
		for(it=sortedPointsInitial.begin()+1; it<sortedPointsInitial.end(); it++) {
			if(temp->cos == it->cos) {
				if(it->distancefromP0 > temp->distancefromP0) {
					temp = it;
				} 
			} else {
				temp->cos = it->cos;
				sortedPoints.push_back(it->point);
			}
		}

		m_convexHull.clear();
		m_convexHull.push_back(P0);
		m_convexHull.push_back(sortedPoints[0]);

		std::vector<Vector3>::iterator sP_Iterator, cH_Iterator;
		int offset = 0;
		cH_Iterator = m_convexHull.begin()+offset;

		for(sP_Iterator=sortedPoints.begin()+1; sP_Iterator<sortedPoints.end(); sP_Iterator++) {
			if(isLeft(*cH_Iterator, *(cH_Iterator+1), *sP_Iterator) < 0.0f) { // check if the point lies to the left
				m_convexHull.push_back(*sP_Iterator);
				offset++;
				cH_Iterator = m_convexHull.begin()+offset;
			} else {
				m_convexHull.pop_back();
				m_convexHull.push_back(*sP_Iterator);
			}
		}
	}

}

void RigidBody::renderObject()
{
	glPushMatrix();
	glTranslatef(m_translate[0], m_translate[1], m_translate[2]);
	glRotatef(m_rotationAngle, 0.0f, 1.0f, 0.0f);

	for (int i = 0; i < m_rigidBody.m_faces.size(); i++) {
		if( m_rigidBody.m_faces[i].numVertices == 3) {
			/*if (m_rigidBody.m_faces[i].numVertices == 3) {
				glBegin(GL_TRIANGLES);
				glNormal3f(m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[0]].v[0], m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[0]].v[1], m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[0]].v[2]);
                glVertex3f(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[0]].v[0], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[0]].v[1], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[0]].v[2]);
                glNormal3f(m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[1]].v[0], m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[1]].v[1], m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[1]].v[2]);
                glVertex3f(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[1]].v[0], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[1]].v[1], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[1]].v[2]);
                glNormal3f(m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[2]].v[0], m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[2]].v[1], m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[2]].v[2]);
                glVertex3f(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[2]].v[0], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[2]].v[1], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[2]].v[2]);
				glEnd();
			} else {*/
				Vector3 e1, e2, normal;
				e1.sub(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[1]], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[0]]);
				e2.sub(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[2]], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[0]]);
				normal.crossProduct(e1,e2);
				normal.normalize();
				glBegin(GL_TRIANGLES);
				glNormal3f(normal[0], normal[1], normal[2]);
				glVertex3f(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[0]].v[0], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[0]].v[1], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[0]].v[2]);
				glVertex3f(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[1]].v[0], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[1]].v[1], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[1]].v[2]);
				glVertex3f(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[2]].v[0], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[2]].v[1], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[2]].v[2]);
				glEnd();
			//}
		} else if(m_rigidBody.m_faces[i].numVertices== 4) {
			/*if (m_rigidBody.m_faces[i].numVertices== 4) {
				glBegin(GL_QUADS);
				glNormal3f(m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[0]].v[0], m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[0]].v[1], m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[0]].v[2]);
                glVertex3f(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[0]].v[0], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[0]].v[1], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[0]].v[2]);
                glNormal3f(m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[1]].v[0], m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[1]].v[1], m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[1]].v[2]);
                glVertex3f(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[1]].v[0], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[1]].v[1], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[1]].v[2]);
                glNormal3f(m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[2]].v[0], m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[2]].v[1], m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[2]].v[2]);
                glVertex3f(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[2]].v[0], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[2]].v[1], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[2]].v[2]);
				glNormal3f(m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[3]].v[0], m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[3]].v[1], m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[3]].v[2]);
                glVertex3f(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[3]].v[0], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[3]].v[1], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[3]].v[2]);
				glEnd();
			} else {*/
				Vector3 e1, e2, normal;
				e1.sub(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[1]], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[0]]);
				e2.sub(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[3]], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[0]]);
				normal.crossProduct(e1,e2);
				normal.normalize();
				glBegin(GL_QUADS);
				glNormal3f(normal[0], normal[1], normal[2]);
				glVertex3f(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[0]].v[0], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[0]].v[1], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[0]].v[2]);
				glVertex3f(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[1]].v[0], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[1]].v[1], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[1]].v[2]);
				glVertex3f(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[2]].v[0], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[2]].v[1], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[2]].v[2]);
				glVertex3f(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[3]].v[0], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[3]].v[1], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[3]].v[2]);
				glEnd();
			//}
		} else {
			/*if (m_rigidBody.m_faces[i].normal.size() == m_rigidBody.m_faces[i].vertex.size()) {
				glBegin(GL_POLYGON);
				for(int j=0; j<m_rigidBody.m_faces[i].n; j++)
				{
					glNormal3f(m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[j]].v[0], m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[j]].v[1], m_rigidBody.m_normals[m_rigidBody.m_faces[i].normal[j]].v[2]);
					glVertex3f(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[j]].v[0], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[j]].v[1], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[j]].v[2]);
				}
				glEnd();
			} else {*/
				Vector3 e1, e2, normal;
				e1.sub(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[1]], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[0]]);
				e2.sub(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[m_rigidBody.m_faces[i].numVertices-1]], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[0]]);
				normal.crossProduct(e1,e2);
				normal.normalize();
				glBegin(GL_POLYGON);
				glNormal3f(normal[0], normal[1], normal[2]);
				for(int j=0; j<m_rigidBody.m_faces[i].numVertices; j++)
					glVertex3f(m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[j]].v[0], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[j]].v[1], m_rigidBody.m_vertices[m_rigidBody.m_faces[i].vertex[j]].v[2]);
				glEnd();
			//}*/
		}
	}

	glPopMatrix();

}

void RigidBody::pressNormalKey(unsigned char& key)
{
	if(key == 'w')
		m_speed = -.5f;
	else if(key == 's')
		m_speed = .5f;
	else if(key == 'd')
		m_changeRotAngle -= 1.0f; 
	else if(key == 'a')
		m_changeRotAngle += 1.0f;
}

void RigidBody::releaseNormalKey(unsigned char& key)
{
	if(key == 'w')
		m_speed = .0f;
	else if(key == 's')
		m_speed = .0f;
	else if(key == 'd')
		m_changeRotAngle = .0f; 
	else if(key == 'a')
		m_changeRotAngle = .0f;
}

void RigidBody::passConvexHulltoSimulation()
{
	std::vector<Vector3>::iterator it;
	int i=0;

	for(it = m_convexHull.begin(); it<m_convexHull.end(); it++) {

		m_pWaterSimulation->m_convexHull[i] = transform(*it);
		i++;

	}

	m_pWaterSimulation->m_convexHullSize = i;
	m_pWaterSimulation->m_boatSpeed = m_speed;
	m_pWaterSimulation->m_rotation = m_changeRotAngle;
}
