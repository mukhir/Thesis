/*
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#include "PreCompiled.h"

#include "Vector3.h"

/**
 * Linear interpolation from 'from' to 'to'
 *
 * @param t alpha value.
 * @param from start vector.
 * @param to end vector.
 */
void Vector3::lerp(const float t, const Vector3& from, const Vector3& to)
{
    if (t <= 0.0f) {
        (*this) = from;
    } else if (t >= 1.0f) {
        (*this) = to;
    } else {
        (*this) = from; // this = from + t * (to - from);
        Vector3 tmp(to);
        tmp.sub(from).scale(t);
        add(tmp);
    }
}

/**
 * Linear interpolation from 'from' to 'to'
 * Interpolates the values as euler angles
 *
 * @param t alpha value.
 * @param from start vector.
 * @param to end vector.
 */
void Vector3::lerpEulerShortestPath(const float t, const Vector3& from, const Vector3& to)
{
    if (t <= 0.0f) {
        (*this) = from;
    } else if (t >= 1.0f) {
        (*this) = to;
    } else {
        float diff;
        for (unsigned int i=0; i < 3; i++) {
            diff = to.v[i] - from.v[i];
            if (fabsf(diff) > 2.0f*(float)GS_PI-fabsf(diff)) {
                // we need to take the shorter path
                if (diff < 0.0f) {
                    v[i] = from.v[i] + t * (-2.0f*(float)GS_PI - diff);
                } else {
                    v[i] = from.v[i] + t * (2.0f*(float)GS_PI - diff);
                }
            } else {
                v[i] = from.v[i] + t * diff;
            }
        }
    }
}

/**
 * Spherical interpolation from 'from' to 'to'
 * Vectors are expected to be normalized.
 *
 * @param t alpha value.
 * @param from start vector.
 * @param to end vector.
 */
void Vector3::slerp(const float t, const Vector3& from, const Vector3& to)
{
    float scale0, scale1;

    if (t <= 0.0f) {
        (*this) = from;
        return;
    } else if ( t >= 1.0f ) {
        (*this) = to;
        return;
    }

    float cosom = from.dotProduct(to);
    if ( (1.0f - cosom) > 1e-6f) {
        float omega = acosf(cosom);
        float sinom = sinf(omega);
        scale0 = sinf( (1.0f - t)*omega) / sinom;
        scale1 = sinf(t*omega) / sinom;
    } else {
        scale0 = 1.0f - t;
        scale1 = t;
    }
    Vector3 tmp(to);
    tmp.scale(scale1);
    (*this) = from;       // this = from * scale0 + to * scale1;
    scale(scale0).add(tmp);
}
