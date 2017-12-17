/** \class Plane
 * A plane is defined in 3D space by the equation
 * Ax + By + Cz + D = 0
 *
 * @author  Christian Ammann
 * @date  18/08/03
 *
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#pragma once

#include "base/math/Vector3.h"
#include "base/math/Matrix4x4.h"

class Plane
{
public:
    Vector3 m_normal;
    float m_distance;

    Plane();
    Plane(float a, float b, float c, float d);
    Plane(const Vector3& origin, const Vector3& normal);
    Plane(const Vector3& p1, const Vector3& p2, const Vector3& p3);

    void set(const Vector3& origin, const Vector3& normal);
    void normalize();
    void transform(const Matrix4x4& tansformation);
    bool isFrontFacingTo(const Vector3& direction) const;
    float signedDistanceTo(const Vector3& point) const;
    bool pointInHalfSpace(const Vector3& point) const;
    bool sphereInHalfSpace(const Vector3& center, float radius) const;
    bool intersectionWithLine(const Vector3& lineOrigin, const Vector3& lineDir, Vector3& intersection) const;
    bool intersectionWithLine(const Vector3& lineOrigin, const Vector3& lineDir, float& t) const;
    bool intersectionWithPlane(const Plane& plane, Vector3& point, Vector3& dir) const;
};

