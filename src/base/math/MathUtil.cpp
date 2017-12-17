/*
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#include "PreCompiled.h"

#include "MathUtil.h"

#include "base/util/DebugUtil.h"

#include "base/math/Vector3.h"
#include "base/math/Plane.h"
#include "Quaternion.h"

#if !defined(NN_PLATFORM_CTR)
MathUtil::MathUtilSqrtTable MathUtil::m_sqrtTable;
#endif

/**
 * Get random value
 *
 * @returns a random value.
 */
float MathUtil::getRandom()
{
    return ((float)rand())/RAND_MAX;
}

/**
 * Get random value on a interval
 *
 * @param fMin interval begin.
 * @param fMax interval end.
 * @returns random value.
 */
float MathUtil::getRandomMinMax(float fMin, float fMax)
{
    GS_ASSERT(fMin <= fMax);

    float randNum = getRandom();
    return fMin + (fMax - fMin) * randNum;
}

/**
 * Get random value on a interval
 *
 * @param iMin interval begin.
 * @param iMax interval end.
 * @returns random value.
 */
int MathUtil::getRandomMinMax(int iMin, int iMax)
{
    int diff = iMax - iMin + 1;

    return iMin + (rand() % diff);
}


/**
 * Get a random vector with length between 0 and 1
 *
 * @param randVec The vector where the random vector is written to
 */
void MathUtil::getRandomVector3(Vector3& randVec)
{
    // Pick a random Z between -1.0f and 1.0f.
    randVec.v[2] = getRandomMinMax(-1.0f, 1.0f);
    
    // Get radius of this circle
    float radius = sqrtf(1.0f - randVec.v[2] * randVec.v[2]);
    
    // Pick a random point on a circle.
    float t = getRandomMinMax(-(float)GS_PI, (float)GS_PI);

    // Compute matching X and Y for our Z.
    randVec.v[0] = cosf(t) * radius;
    randVec.v[1] = sinf(t) * radius;
}

/**
 * Convert degree to radian
 *
 * @param degree  degree value.
 * @returns radian value.
 */
float MathUtil::degreeToRadian(float degree)
{
    return ((float)(( (degree) / 180.0f ) * (float)GS_PI));
}

/**
 * Convert radian to degree
 *
 * @param radian  radian value.
 * @returns degree value.
 */
float MathUtil::radianToDegree(float radian)
{
    return ((float)(( (radian) * 180.0f ) / (float)GS_PI));
}

/**
 * Check if a number is power of two.
 *
 * @param n  number.
 * @returns true if number is power of two.
 */
bool MathUtil::isPow2(const int n)
{
    return n > 0 && (n & (n - 1)) == 0;
}

/**
 * This function gets the first power of 2 >= the int that we pass it.
 *
 * @param x  value to get next pow2 of.
 * @returns first power of 2 >= a.
 */
int MathUtil::nextPow2(const int x)
{
    int rval = 1;
    while (rval < x) {
        rval<<=1;
    }
    return rval;
}

float MathUtil::sign(float f)
{
    if (f > 0.0f) {
        return 1.0f;
    }
    if (f < 0.0f) {
        return -1.0f;
    }
    return 0.0f;
}

