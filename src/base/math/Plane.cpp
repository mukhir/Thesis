/*
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#include "PreCompiled.h"

#include "Plane.h"

#include "base/math/MathUtil.h"

/**
 * Construct a new Plane object
 */
Plane::Plane() : m_normal(0.0f,1.0f,0.0f), m_distance(0.0f)
{
}

/**
 * Construct a new Plane object
 *
 * @param at  first value of plane equation
 * @param bt  second value of plane equation
 * @param ct  third value of plane equation
 * @param dt  fourth value of plane equation
 */
Plane::Plane(float at, float bt, float ct, float dt) :
    m_normal(at, bt, ct),
    m_distance(dt)
{
}

/**
 * Construct a new Plane object
 *
 * @param origin  origin vector
 * @param normal  normal normal vector
 */
Plane::Plane(const Vector3& origin, const Vector3& normal)
{
    set(origin, normal);
}

/**
 * Construct new Plane object from a triangle
 *
 * @param p1  first triangle vertex
 * @param p2  second triangle vertex
 * @param p3  third triangle vertex
 */
Plane::Plane(const Vector3& p1, const Vector3& p2, const Vector3& p3){
    Vector3 p2Mp1;
    p2Mp1.sub(p2,p1);
    Vector3 p3Mp1;
    p3Mp1.sub(p3,p1);
    m_normal.crossProduct(p2Mp1, p3Mp1);
    m_normal.normalize();
    m_distance = -(m_normal.dotProduct(p1));
}

/**
 * Set plane properties
 * @param origin  origin vector
 * @param normal  normal normal vector
 */
void Plane::set(const Vector3& origin, const Vector3& normal)
{
    m_normal = normal;
    m_distance = -(normal.dotProduct(origin));
}

/**
 * Normalize plane equation
 */
void Plane::normalize()
{
    float t = MathUtil::invSqrt(m_normal.dotProduct(m_normal));
    m_normal.scale(t);
    m_distance *= t;
}

void Plane::transform(const Matrix4x4& transformation)
{
    Vector3 pointOnPlane(-m_distance*m_normal[0], -m_distance*m_normal[1], -m_distance*m_normal[2]);
    Vector3 newNormal;
    transformation.rotateVector(m_normal, newNormal);
    newNormal.normalize();
    m_normal = newNormal;
    transformation.transformVector(pointOnPlane, newNormal); 
    m_distance = -(m_normal.dotProduct(newNormal));
}

/**
 * Is plane front facing to a direction
 *
 * @param direction  direction vector
 * @return returns true if plane is front faceing otherwise false.
 */
bool Plane::isFrontFacingTo(const Vector3& direction) const
{
    return (m_normal.dotProduct(direction) <= 0);
}

/**
 * Signed distance to point
 *
 * @param point  point to mesaure distance to
 * @return returns distance to plane
 */
float Plane::signedDistanceTo(const Vector3& point) const
{
    return (point.dotProduct(m_normal)) + m_distance;
}

/**
 * Is point in half space defined by this plane
 *
 * @param point  point
 * @return returns true if point lies in halfspace otherwise false
 */
bool Plane::pointInHalfSpace(const Vector3& point) const
{
    return (m_normal.dotProduct(point) + m_distance) <= 0;
}

/**
 * Is sphere in half space defined by this plane
 *
 * @param center  center of sphere
 * @param radius  radius of sphere
 * @return returns true if sphere lies in halfspace otherwise false
 */
bool Plane::sphereInHalfSpace(const Vector3& center, float radius) const
{
    return (m_normal.dotProduct(center) + m_distance) <= -radius;
}

/**
 * Calculates intersection with a line
 *
 * @param lineOrigin  origin of line
 * @param lineDir  direction of line
 * @param intersection  possible intersection point
 * @return returns true if line intersects with plane otherwise false
 */
bool Plane::intersectionWithLine(const Vector3& lineOrigin, const Vector3& lineDir, Vector3& intersection) const
{
    float t;
    if (intersectionWithLine(lineOrigin, lineDir, t)) {
        intersection = lineOrigin;
        Vector3 tmp = lineDir;
        tmp.scale(t);
        intersection.add(tmp);
        return true;
    }
    return false;
}

/**
 * Calculates intersection with a line
 *
 * @param lineOrigin  origin of line
 * @param lineDir  direction of line
 * @param t  possible intersection scale for lineDir to reach intersection point
 * @return returns true if line intersects with plane otherwise false
 */
bool Plane::intersectionWithLine(const Vector3& lineOrigin, const Vector3& lineDir, float& t) const
{
    const float epsilon = 1e-4f; 

    // Check for (near) parallel line and plane
    float denominator = lineDir.dotProduct(m_normal);
    if(fabsf(denominator) < epsilon){
        return false;
/*
        // Check if line lies in the plane or not.
        if(fabsf(lineOrigin.dotProduct(normal) + distance) < epsilon) {
        } else {
            t=0;
            return true;
        } else {
            return false;
        }
*/
    }

    // Nonparallel, so compute intersection
    t = -(lineOrigin.dotProduct(m_normal) + m_distance) / denominator;
    return true;
}

/**
 * Calculates intersection with a plane
 *
 * @param plane  plane
 * @param point  intersection line origin point
 * @param dir  interesction line direction
 * @return returns true if plane intersects with plane otherwise false
 */
bool Plane::intersectionWithPlane(const Plane& plane, Vector3& point, Vector3& dir) const
{
    const float epsilon = 1e-6f;
    // determine intersection line direction
    dir.crossProduct(m_normal, plane.m_normal);
    float dnorm = dir.calcMagnitude();

    // if planes are parallel then fail
    if (dnorm < epsilon) {
        point.zeroise();
        dir.zeroise();
        return false;
    }

    // determine intersection point with the best suited coordinate plane

    float tmpAbs;
    float maxabs = fabsf(dir.v[0]);
    unsigned int index = 0;

    if ((tmpAbs = fabsf(dir.v[1])) > maxabs) { maxabs = tmpAbs ; index = 1; }
    if ((tmpAbs = fabsf(dir.v[2])) > maxabs) { maxabs = tmpAbs ; index = 2; }

    switch (index) {
        case 0:
            point.set(
                0.0f,
                (m_normal.v[2]*plane.m_distance - plane.m_normal.v[2]*m_distance) / dir.v[0],
                (plane.m_normal.v[1]*m_distance - m_normal.v[1]*plane.m_distance) / dir.v[0]);
            break;
        case 1:
            point.set(
                (plane.m_normal.v[2]*m_distance - m_normal.v[2]*plane.m_distance) / dir.v[1],
                0.0f,
                (m_normal.v[0]*plane.m_distance - plane.m_normal.v[0]*m_distance) / dir.v[1]);
            break;
        case 2:
            point.set(
                (m_normal.v[1]*plane.m_distance - plane.m_normal.v[1]*m_distance) / dir.v[2],
                (plane.m_normal.v[0]*m_distance - m_normal.v[0]*plane.m_distance) / dir.v[2],
                0.0f);
            break;
        default: return false;
    }
    dir.normalize();
    return true;
}
