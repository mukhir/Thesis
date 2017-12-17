/** \class Quaternion
 * Quaternion
 *
 * @author  Christian Ammann
 * @date  08/11/05
 *
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#pragma once

#include "Vector3.h"

class Matrix4x4;

class Quaternion
{
public:

    inline Quaternion()
    {
        x = y = z = w = 0.0f;
    }

    inline Quaternion(const Quaternion &Q)
    {
        x = Q.x;
        y = Q.y;
        z = Q.z;
        w = Q.w;
    }

    inline Quaternion(float Q_w)
    {
        x = 0;
        y = 0;
        z = 0;
        w = Q_w;
    }

    inline Quaternion(float Q_x, float Q_y, float Q_z, float Q_w)
    {
        x = Q_x;
        y = Q_y;
        z = Q_z;
        w = Q_w;
    }

    inline Quaternion& operator=(const Quaternion &Q)
    {
        x = Q.x;
        y = Q.y;
        z = Q.z;
        w = Q.w;
        return *this;
    }

    inline Quaternion(float radians, const Vector3& axis)
    {
        float t = axis.calcMagnitudeSquared();
        if(t != 0.0f){
            t = sinf(radians*0.5f)* MathUtil::invSqrt(t);
        }   

        // Build quaternion
        x = t * axis.v[0];
        y = t * axis.v[1];
        z = t * axis.v[2];
        w = cosf(radians*0.5f);
    }

    inline Quaternion(const Matrix4x4& matrix)
    {
        set(matrix);
    }

    inline Quaternion& mult(const Quaternion& right)
    {
        float temp[3];
        temp[0] = w*right.w - right.x*x - y*right.y - right.z*z;
        temp[1] = w*right.x + right.w*x + y*right.z - right.y*z;
        temp[2] = w*right.y + right.w*y + z*right.x - right.z*x;
        z = w*right.z + right.w*z + x*right.y - right.x*y;

        w = temp[0];
        x = temp[1];
        y = temp[2];
        return *this;
    }

    inline Quaternion& mult(const Quaternion& left, const Vector3& right)
    {
        float xt =   left.w*right.v[0] + left.y*right.v[2] - left.z*right.v[1];
        float yt =   left.w*right.v[1] + left.z*right.v[0] - left.x*right.v[2];
        float zt =   left.w*right.v[2] + left.x*right.v[1] - left.y*right.v[0];
        float wt = - left.x*right.v[0] - left.y*right.v[1] - left.z*right.v[2];

        x = xt;
        y = yt;
        z = zt;
        w = wt;
        return *this;
    }

    inline Quaternion& mult(float s)
    {
        x *= s;
        y *= s;
        z *= s;
        w *= s;
        return *this;
    }

    inline Quaternion& add(const Quaternion& left, const Quaternion& right)
    {
        x = left.x + right.x;
        y = left.y + right.y;
        z = left.z + right.z;
        w = left.w + right.w;
        return *this;
    }

    inline Quaternion& add(const Quaternion& right)
    {
        x += right.x;
        y += right.y;
        z += right.z;
        w += right.w;
        return *this;
    }

    inline Quaternion& sub(const Quaternion& left, const Quaternion& right)
    {
        x = left.x - right.x;
        y = left.y - right.y;
        z = left.z - right.z;
        w = left.w - right.w;
        return *this;
    }

    inline Quaternion& sub(const Quaternion& right)
    {
        x -= right.x;
        y -= right.y;
        z -= right.z;
        w -= right.w;
        return *this;
    }

    inline Quaternion& negate()
    {
        x = -x;
        y = -y;
        z = -z;
        w = -w;
        return *this;
    }

    inline void normalise()
    {
        float oneOverLen = MathUtil::invSqrt(N2(*this));
        x *= oneOverLen;
        y *= oneOverLen;
        z *= oneOverLen;
        w *= oneOverLen;
    }

    inline float N(const Quaternion &Q)
    {
        return sqrt(Q.x*Q.x + Q.y*Q.y + Q.z*Q.z + Q.w*Q.w);
    }

    inline float N2(const Quaternion &Q)
    {
        return Q.x*Q.x + Q.y*Q.y + Q.z*Q.z + Q.w*Q.w;
    }

    inline void rot(Vector3& v) const
    {
        // Assume unit quaternion
        Quaternion inverseQuat(-x, -y, -z, w);

        Quaternion left;
        left.mult(*this, v);
        v.v[0] = left.w*inverseQuat.x + inverseQuat.w*left.x + left.y*inverseQuat.z - inverseQuat.y*left.z;
        v.v[1] = left.w*inverseQuat.y + inverseQuat.w*left.y + left.z*inverseQuat.x - inverseQuat.z*left.x;
        v.v[2] = left.w*inverseQuat.z + inverseQuat.w*left.z + left.x*inverseQuat.y - inverseQuat.x*left.y;
    }

    inline float dotProduct(const Quaternion& other) const
    {
        return w*other.w+x*other.x+y*other.y+z*other.z;
    }

    Quaternion(const Vector3& euler);

    void set(const Matrix4x4& matrix);

    void nlerp(const Quaternion &Q, const Quaternion &R, float t);
    void nlerpShortestPath(const Quaternion &Q, const Quaternion &R, float t);
    void slerp(const Quaternion &Q, const Quaternion &R, float t);
    void slerpShortestPath(const Quaternion& from, const Quaternion& to, float t);
    void addShortestPath(const Quaternion &Q, float t);

    void getEulerAngles(Vector3& destEuler) const;


    float w, x, y, z;


};

