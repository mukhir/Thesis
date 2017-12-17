/*
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#include "PreCompiled.h"

#include "base/util/DebugUtil.h"
#include "base/math/MathUtil.h"

#include "Matrix4x4.h"


const float Matrix4x4::NEARZERO = (1e-36f);

void Matrix4x4::look(const Vector3& camPosition, const Vector3& direction, const Vector3& up)
{
	/*
zaxis = normal(At - Eye)
xaxis = normal(cross(Up, zaxis))
yaxis = cross(zaxis, xaxis)

 xaxis.x           yaxis.x           zaxis.x          0
 xaxis.y           yaxis.y           zaxis.y          0
 xaxis.z           yaxis.z           zaxis.z          0
-dot(xaxis, eye)  -dot(yaxis, eye)  -dot(zaxis, eye)  l
	*/
/*
    Vector3 dirN(direction);
    dirN.normalize();

    Vector3 xN;
    xN.crossProduct( up, dirN);
    xN.normalize();

    Vector3 upN;
    upN.crossProduct(dirN, xN);

    m[0] = xN.v[0];
    m[1] = upN.v[0];
    m[2] = -dirN.v[0];
    m[3] = 0.0f;

    m[4] = xN.v[1];
    m[5] = upN.v[1];
    m[6] = -dirN.v[1];
    m[7] = 0.0f;

    m[8] = xN.v[2];
    m[9] = upN.v[2];
    m[10]= -dirN.v[2];
    m[11]= 0.0f;

    m[12]= -xN.dotProduct(camPosition);
    m[13]= -upN.dotProduct(camPosition);
    m[14]= dirN.dotProduct(camPosition);
    m[15]= 1.0f;
*/
	
    Vector3 dirN(direction);
    dirN.normalize();

    Vector3 xN;
    xN.crossProduct(dirN, up);
    xN.normalize();

    Vector3 upN;
    upN.crossProduct(xN, dirN);

    m[0] = xN.v[0];
    m[1] = upN.v[0];
    m[2] = -dirN.v[0];
    m[3] = 0.0f;

    m[4] = xN.v[1];
    m[5] = upN.v[1];
    m[6] = -dirN.v[1];
    m[7] = 0.0f;

    m[8] = xN.v[2];
    m[9] = upN.v[2];
    m[10]= -dirN.v[2];
    m[11]= 0.0f;

    m[12]= -xN.dotProduct(camPosition);
    m[13]= -upN.dotProduct(camPosition);
    m[14]= dirN.dotProduct(camPosition);
    m[15]= 1.0f;
	 
}

void Matrix4x4::lookAt(const Vector3& camPosition, const Vector3& center, const Vector3& up)
{
    Vector3 direction(center);
    direction.sub(camPosition);
    look(camPosition, direction, up);
}


void Matrix4x4::getEulerAngles(Vector3& euler, Vector3* pScale) const 
{
    //float matrix[3][3];
    //float cx,sx,x;
    //float cy,sy,y,yr;
    //float cz,sz,z;

///////////////////////////////////////////////////////////////////////////////
    // CONVERT QUATERNION TO MATRIX - I DON'T REALLY NEED ALL OF IT
    //matrix[0][0] = 1.0f - (2.0f * quat->y * quat->y) - (2.0f * quat->z * quat->z);
    //matrix[0][1] = (2.0f * quat->x * quat->y) - (2.0f * quat->w * quat->z);
    //matrix[0][2] = (2.0f * quat->x * quat->z) + (2.0f * quat->w * quat->y);
    //matrix[1][0] = (2.0f * quat->x * quat->y) + (2.0f * quat->w * quat->z);
    //matrix[1][1] = 1.0f - (2.0f * quat->x * quat->x) - (2.0f * quat->z * quat->z);
    //matrix[1][2] = (2.0f * quat->y * quat->z) - (2.0f * quat->w * quat->x);
    //matrix[2][0] = (2.0f * quat->x * quat->z) - (2.0f * quat->w * quat->y);
    //matrix[2][1] = (2.0f * quat->y * quat->z) + (2.0f * quat->w * quat->x);
    //matrix[2][2] = 1.0f - (2.0f * quat->x * quat->x) - (2.0f * quat->y * quat->y);

    /*float sy = -m[2+0*4];
    float cy = sqrtf(1 - (sy * sy));

    float yr = (float)atan2(sy, cy);
    euler.v[1] = yr;

    // Avoid divide by zero error only where y= +-90 or +-270 not checking cy because of precision errors
    if (sy != 1.0f && sy != -1.0f) {
        float cx = m[2+2*4] / cy;
        float sx = m[2+1*4] / cy;
        euler.v[0] = (float)atan2(sx, cx);

        float cz = m[0+0*4] / cy;
        float sz = m[1+0*4] / cy;
        euler.v[2] = (float)atan2(sz, cz);
    } else {
        // Since cos(y) is 0, i am screwed.  adopt the standard z = 0

        //matrix[1][1] = 1.0f - (2.0f * quat->x * quat->x) - (2.0f * quat->z * quat->z);
        //matrix[1][2] = (2.0f * quat->y * quat->z) - (2.0f * quat->w * quat->x);
        
        float cx = m[1+1*4];
        float sx = -m[1+2*4];
        euler.v[0] = (float)atan2(sx, cx);

        float cz = 1.0f;
        float sz = 0.0f;
        euler.v[2] = (float)atan2(sz, cz);
    }*/


    float oneOverXScale = 1.0f;
    float oneOverYScale = 1.0f;
    float oneOverZScale = 1.0f;
    if (pScale != NULL) {
        if (pScale->v[0] != 0.0f) {
            oneOverXScale = 1.0f/pScale->v[0];
        }
        if (pScale->v[1] != 0.0f) {
            oneOverYScale = 1.0f/pScale->v[1];
        }
        if (pScale->v[2] != 0.0f) {
            oneOverZScale = 1.0f/pScale->v[2];
        }
    }


    float sy = -m[2+0*4]*oneOverXScale;
    
    float cy = 0.0f;

    float tmp1 = 1 - (sy * sy);
    const float EPSILON = 0.000001f;

    /*if (tmp1 < -EPSILON) {
        LogManager::getInstance() << "Its negative: \n" << this->toString().c_str() << "\n";
        euler.v[0] = 0;
        euler.v[1] = 0;
        euler.v[2] = 0;
        return;
    }*/

    //if (tmp1 < -EPSILON  || tmp1 > EPSILON) {
    if (tmp1 > EPSILON) {
        cy = sqrt(tmp1);

        float cx = m[2+2*4]*oneOverZScale / cy;
        float sx = m[2+1*4]*oneOverYScale / cy;
        euler.v[0] = (float)atan2(sx, cx);

        float cz = m[0+0*4]*oneOverXScale / cy;
        float sz = m[1+0*4]*oneOverXScale / cy;
        euler.v[2] = (float)atan2(sz, cz);

    } else {
        // Since cos(y) is 0, i am screwed.  adopt the standard z = 0

        //matrix[1][1] = 1.0f - (2.0f * quat->x * quat->x) - (2.0f * quat->z * quat->z);
        //matrix[1][2] = (2.0f * quat->y * quat->z) - (2.0f * quat->w * quat->x);
        
        float cx = m[1+1*4]*oneOverYScale;
        float sx = -m[1+2*4]*oneOverZScale;
        euler.v[0] = (float)atan2(sx, cx);

        float cz = 1.0f;
        float sz = 0.0f;
        euler.v[2] = (float)atan2(sz, cz);
    }

    float yr = (float)atan2(sy, cy);
    euler.v[1] = yr;
 
}

