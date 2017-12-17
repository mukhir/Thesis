/** \class Vector3
 * Vector class for R^3
 *
 * @author  Christian Ammann
 * @date  18/02/03
 *
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#pragma once

#include "base/math/MathUtil.h"
#include "base/util/DebugUtil.h"

#include <cmath>

class Vector3
{
public:
    inline Vector3() {};
    inline Vector3(float x, float y, float z) { set(x,y,z); }

    // 3 floats assumed
    inline Vector3(const float* floats) { operator=(floats); }
    inline Vector3(const Vector3& operand) { operator=(operand); }

    inline void zeroise() { v[0] = v[1] = v[2] = 0.0f; }

    inline void set(float xt, float yt, float zt)
    {
        v[0] = xt;
        v[1] = yt;
        v[2] = zt;
    }

    inline void set(const float* pFloats)
    {
        v[0] = pFloats[0];
        v[1] = pFloats[1];
        v[2] = pFloats[2];
    }

    inline Vector3& operator=(const float* floats)
    {
        v[0] = floats[0];
        v[1] = floats[1];
        v[2] = floats[2];
        return *this;
    }

    inline Vector3& operator=(const Vector3& operand)
    {
        v[0] = operand.v[0];
        v[1] = operand.v[1];
        v[2] = operand.v[2];
        return *this;
    }

    /**
     * Watch out when comparing floating point values:
     * 0x80000000 is negative zero (-1.0 * 2**-127), and 
     * 0x00000000 is positive zero (1.0 * 2**-127)
     * This method will return false when comparing negative zero with positive zero
     */
    inline bool operator==(const Vector3& operand) const
    {
        unsigned int m = (*(unsigned int*)&v[0]) - (*(unsigned int*)&operand.v[0]);
        m |= (*(unsigned int*)&v[1]) - (*(unsigned int*)&operand.v[1]);
        m |= (*(unsigned int*)&v[2]) - (*(unsigned int*)&operand.v[2]);
        return (m == 0);
    }

    inline bool isApproximately(const Vector3& operand, float margin) const
    {
        if ( fabs(v[0]-operand.v[0])<margin && fabs(v[1]-operand.v[1])<margin && fabs(v[2]-operand.v[2])<margin ) {
            return true;
        }
        return false;
    }

    inline float* raw() { return v; }
    inline const float* rawConst() const { return v; }
    inline float& operator[](int n) { return v[n]; }
    inline const float& operator[](int n) const { return v[n]; }


    // returns true if any componnent is non-zero
    inline bool isNonZero(void) const
    {
        return (v[0]!=0.0f || v[1]!=0.0f || v[2]!=0.0f);
    }
        // returns true if any componnent is zero

    inline bool hasZeroComponent(void) const
    {
        return (v[0]==0.0f || v[1]==0.0f || v[2]==0.0f);
    }

    inline Vector3& add(const Vector3& operand)
    {
        v[0]+=operand.v[0];
        v[1]+=operand.v[1];
        v[2]+=operand.v[2];
        return *this;
    }

    inline Vector3& madd(const Vector3& operand, float scale)
    {
        v[0]+=operand.v[0]*scale;
        v[1]+=operand.v[1]*scale;
        v[2]+=operand.v[2]*scale;
        return *this;
    }

    inline Vector3& add(const Vector3& left, const Vector3& right)
    {
        v[0]=left.v[0]+right.v[0];
        v[1]=left.v[1]+right.v[1];
        v[2]=left.v[2]+right.v[2];
        return *this;
    }

    inline Vector3& sub(const Vector3& operand)
    {
        v[0] -= operand.v[0];
        v[1] -= operand.v[1];
        v[2] -= operand.v[2];
        return *this;
    }

    inline Vector3& sub(const Vector3& left, const Vector3& right)
    {
        v[0] = left.v[0]-right.v[0];
        v[1] = left.v[1]-right.v[1];
        v[2] = left.v[2]-right.v[2];
        return *this;
    }

    inline Vector3& mult(const Vector3& operand)
    {
        v[0] *= operand.v[0];
        v[1] *= operand.v[1];
        v[2] *= operand.v[2];

        return *this;
    }

    inline Vector3& mult(const Vector3& left, const Vector3& right)
    {
        v[0] = left.v[0]*right.v[0];
        v[1] = left.v[1]*right.v[1];
        v[2] = left.v[2]*right.v[2];

        return *this;
    }

    inline Vector3& div(const Vector3& operand)
    {
        GS_ASSERT(!operand.hasZeroComponent());

        if (operand.v[0] != 0.0f) {
            v[0] /= operand.v[0];
        }
        if (operand.v[1] != 0.0f) {
            v[1] /= operand.v[1];
        }
        if (operand.v[2] != 0.0f) {
            v[2] /= operand.v[2];
        }
        return *this;
    }

    inline Vector3& div(const Vector3& left, const Vector3& right)
    {
        GS_ASSERT(!right.hasZeroComponent());

        if (right.v[0] != 0.0f) {
            v[0] = left.v[0]/right.v[0];
        }
        if(right.v[1] != 0.0f) {
            v[1] = left.v[1]/right.v[1];
        }
        if(right.v[2] != 0.0f) {
            v[2] = left.v[2]/right.v[2];
        }
        return *this;
    }

    inline Vector3& rescale(float magnitude)
    {
        GS_ASSERT(isNonZero());

        float scalar = magnitude * MathUtil::invSqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
        v[0] *= scalar;
        v[1] *= scalar;
        v[2] *= scalar;
        return *this;
    }

    inline Vector3& scale(float scalar)
    {
        v[0] *= scalar;
        v[1] *= scalar;
        v[2] *= scalar;
        return *this;
    }

    inline Vector3& scale(const Vector3& operand, float scalar)
    {
        v[0] = operand.v[0]*scalar;
        v[1] = operand.v[1]*scalar;
        v[2] = operand.v[2]*scalar;
        return *this;
    }

    // additive inverse
    inline Vector3& negate()
    {
        v[0]= -v[0];
        v[1]= -v[1];
        v[2]= -v[2];

        return *this;
    }

    // multiplicative inverse
    inline Vector3& reciprocate()
    {
        v[0] = (v[0] == 0.0f)? 0.0f: 1.0f/v[0];
        v[1] = (v[1] == 0.0f)? 0.0f: 1.0f/v[1];
        v[2] = (v[2] == 0.0f)? 0.0f: 1.0f/v[2];

        return *this;
    }


    inline Vector3& crossProduct(const Vector3& left, const Vector3& right){
        GS_ASSERT(this!=&left);
        GS_ASSERT(this!=&right);

        v[0] = left.v[1]*right.v[2] - left.v[2]*right.v[1];
        v[1] = left.v[2]*right.v[0] - left.v[0]*right.v[2];
        v[2] = left.v[0]*right.v[1] - left.v[1]*right.v[0];

        return *this;
    }

    inline static float dotProduct(const Vector3& left, const Vector3& right)
    {
        return left.v[0]*right.v[0]+left.v[1]*right.v[1]+left.v[2]*right.v[2];
    }

    inline float dotProduct(const Vector3& operand) const
    {
        return v[0]*operand.v[0]+v[1]*operand.v[1]+v[2]*operand.v[2];
    }

    inline Vector3& normalize()
    {
        float invLength = MathUtil::invSqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
        v[0] *= invLength;
        v[1] *= invLength;
        v[2] *= invLength;

        return *this;
    }

    inline float normalizeRet()
    {
        float sqrLength = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
        float invLength = MathUtil::invSqrt(sqrLength);
        v[0] *= invLength;
        v[1] *= invLength;
        v[2] *= invLength;

        return sqrLength * invLength;
    }

    inline float calcMagnitude() const
    {
        return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    }

    inline float calcMagnitudeSquared() const
    {
        return v[0]*v[0]+v[1]*v[1]+v[2]*v[2];
    }

    inline float calcDistanceFrom(const Vector3& operand) const
    {
        return sqrt(calcDistanceSquaredFrom(operand));
    }

    inline float calcDistanceSquaredFrom(const Vector3& operand) const
    {
        float part = v[0]-operand.v[0];
        float sum  = part*part;

        part = v[1]-operand.v[1];
        sum += part*part;

        part = v[2]-operand.v[2];
        return sum + part*part;
    }
   
    inline void min(Vector3& other)
    {
        v[0] = GS_MIN(v[0], other.v[0]);
        v[1] = GS_MIN(v[1], other.v[1]);
        v[2] = GS_MIN(v[2], other.v[2]);
    }
  
    inline void max(Vector3& other)
    {
        v[0] = GS_MAX(v[0], other.v[0]);
        v[1] = GS_MAX(v[1], other.v[1]);
        v[2] = GS_MAX(v[2], other.v[2]);
    }
 
 

    // linear interpolation
    void lerp(const float t, const Vector3& from, const Vector3& to);
    // linear interpolation
    void lerpEulerShortestPath(const float t, const Vector3& from, const Vector3& to);
    // spherical interpolation
    void slerp(const float t, const Vector3& from, const Vector3& to);

    
    float v[3];
};



