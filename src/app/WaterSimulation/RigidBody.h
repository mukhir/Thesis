/** \class RigidBody
 * Defines rigid body to interacting with shallow water
 *
 * @author  Rahul Mukhi
 * @date 04/05/12
 *
 * Copyright (C) GIANTS Software GmbH, Confidential, All Rights Reserved.
 */

#include "ObjReader.h"
#include "WaterSimulation.h"
#include "glut/glut.h"
#include "iostream"
#include "Camera.h"
#include <stdlib.h>
#include <queue>

class RigidBody
{
public:
	RigidBody(WaterSimulation& waterSimulation, Camera& camera);
	~RigidBody();

	struct triangle
	{
		int v0Index;
		int v1Index;
		int v2Index;
	};

	struct convexHull
	{
		Vector3 point;
		float cos, distancefromP0;
	};

	struct angleComparison 
	{
		bool operator()(const convexHull& c1, const convexHull& c2)
		{
			return (c1.cos > c2.cos);
		}
	}sortAngle;

	void initialize();
	void renderObject();
	void rigidBodyInteraction();
	void setPosition();
	void calculateBuoyantForce();
	void calculateConvexHull();
	void pressNormalKey(unsigned char& key);
	void releaseNormalKey(unsigned char& key);

private: 

	static const float MASS, LINEAR_CONSTANT, SCALE, WATER_DENSITY, PI_BY_180;
	float m_bottom, m_top;
	float m_yVelocity, m_speed;
	float m_changeRotAngle, m_rotationAngle;
	Vector3 m_translate;

	float calculateVolumeSubmerged();
	float tetrahedronVolume(const Vector3& v0, const Vector3& v1, const Vector3& v2, float waterHeight);
	float clipTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, const float& d0, const float& d1, const float& d2, float waterHeight);
	void passConvexHulltoSimulation();

	inline float isLeft(const Vector3& P0, const Vector3& P1, const Vector3& P2)
	{
		return ((P1.v[0] - P0.v[0])*(P2.v[2] - P0.v[2]) - (P2.v[0] - P0.v[0])*(P1.v[2] - P0.v[2]));
	}

	inline Vector3 transform(const Vector3& v)
	{
		float angle = m_rotationAngle*PI_BY_180;
		return Vector3(v[0]*cos(-angle) - v[2]*sin(-angle) + m_translate[0], v[1] + m_translate[1], v[0]*sin(-angle) + v[2]*cos(-angle) + m_translate[2]);
	}

	std::vector<triangle> m_triangles;
	std::vector<Vector3> m_waterPlaneIntersection;
	std::vector<Vector3> m_convexHull;

	ObjReader m_rigidBody;
	Camera *m_pCamera;

	WaterSimulation *m_pWaterSimulation;
};

