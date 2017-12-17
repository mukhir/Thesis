/** \class MathUtil
 * Math Util
 *
 * @author  Christian Ammann
 * @date  21/08/03
 *
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#pragma once

#include <math.h>

#include "base/Prerequisites.h"
#include "base/Platform.h"

class Vector3;
class Quaternion;

class MathUtil
{
public:

    static float invSqrt16(float x);
    static float invSqrt(float x);
    static double invSqrt64(float x);
    static float getRandom();
    static float getRandomMinMax(float fMin, float fMax);
    static int getRandomMinMax(int iMin, int iMax);
    static void getRandomVector3(Vector3& randVec);
    static float degreeToRadian(float degree);
    static float radianToDegree(float radian);
    static bool isPow2(const int n);
    static int nextPow2(const int x);
    static float sign(float f);

    template <class C> static unsigned int getMaxElement(C* pElements, unsigned int numElements)
    {
        C* curMaxElement = &(pElements[0]);
        unsigned int maxIndex = 0;
        for (unsigned int i=1; i <numElements; i++) {
            if (pElements[i] > *curMaxElement) {
                maxIndex = i;
                curMaxElement = &(pElements[i]);
            }
        }
        return maxIndex;
    }

    template <class C> static unsigned int getMinElement(C* pElements, unsigned int numElements)
    {
        C* curMinElement = &(pElements[0]);
        unsigned int minIndex = 0;
        for (unsigned int i=1; i <numElements; i++) {
            if (pElements[i] < *curMinElement) {
                minIndex = i;
                curMinElement = &(pElements[i]);
            }
        }
        return minIndex;
    }

    static void tangent(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& w1, const Vector3& w2, const Vector3& w3, const Vector3& normal, Vector3& tangentVal, Vector3& biTangentVal, float& biTangentFactor);

    static void tangentArray(unsigned int numVertices, float* pVertices, float* pNormals, float* pTexCoords, unsigned int numTriangles, unsigned short* pIndices, bool normalizeTangents, float* pTangents);
    static void tangentArray(unsigned int numVertices, float* pVertices, float* pNormals, float* pTexCoords, unsigned int numTriangles, unsigned int* pIndices, bool normalizeTangents, float* pTangents);

    static float getTetrahedronVolume(const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector3& p4);
    static bool lineLineIntersect3D (const Vector3& p1, const Vector3& _dir1, const Vector3& p2, const Vector3& _dir2, Vector3& dest);
    static bool coplanarTriTriIntersect(const Vector3& normal, const Vector3& t1p1, const Vector3& t1p2, const Vector3& t1p3, const Vector3& t2p1, const Vector3& t2p2, const Vector3& t2p3);
    static bool edgeTriIntersect(const Vector3& edgeStart, const Vector3& edgeEnd, const Vector3& tp1, const Vector3& tp2, const Vector3& tp3, int i0, int i1);
    static bool pointInTri(const Vector3& point, const Vector3& tp1, const Vector3& tp2, const Vector3& tp3, int i0, int i1);
    static bool rayTriangleIntersect3D(const Vector3& rayOrigin, const Vector3& rayDir, const Vector3& vert0, const Vector3& vert1, const Vector3& vert2, float& distance);

    static void cartesianToPolar(const Vector3& vec, float& theta, float& rho);

    static void quatTransToDualQuaternion(const Quaternion& quat, const Vector3& trans, float dualQuat[2][4]);

    static float fround(float val, int digits)
    {
        int modifier = 1;
        for (int i = 0; i < digits; ++i){
            modifier *= 10;
        }

        if(val < 0.0f){
            return (floor(val * modifier - 0.5f) / modifier);
        }

        return (floor(val * modifier + 0.5f) / modifier);
    }

    static inline bool isNaN(float value) {
#if defined(_WIN32) || defined(_XBOX)
        return (_isnan(value) != 0);
#elif defined(__CELLOS_LV2__)
        return isNaN(value);
#elif defined(__APPLE__) || defined(__linux__)
        return isnan(value);
#else
#error MathUtil::isNaN not implemented
#endif
    }

    static inline bool isFinite(float value) {
#if defined(_WIN32) || defined(_XBOX)
        return (_finite(value) != 0);
#elif defined(__CELLOS_LV2__)
        return isFinite(value);
#elif defined(__APPLE__) || defined(__linux__)
        return finite(value);
#else
#error MathUtil::isFinite not implemented
#endif
    }

#if 0
    typedef void (*FASTVECTORTRANSFORM)(float* m, unsigned char* vin, unsigned char* vout, unsigned int num);
    static FASTVECTORTRANSFORM getFastVectorTransform(unsigned int stride);
    static void fastVectorTransform12bytes(float* m, unsigned char* vin, unsigned char* vout, unsigned int num);
    static void fastVectorTransform16bytes(float* m, unsigned char* vin, unsigned char* vout, unsigned int num);
    static void fastVectorTransform32bytes(float* m, unsigned char* vin, unsigned char* vout, unsigned int num);
    static void fastVectorTransform48bytes(float* m, unsigned char* vin, unsigned char* vout, unsigned int num);
    static void fastVectorTransform4bytesAligned(float* m, unsigned char* vin, unsigned char* vout, unsigned int num);
#endif

private:

#if !defined(NN_PLATFORM_CTR)


    enum {
        LOOKUP_BITS      = 8,                        // number of mantissa bits for lookup
        EXP_POS          = 23,                       // position of exponent
        EXP_BIAS         = 127,                      // bias of exponent
        LOOKUP_POS       = (EXP_POS-LOOKUP_BITS),    // position of the mantissa lookup
        SEED_POS         = (EXP_POS-8),              // position of the mantissa seed
        ISQRT_TABLE_SIZE = (2 << LOOKUP_BITS),       // number of entries in the lookup table
        LOOKUP_MASK      = (ISQRT_TABLE_SIZE - 1)    // mask for table input
    };

    union _flint {
        unsigned int    i;
        float           f;
    };

    class MathUtilSqrtTable {
    public:
        MathUtilSqrtTable()
        {
            union _flint fi, fo;

            for (unsigned int i = 0; i < ISQRT_TABLE_SIZE; i++) {
                fi.i        = ((EXP_BIAS-1) << EXP_POS) | (i << LOOKUP_POS);
                fo.f        = (float)(1.0 / ::sqrt((double)fi.f));
                m_iSqrt[i]    = ((unsigned int)(((fo.i + (1<<(SEED_POS-2))) >> SEED_POS) & 0xFF))<<SEED_POS; // rounding
            }
            m_iSqrt[ISQRT_TABLE_SIZE / 2] = ((unsigned int)(0xFF))<<(SEED_POS); // special case for invSqrt(1.0)
        }
        unsigned int m_iSqrt[ISQRT_TABLE_SIZE];
    };

    static MathUtilSqrtTable m_sqrtTable;

#endif

    template <class IndexType> static void tangentArrayInternal(unsigned int numVertices, float* pVertices, float* pNormals, float* pTexCoords, unsigned int numTriangles, IndexType* pIndices, bool normalizeTangents, float* pTangents);
};

GS_FORCEINLINE float MathUtil::invSqrt16(float x)
{
#if defined(NN_PLATFORM_CTR)
    return 1.0f/::sqrtf(x);
#else
    unsigned int a = ((union _flint*)(&x))->i;
    union _flint seed;

    double xhalf = (double)x * 0.5;
    seed.i = (( ( (3*EXP_BIAS-1) - ( (a >> EXP_POS) & 0xFF) ) >> 1)<<EXP_POS) | m_sqrtTable.m_iSqrt[(a >> (EXP_POS-LOOKUP_BITS)) & LOOKUP_MASK];
    double r = seed.f;              // accurate to LOOKUP_BITS
    r = r * (1.5 - r * r * xhalf); // first iteration: accurate to 2*LOOKUP_BITS <- LOOKUP_BITS = 8
    return (float)r;
#endif
}

GS_FORCEINLINE float MathUtil::invSqrt(float x)
{
#if defined(NN_PLATFORM_CTR)
    return 1.0f/::sqrtf(x);
#else
    unsigned int a = ((union _flint*)(&x))->i;
    union _flint seed;

    double xhalf = (double)x * 0.5;
    seed.i = (( ( (3*EXP_BIAS-1) - ( (a >> EXP_POS) & 0xFF) ) >> 1)<<EXP_POS) | m_sqrtTable.m_iSqrt[(a >> (EXP_POS-LOOKUP_BITS)) & LOOKUP_MASK];
    double r = seed.f;              // accurate to LOOKUP_BITS
    r = r * (1.5 - r * r * xhalf); // first iteration:  accurate to 2*LOOKUP_BITS
    r = r * (1.5 - r * r * xhalf); // second iteration: accurate to 4*LOOKUP_BITS <- LOOKUP_BITS = 8
    return (float)r;
#endif
}

GS_FORCEINLINE double MathUtil::invSqrt64(float x)
{
#if defined(NN_PLATFORM_CTR)
    return 1.0/::sqrt((double)x);
#else
    unsigned int a = ((union _flint*)(&x))->i;
    union _flint seed;

    double xhalf = (double)x * 0.5;
    seed.i = (( ( (3*EXP_BIAS-1) - ( (a >> EXP_POS) & 0xFF) ) >> 1)<<EXP_POS) | m_sqrtTable.m_iSqrt[(a >> (EXP_POS-LOOKUP_BITS)) & LOOKUP_MASK];
    double r = seed.f;              // accurate to LOOKUP_BITS
    r = r * (1.5 - r * r * xhalf); // first iteration:  accurate to 2*LOOKUP_BITS
    r = r * (1.5 - r * r * xhalf); // second iteration: accurate to 4*LOOKUP_BITS
    r = r * (1.5 - r * r * xhalf); // third iteration:  accurate to 8*LOOKUP_BITS <- LOOKUP_BITS = 8
    return r;
#endif
}
