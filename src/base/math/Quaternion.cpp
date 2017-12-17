/*
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#include "PreCompiled.h"

#include <cmath>

#include "base/math/MathUtil.h"

#include "Quaternion.h"
#include "Matrix4x4.h"

void Quaternion::set(const Matrix4x4& matrix)
{
    float tr = matrix.m[0] + matrix.m[5] + matrix.m[10];
    float s;
    if (tr >= 0) {
        s = sqrtf(tr + 1);
        w = 0.5f * s;
        s = 0.5f / s;
        x = (matrix.m[6] - matrix.m[9]) * s;
        y = (matrix.m[8] - matrix.m[2]) * s;
        z = (matrix.m[1] - matrix.m[4]) * s;
    } else {
        int i = 0;
        if (matrix.m[5] > matrix.m[0]) {
            i = 1;
        }
        if (matrix.m[10] > matrix.m[i*4 + i]) {
            i = 2;
        }
        switch (i) {
        case 0:
            s = sqrtf((matrix.m[0] - (matrix.m[5] + matrix.m[10])) + 1);
            x = 0.5f * s;
            s = 0.5f / s;
            y = (matrix.m[4] + matrix.m[1]) * s; 
            z = (matrix.m[2] + matrix.m[8]) * s;
            w = (matrix.m[6] - matrix.m[9]) * s;
            break;
        case 1:
            s = sqrtf((matrix.m[5] - (matrix.m[10] + matrix.m[0])) + 1);
            y = 0.5f * s;
            s = 0.5f / s;
            z = (matrix.m[9] + matrix.m[6]) * s;
            x = (matrix.m[4] + matrix.m[1]) * s;
            w = (matrix.m[8] - matrix.m[2]) * s;
            break;
        case 2:
            s = sqrtf((matrix.m[10] - (matrix.m[0] + matrix.m[5])) + 1);
            z = 0.5f * s;
            s = 0.5f / s;
            x = (matrix.m[2] + matrix.m[8]) * s;
            y = (matrix.m[9] + matrix.m[6]) * s;
            w = (matrix.m[1] - matrix.m[4]) * s;
        }
    }
}

Quaternion::Quaternion(const Vector3& euler)
{
    set(Matrix4x4(euler, Vector3(1,1,1), Vector3(0,0,0)));
    // this is does not use the correct euler order
    /*float c1 = cosf(euler.v[0]/2.0f);
    float s1 = sinf(euler.v[0]/2.0f);
    float c2 = cosf(euler.v[1]/2.0f);
    float s2 = sinf(euler.v[1]/2.0f);
    float c3 = cosf(euler.v[2]/2.0f);
    float s3 = sinf(euler.v[2]/2.0f);
    float c1c2 = c1*c2;
    float s1s2 = s1*s2;
    w = c1c2*c3 - s1s2*s3;
    x = c1c2*s3 + s1s2*c3;
    y = s1*c2*c3 + c1*s2*s3;
    z = c1*s2*c3 - s1*c2*s3;*/
}


/**
 * Normalized linear interpolation
 *
 * @param Q  first quaternion.
 * @param R  second quaternion.
 * @param t  weight of the second quaternion.
 */
void Quaternion::nlerp(const Quaternion &Q, const Quaternion &R, float t)
{
    sub(R, Q);
    mult(t);
    add(Q); // *this = Q + t * (R - Q);
    normalise();
}

void Quaternion::nlerpShortestPath(const Quaternion &Q, const Quaternion &R, float t)
{
    float c = Q.dotProduct(R);
    if (c < 0.0f) {
        *this = R;
        negate();
        sub(Q); // ((-R) - Q)
    } else {
        sub(R, Q); // (R - Q)
    }
    mult(t);
    add(Q); // *this = Q + t * (R - Q) or Q + t * ((-R) - Q);
    normalise();
}

void Quaternion::slerp(const Quaternion &Q, const Quaternion &R, float t)
{
    //return Quaternion(Q, R, t);
    float a = acosf(Q.x * R.x + Q.y * R.y + Q.z * R.z + Q.w * R.w);
    if (fabsf(a) < 1e-03f) {
        *this = Q;
        return;
    }
    float s = sinf(a);

    Quaternion qt(Q);
    qt.mult(sinf(a * (1 - t)));

    Quaternion rt(R);
    rt.mult(sinf(a * t));
    add(rt, qt);
    mult(1.0f/s);
}

void Quaternion::slerpShortestPath(const Quaternion& from, const Quaternion& to, float t)
{
    float fCos = from.dotProduct(to);
    float fAngle = acosf(fCos);

    if (fabsf(fAngle) < 1e-03f) {
        *this = from;
        return;
    }

    float fSin = sinf(fAngle);
    float fInvSin = 1.0f/fSin;
    float fCoeff0 = sinf((1.0f-t)*fAngle)*fInvSin;
    float fCoeff1 = sinf(t*fAngle)*fInvSin;

    Quaternion tempFrom(from); 
    Quaternion tempTo(to);
    tempTo.mult(fCoeff1);
    // do we need to invert rotation?
    if (fCos < 0.0f) {
        fCoeff0 = -fCoeff0;
        // taking the complement requires renormalisation
        tempFrom.mult(fCoeff0);
        add(tempFrom, tempTo);
        normalise();
    } else {
        tempFrom.mult(fCoeff0);
        add(tempFrom, tempTo);
    }
}

void Quaternion::addShortestPath(const Quaternion &Q, float t)
{
    float c = dotProduct(Q);
    Quaternion tmp = Q;
    if (c < 0.0f) {
        tmp.negate();
    }
    tmp.mult(t);
    add(tmp); // *this = this + t * Q or this + t * (-Q);
}

void Quaternion::getEulerAngles(Vector3& destEuler) const
{
    Matrix4x4 matrix;
    matrix.identity();
    matrix.setRotation(*this);
    matrix.getEulerAngles(destEuler);

    /*
    // TODO check if the code below is correct
    float test = x*y + z*w;
    if (test > 0.499f) { // singularity at north pole
        destEuler.v[0] = 2.0f * atan2(x,w);
        destEuler.v[1] = (float)GS_PI/2.0f;
        destEuler.v[2] = 0.0f;
        return;
    }
    if (test < -0.499f) { // singularity at south pole
        destEuler.v[0] = -2.0f * atan2(x,w);
        destEuler.v[1] = -(float)GS_PI/2.0f;
        destEuler.v[2] = 0.0f;
        return;
    }
    float sqx = x*x;
    float sqy = y*y;
    float sqz = z*z;
    destEuler.v[0] = atan2f(2.0f*y*w-2.0f*x*z , 1.0f - 2.0f*sqy - 2.0f*sqz);
    destEuler.v[1] = asinf(2.0f*test);
    destEuler.v[2] = atan2f(2.0f*x*w-2.0f*y*z , 1.0f - 2.0f*sqx - 2.0f*sqz);*/
}