void MathUtil::tangent(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& w1, const Vector3& w2, const Vector3& w3, const Vector3& normal, Vector3& tangentVal, Vector3& biTangentVal, float& biTangentFactor)
{

    float x1 = v2.v[0] - v1.v[0];
    float x2 = v3.v[0] - v1.v[0];
    float y1 = v2.v[1] - v1.v[1];
    float y2 = v3.v[1] - v1.v[1];
    float z1 = v2.v[2] - v1.v[2];
    float z2 = v3.v[2] - v1.v[2];
    
    float s1 = w2.v[0] - w1.v[0];
    float s2 = w3.v[0] - w1.v[0];
    float t1 = w2.v[1] - w1.v[1];
    float t2 = w3.v[1] - w1.v[1];

    Vector3 sdir;
    Vector3 tdir;

    float denominator = (s1 * t2 - s2 * t1);
    const float epsilon = 0.00001f;

    if (denominator < epsilon && denominator > -epsilon) {
        sdir.set(1, 0, 0);
        tdir.set(0, 1, 0);
    } else {
        float r = 1.0f / denominator;
        sdir.set((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
/*
    T = Vector3((vDirVec_v3v_to_v1v * vDirVec_v2_to_v1.v[0] - vDirVec_v2v_to_v1v * vDirVec_v3_to_v1.v[0]) * fScale1,
            (vDirVec_v3v_to_v1v * vDirVec_v2_to_v1.v[1] - vDirVec_v2v_to_v1v * vDirVec_v3_to_v1.v[1]) * fScale1,
            (vDirVec_v3v_to_v1v * vDirVec_v2_to_v1.v[2] - vDirVec_v2v_to_v1v * vDirVec_v3_to_v1.v[2]) * fScale1);
*/
        
        
        
        tdir.set((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

/*
        B = Vector3((-vDirVec_v3u_to_v1u * vDirVec_v2_to_v1.v[0] + vDirVec_v2u_to_v1u * vDirVec_v3_to_v1.v[0]) * fScale1,
                     (-vDirVec_v3u_to_v1u * vDirVec_v2_to_v1.v[1] + vDirVec_v2u_to_v1u * vDirVec_v3_to_v1.v[1]) * fScale1,
                     (-vDirVec_v3u_to_v1u * vDirVec_v2_to_v1.v[2] + vDirVec_v2u_to_v1u * vDirVec_v3_to_v1.v[2]) * fScale1);
*/


    }

    // Gram-Schmidt orthogonalize
    //tangent = |tangent - normal * (normal dot tangent)|;
    Vector3 tmpNorm(normal);
    tmpNorm.scale(normal.dotProduct(sdir));
    tangentVal.sub(sdir, tmpNorm);
    tangentVal.normalize();

    //float tmp = tangent->v[1];
    //tangent->v[1] = tangent->v[2];
    //tangent->v[2] = tmp*-1;
    //tangent->set(0,0,0);

    biTangentVal.crossProduct(normal, tangentVal);
//    biTangent->crossProduct(*tangent, *normal);

/*    
    // Calculate handedness
    //tangent->v[3] = ((n cross t) dot tan2[a] < 0) ? -1 : 1;
    Vector3 txn;
    txn.crossProduct(*normal, *tangent);
    tangent->v[3] = (txn.dotProduct(tdir) < 0.0f) ? -1.0f : 1.0f;
*/

    biTangentFactor = 1.0f;
    Vector3 txn;
    txn.crossProduct(normal, tangentVal);
    if ((txn.dotProduct(tdir) < 0.0f)) {
        biTangentVal.negate();
        biTangentFactor = -1.0f;
    }

}

void MathUtil::tangentArray(unsigned int numVertices, float* pVertices, float* pNormals, float* pTexCoords, unsigned int numTriangles, unsigned short* pIndices, bool normalizeTangents, float* pTangents)
{
    tangentArrayInternal(numVertices, pVertices, pNormals, pTexCoords, numTriangles, pIndices, normalizeTangents, pTangents);
}

void MathUtil::tangentArray(unsigned int numVertices, float* pVertices, float* pNormals, float* pTexCoords, unsigned int numTriangles, unsigned int* pIndices, bool normalizeTangents, float* pTangents)
{
    tangentArrayInternal(numVertices, pVertices, pNormals, pTexCoords, numTriangles, pIndices, normalizeTangents, pTangents);
}

float MathUtil::getTetrahedronVolume(const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector3& p4)
{
    float mat[3][3];
    float ret;

    for (unsigned int i=0; i<3; i++) {
        mat[0][i] = p1.v[i] - p4.v[i];
        mat[1][i] = p2.v[i] - p4.v[i];
        mat[2][i] = p3.v[i] - p4.v[i];
    }
    ret = mat[0][0] * ( mat[1][1]*mat[2][2] - mat[1][2]*mat[2][1] ) +
          mat[0][1] * ( mat[1][2]*mat[2][0] - mat[1][0]*mat[2][2] ) +
          mat[0][2] * ( mat[1][0]*mat[2][1] - mat[1][1]*mat[2][0] );
    ret = ret/6.0f;
    return ret;
}

/** 
 * line line intersection
 * returns false if lines are parallel, otherwise true
 * dest is set to the point on the second line which is closest to the first line
 *
 * @param p1  point on first line
 * @param _dir1  direction of first line
 * @param p2  point on second line
 * @param _dir2  direction of second line
 * @param dest  intersection point
 * @returns returns false if the lines are parallel, otherwise true
 */
bool MathUtil::lineLineIntersect3D(const Vector3& p1, const Vector3& _dir1, const Vector3& p2, const Vector3& _dir2, Vector3& dest)
{
    const float epsilon = 1e-6f;
    Vector3 dir1 = _dir1; dir1.normalize();
    Vector3 dir2 = _dir2; dir2.normalize();

    // connecting line 'C' is perpendicular to both
    Vector3 perp; perp.crossProduct(dir1, dir2);

    // check for near-parallel lines
    float dist = perp.dotProduct(perp);

    if (dist < epsilon) {
        // lines parallel - any point on 2nd line will do
        dest = p2;
        return false;
    }

    // form a plane containing the line A and C, and another containing B and C

    // normalize perp
    perp.scale(1.0f/sqrtf(dist));

    Vector3 normal;
    normal.crossProduct(perp, dir1);
    normal.normalize();
    Plane pA(p1, normal);

    normal.crossProduct(perp, dir2);
    normal.normalize();
    Plane pB(p2, normal);

    Vector3 tmpOrg, tmpDir;

    if (!pA.intersectionWithPlane(pB, tmpOrg, tmpDir)) {
        // this shouldnt ever happen because we already tested for parallel lines
        dest = p2;
        return false;
    }

    if (!pA.intersectionWithLine(p2, dir2, dest)) {
        dest = p2;
        return false;
    }
    return true;
}

bool MathUtil::coplanarTriTriIntersect(const Vector3& normal, const Vector3& t1p1, const Vector3& t1p2, const Vector3& t1p3, const Vector3& t2p1, const Vector3& t2p2, const Vector3& t2p3)
{
    float a[3];
    a[0] = fabs(normal[0]);
    a[1] = fabs(normal[1]);
    a[2] = fabs(normal[2]);

    int i0, i1;
    // project onto an axis-aligned plane, that maximizes the area of the triangles, compute indices: i0,i1.
    if (a[0] > a[1]) {
        if (a[0] > a[2]) {
            i0=1;
            i1=2;
        } else {
            i0=0;
            i1=1;
        }
    } else {
        if (a[2] > a[1]) {
            i0=0;
            i1=1;  
        } else {
            i0=0;
            i1=2;
        }
    }

    // test all edges of triangle 1 against the triangle 2
    if (edgeTriIntersect(t1p1, t1p2, t2p1, t2p2, t2p3, i0, i1)) {
        return true;
    }
    if (edgeTriIntersect(t1p2, t1p3, t2p1, t2p2, t2p3, i0, i1)) {
        return true;
    }
    if (edgeTriIntersect(t1p3, t1p1, t2p1, t2p2, t2p3, i0, i1)) {
        return true;
    }


    // test if triangle 1 is totally in triangle 2
    if (pointInTri(t1p1, t2p1, t2p2, t2p3, i0, i1)) {
        return true;
    }
    if (pointInTri(t2p1, t1p1, t1p2, t1p3, i0, i1)) {
        return true;
    }

    return false;
}

bool MathUtil::edgeTriIntersect(const Vector3& edgeStart, const Vector3& edgeEnd, const Vector3& tp1, const Vector3& tp2, const Vector3& tp3, int i0, int i1)
{
    float ax = edgeEnd[i0] - edgeStart[i0];
    float ay = edgeEnd[i1] - edgeStart[i1];

    float bx, by, cx, cy, f, d, e;

    // test start->end with tp1->tp2
    bx = tp1[i0]-tp2[i0];
    by = tp1[i1]-tp2[i1];
    cx = edgeStart[i0]-tp1[i0];
    cy = edgeStart[i1]-tp1[i1];
    f = ay*bx - ax*by;
    d = by*cx - bx*cy;
    if ((f>0 && d>=0 && d<=f) || (f<0 && d<=0 && d>=f)) {
        e = ax*cy-ay*cx;
        if (f>0) {
            if (e>=0 && e<=f) {
                return true;
            }
        } else {
            if (e<=0 && e>=f) {
                return true;
            }
        }
    }

    // test start->end with tp2->tp3
    bx = tp2[i0]-tp3[i0];
    by = tp2[i1]-tp3[i1];
    cx = edgeStart[i0]-tp2[i0];
    cy = edgeStart[i1]-tp2[i1];
    f = ay*bx - ax*by;
    d = by*cx - bx*cy;
    if ((f>0 && d>=0 && d<=f) || (f<0 && d<=0 && d>=f)) {
        e = ax*cy-ay*cx;
        if (f>0) {
            if (e>=0 && e<=f) {
                return true;
            }
        } else {
            if (e<=0 && e>=f) {
                return true;
            }
        }
    }

    // test start->end with tp3->tp1
    bx = tp3[i0]-tp1[i0];
    by = tp3[i1]-tp1[i1];
    cx = edgeStart[i0]-tp3[i0];
    cy = edgeStart[i1]-tp3[i1];
    f = ay*bx - ax*by;
    d = by*cx - bx*cy;
    if ((f>0 && d>=0 && d<=f) || (f<0 && d<=0 && d>=f)) {
        e = ax*cy-ay*cx;
        if (f>0) {
            if (e>=0 && e<=f) {
                return true;
            }
        } else {
            if (e<=0 && e>=f) {
                return true;
            }
        }
    }
    return false;
}
  
bool MathUtil::pointInTri(const Vector3& point, const Vector3& tp1, const Vector3& tp2, const Vector3& tp3, int i0, int i1)
{
    float a,b,c,d0,d1;
    a = tp2[i1]-tp1[i1];
    b = -(tp2[i0]-tp1[i0]);
    c = -a*tp1[i0]-b*tp1[i1];
    d0 = a*point[i0]+b*point[i1]+c;

    a = tp3[i1]-tp2[i1];
    b = -(tp3[i0]-tp2[i0]);
    c = -a*tp2[i0]-b*tp2[i1];
    d1 = a*point[i0]+b*point[i1]+c;

    if (d0*d1 > 0.0f) {

        a = tp1[i1]-tp3[i1];
        b = -(tp1[i0]-tp3[i0]);
        c = -a*tp3[i0]-b*tp3[i1];
        float d2 = a*point[i0]+b*point[i1]+c;
        if (d0*d2 > 0.0f) {
            return true;
        }
    }
    return false;
}

bool MathUtil::rayTriangleIntersect3D(const Vector3& rayOrigin, const Vector3& rayDir, const Vector3& vert0, const Vector3& vert1, const Vector3& vert2, float& distance)
{
    const float LOCAL_EPSILON = 0.000001f;
    const float mGeomEpsilon = 0.0001f;
    
    // Find vectors for two edges sharing vert0
    Vector3 edge1; edge1.sub(vert1, vert0);
    Vector3 edge2; edge2.sub(vert2, vert0);

    // Begin calculating determinant - also used to calculate U parameter
    Vector3 pvec; pvec.crossProduct(rayDir, edge2);

    // If determinant is near zero, ray lies in plane of triangle
    float det = edge1.dotProduct(pvec);
    if(det < LOCAL_EPSILON) return false;

    // Calculate distance from vert0 to ray origin
    Vector3 tvec; tvec.sub(rayOrigin, vert0);

    // Calculate U parameter and test bounds
    float u = tvec.dotProduct(pvec);
    if(u < -mGeomEpsilon || u > det + mGeomEpsilon) return false;

    // Prepare to test V parameter
    Vector3 qvec; qvec.crossProduct(tvec, edge1);

    // Calculate V parameter and test bounds
    float v = rayDir.dotProduct(qvec);
    if(v < -mGeomEpsilon || (u + v) > det + mGeomEpsilon) return false;

    // Calculate t, scale parameters, ray intersects triangle
    distance = edge2.dotProduct(qvec);

    // Det > 0 so we can early exit here
    // Intersection point is valid if distance is positive (else it can just be a face behind the orig point)
    if(distance < 0.0f) return false;
    
    distance/=det;

    return true;
}

void MathUtil::cartesianToPolar(const Vector3& vec, float& theta, float& rho)
{
    Vector3 v(vec);
    v.normalize();

    theta = acos(v[1]);

    // build a normalized 2d vector of the xz component
    float inv2dLength = sqrt(v[0]*v[0] + v[2]*v[2]);
    float x = v[0]*inv2dLength;
    float z = v[2]*inv2dLength;

    if (x >= 0.0f && z >= 0.0f) {
        // 1. quadrant
        rho = asin(x);
    } else if (x < 0.0f && z >= 0.0f) {
        // 2. quadrant
        rho = asin(z) + degreeToRadian(270.0f);
    } else if (x < 0.0f && z < 0.0f) {
        // 3. quadrant
        rho = asin(-x) + degreeToRadian(180.0f);
    } else {
        // 4. quadrant
        rho = asin(-z) + degreeToRadian(90.0f);
    }
}

void MathUtil::quatTransToDualQuaternion(const Quaternion& quat, const Vector3& trans, float dualQuat[2][4])
{
     // non-dual part (just the quaternion)
    dualQuat[0][0]= quat.w;
    dualQuat[0][1]= quat.x;
    dualQuat[0][2]= quat.y;
    dualQuat[0][3]= quat.z;
    // dual part
    dualQuat[1][0] = -0.5f*(trans.v[0]*quat.x + trans.v[1]*quat.y + trans.v[2]*quat.z); // w
    dualQuat[1][1] = 0.5f*( trans.v[0]*quat.w + trans.v[1]*quat.z - trans.v[2]*quat.y); // x
    dualQuat[1][2] = 0.5f*(-trans.v[0]*quat.z + trans.v[1]*quat.w + trans.v[2]*quat.x); // y
    dualQuat[1][3] = 0.5f*( trans.v[0]*quat.y - trans.v[1]*quat.x + trans.v[2]*quat.w); // z
}

#if 0

MathUtil::FASTVECTORTRANSFORM MathUtil::getFastVectorTransform(unsigned int stride)
{
    if (stride == 12) {
        return fastVectorTransform12bytes;
    } else if (stride == 16) {
        return fastVectorTransform16bytes;
    } else if (stride == 32) {
        return fastVectorTransform32bytes;
    } else if (stride == 48) {
        return fastVectorTransform48bytes;
    }
    return NULL;
}

void MathUtil::fastVectorTransform4bytesAligned(float* m, unsigned char* vin, unsigned char* vout, unsigned int num)
{
    static const int vecSize = 2 * sizeof(float)*4;

    // if there are an odd number of vectors, process the first one
    // separately and advance the pointers
    if (num & 0x1) {
        float* in = (float*)vin;
        float* out = (float*)vout;

        out[0]=in[0]*m[0]+in[1]*m[4]+in[2]*m[8]+m[12];
        out[1]=in[0]*m[1]+in[1]*m[5]+in[2]*m[9]+m[13];
        out[2]=in[0]*m[2]+in[1]*m[6]+in[2]*m[10]+m[14];

        vin += sizeof(float)*4;
        vout += sizeof(float)*4;
    }
    num >>= 1; // we process two vectors at a time

    __asm {
        mov        esi, vin
            mov        edi, vout
            mov        ecx, num

            // load columns of matrix into xmm4-7
            mov        edx, m
            movaps    xmm4, [edx]
        movaps    xmm5, [edx+0x10]
        movaps    xmm6, [edx+0x20]
        movaps    xmm7, [edx+0x30]

BT2_START:
        // process x (hiding the prefetches in the delays)
        movss        xmm1, [esi+0x00]
        movss        xmm3, [esi+0x10]
        shufps    xmm1, xmm1, 0x00
            prefetchnta [edi+0x30]
        shufps    xmm3, xmm3, 0x00
            mulps        xmm1, xmm4
            prefetchnta    [esi+0x30]
        mulps        xmm3, xmm4

            // process y
            movss        xmm0, [esi+0x04]
        movss        xmm2, [esi+0x14]
        shufps    xmm0, xmm0, 0x00
            shufps    xmm2, xmm2, 0x00
            mulps        xmm0, xmm5
            mulps        xmm2, xmm5
            addps        xmm1, xmm0
            addps        xmm3, xmm2

            // process z (hiding some pointer arithmetic between
            // the multiplies)
            movss        xmm0, [esi+0x08]
        movss        xmm2, [esi+0x18]
        shufps    xmm0, xmm0, 0x00
            shufps    xmm2, xmm2, 0x00
            mulps        xmm0, xmm6
            add            esi, vecSize
            mulps        xmm2, xmm6
            add            edi, vecSize
            addps        xmm1, xmm0
            addps        xmm3, xmm2

            // process w
            addps        xmm1, xmm7
            addps        xmm3, xmm7

            // write output vectors to memory and loop
            movaps    [edi-0x20], xmm1
            movaps    [edi-0x10], xmm3
            dec            ecx
            jnz            BT2_START
    }
}

/*void MathUtil::fastVectorTransformLoadMatrix(float* m)
{
    __asm {
        mov        edx, m
        movaps    xmm4, [edx]
        movaps    xmm5, [edx+0x10]
        movaps    xmm6, [edx+0x20]
        movaps    xmm7, [edx+0x30]
    }
}

void MathUtil::fastVectorTransformZeroMatrixTranslation()
{
    __asm {
        xorps       xmm7,xmm7
    }
}*/

void MathUtil::fastVectorTransform12bytes(float* m, unsigned char* vin, unsigned char* vout, unsigned int num)
{
#define MATHUTIL_STRIDE 12
#include "MathUtilFastVectorTransform.h"
#undef MATHUTIL_STRIDE
}

void MathUtil::fastVectorTransform16bytes(float* m, unsigned char* vin, unsigned char* vout, unsigned int num)
{
#define MATHUTIL_STRIDE 16
#include "MathUtilFastVectorTransform.h"
#undef MATHUTIL_STRIDE
}

void MathUtil::fastVectorTransform32bytes(float* m, unsigned char* vin, unsigned char* vout, unsigned int num)
{
#define MATHUTIL_STRIDE 32
#include "MathUtilFastVectorTransform.h"
#undef MATHUTIL_STRIDE
}

void MathUtil::fastVectorTransform48bytes(float* m, unsigned char* vin, unsigned char* vout, unsigned int num)
{
#define MATHUTIL_STRIDE 48
#include "MathUtilFastVectorTransform.h"
#undef MATHUTIL_STRIDE
}

#endif

template <class IndexType> void MathUtil::tangentArrayInternal(unsigned int numVertices, float* pVertices, float* pNormals, float* pTexCoords, unsigned int numTriangles, IndexType* pIndices, bool normalizeTangents, float* pTangents)
{
    float* pTan1 = new float[2*numVertices * 3];
    float* pTan2 = pTan1+3*numVertices;
    memset(pTan1, 0, sizeof(float)*2*numVertices * 3);

    for (unsigned int i=0; i<numTriangles; i++) {
        IndexType i1 = *pIndices++;
        IndexType i2 = *pIndices++;
        IndexType i3 = *pIndices++;

        float* v1 = pVertices+3*i1;
        float* v2 = pVertices+3*i2;
        float* v3 = pVertices+3*i3;
        float* w1 = pTexCoords+2*i1;
        float* w2 = pTexCoords+2*i2;
        float* w3 = pTexCoords+2*i3;

        float x1 = v2[0] - v1[0];
        float x2 = v3[0] - v1[0];
        float y1 = v2[1] - v1[1];
        float y2 = v3[1] - v1[1];
        float z1 = v2[2] - v1[2];
        float z2 = v3[2] - v1[2];
        
        float s1 = w2[0] - w1[0];
        float s2 = w3[0] - w1[0];
        float t1 = w2[1] - w1[1];
        float t2 = w3[1] - w1[1];


        // Note: we dont do any divide by zero check, because if this is happening, there is no nice result anyway
        float r = 1.0f / (s1 * t2 - s2 * t1);
        Vector3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
        Vector3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

        pTan1[i1*3+0] += sdir[0]; pTan1[i1*3+1] += sdir[1]; pTan1[i1*3+2] += sdir[2];
        pTan1[i2*3+0] += sdir[0]; pTan1[i2*3+1] += sdir[1]; pTan1[i2*3+2] += sdir[2];
        pTan1[i3*3+0] += sdir[0]; pTan1[i3*3+1] += sdir[1]; pTan1[i3*3+2] += sdir[2];

        pTan2[i1*3+0] += tdir[0]; pTan2[i1*3+1] += tdir[1]; pTan2[i1*3+2] += tdir[2];
        pTan2[i2*3+0] += tdir[0]; pTan2[i2*3+1] += tdir[1]; pTan2[i2*3+2] += tdir[2];
        pTan2[i3*3+0] += tdir[0]; pTan2[i3*3+1] += tdir[1]; pTan2[i3*3+2] += tdir[2];
    }

    for (unsigned int i=0; i < numVertices; i++) {
        Vector3 normal(pNormals+3*i);
        Vector3 tan(pTan1+3*i);
        Vector3 btan(pTan2+3*i);

        Vector3 newtan;
        
        if (normalizeTangents) {
            // Gram-Schmidt orthogonalize
            newtan = normal;
            newtan.scale(normal.dotProduct(tan));
            newtan.sub(tan, newtan); // newtan = (t - n * dot(n, t))
            newtan.normalize();
        } else {
            newtan = tan;
        }

        // Calculate handedness
        Vector3 temp;
        temp.crossProduct(normal, tan);

        float handedness = (temp.dotProduct(btan) < 0.0f) ? -1.0f : 1.0f;

        *pTangents++ = newtan[0];
        *pTangents++ = newtan[1];
        *pTangents++ = newtan[2];
        *pTangents++ = handedness;
    }

    delete[] pTan1;
}
