/** \class Matrix4x4
 * Matrix Class 4x4 in R^3.
 *
 * Column major (x,y) where
 *   00 10 20 30
 *   01 11 21 31
 *   02 12 22 32
 *   03 13 23 33
 * is stored as
 *    0  4  8 12
 *    1  5  9 13
 *    2  6 10 14
 *    3  7 11 15
 * representing
 *   Rot Rot Rot Tx
 *   Rot Rot Rot Ty
 *   Rot Rot Rot Tz
 *    0   0   0   1
 *
 * @author  Christian Ammann
 * @date  18/02/03
 *
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#pragma once

#include "Vector3.h"
#include "Vector4.h"
#include "Quaternion.h"

class Vector4;
class Quaternion;

class Matrix4x4{
    public:
    
    // Allows scale of 1/3 the exponent
    static const float NEARZERO;

    enum AxisID {X_AXIS, Y_AXIS, Z_AXIS};

    // No default clear
    inline Matrix4x4(){}
    inline Matrix4x4(const float *pFloats)
    {
        // 16 floats assumed
        operator=(pFloats);
    }

    inline Matrix4x4(const Matrix4x4 &operand)
    {
        operator=(operand);
    }

    inline Matrix4x4(const Vector3 &x,const Vector3 &y, const Vector3 &z, const Vector3 &t)
    {
        set(x,y,z,t);
    }

    /** 
     * Create new Matrix4x4 object
     *
     * @param rotation  rotation vector.
     * @param scaleVector  scale vector.
     * @param translation  translation vector.
     */
    inline Matrix4x4(const Vector3& rotation, const Vector3& scaleVector, const Vector3& translation)
    {
        identity();
        setTranslation(translation);
        rotate3x4(rotation.v[2], Matrix4x4::Z_AXIS);
        rotate3x4(rotation.v[1], Matrix4x4::Y_AXIS);
        rotate3x4(rotation.v[0], Matrix4x4::X_AXIS);
        scale3x4(scaleVector);
    }

    /** 
     * Create new Matrix4x4 object
     */
    inline Matrix4x4(float m0, float m1, float m2, float m4, float m5, float m6, float m8, float m9, float m10)
    {
        m[0]=m0;
        m[1]=m1;
        m[2]=m2;
        m[3]=0.0f;
        m[4]=m4;
        m[5]=m5;
        m[6]=m6;
        m[7]=0.0f;
        m[8]=m8;
        m[9]=m9;
        m[10]=m10;
        m[11]=0.0f;
        m[12]=0.0f;
        m[13]=0.0f;
        m[14]=0.0f;
        m[15]=1.0f;
    }

    /** 
     * Set to identity matrix
     */
    inline Matrix4x4& identity()
    {
        // identity
        m[0]=1.0;
        m[1]=0.0;
        m[2]=0.0;
        m[3]=0.0;

        m[4]=0.0;
        m[5]=1.0;
        m[6]=0.0;
        m[7]=0.0;

        m[8]=0.0;
        m[9]=0.0;
        m[10]=1.0;
        m[11]=0.0;

        m[12]=0.0;
        m[13]=0.0;
        m[14]=0.0;
        m[15]=1.0;
        return *this;
    }

    inline Matrix4x4& operator=(const float *pFloats)
    {
        // 16 floats assumed
        for(int i=0;i<16;i++){
            m[i]=pFloats[i];
        }

        return *this;
    }

    inline Matrix4x4& copyMatrix(const float ppFloats[4][4])
    {
        for(int i=0;i<4;i++){
            int ij=i*4;
            for(int j=0;j<4;j++){
                m[ij+j]=ppFloats[i][j];
            }
        }
        return *this;
    }

    inline Matrix4x4& operator=(const Matrix4x4 &operand)
    {
        for(int i=0;i<16;i++)
            m[i]=operand.m[i];

        return *this;
    }

    /** 
     * Set matrix
     *
     * @param x  1st column.
     * @param y  2nd column.
     * @param z  3th column.
     * @param t  4th column.
     */
    inline Matrix4x4& set(const Vector3 &x, const Vector3 &y,const Vector3 &z, const Vector3 &t)
    {
        const float *v=x.rawConst();
        m[0 ]=v[0];
        m[1 ]=v[1];
        m[2 ]=v[2];
        m[3 ]=0.0f;

        v=y.rawConst();
        m[4 ]=v[0];
        m[5 ]=v[1];
        m[6 ]=v[2];
        m[7 ]=0.0f;

        v=z.rawConst();
        m[8 ]=v[0];
        m[9 ]=v[1];
        m[10]=v[2];
        m[11]=0.0f;

        v=t.rawConst();
        m[12]=v[0];
        m[13]=v[1];
        m[14]=v[2];
        m[15]=1.0f;

        return *this;
    }

    inline bool operator==(const Matrix4x4 &operand) const
    {
        for(int i=0;i<16;i++){
            if(m[i]!=operand.m[i]){
                return false;
            }
        }
        return true;
    }

    inline float* raw(){ return m; }
    inline const float* rawConst() const { return m; }
    inline float& operator[](int n) { return m[n]; }
    inline const float& operator[](int n) const { return m[n]; }

    inline void setTranslation(const Vector3 &vec) {
        const float *v=vec.rawConst();
        m[12]=v[0];
        m[13]=v[1];
        m[14]=v[2];
    }

    inline Matrix4x4& mult(const Matrix4x4 &left, const Matrix4x4 &right)
    {
        GS_ASSERT(this != &left);
        GS_ASSERT(this != &right);
        // [i][j]=sum [i][k]*[k][j] for all k

        const float *m2=left.rawConst();
        const float *m1=right.rawConst();

        m[ 0]=m1[ 0]*m2[ 0]+m1[ 1]*m2[ 4]+m1[ 2]*m2[ 8]+m1[ 3]*m2[12];
        m[ 1]=m1[ 0]*m2[ 1]+m1[ 1]*m2[ 5]+m1[ 2]*m2[ 9]+m1[ 3]*m2[13];
        m[ 2]=m1[ 0]*m2[ 2]+m1[ 1]*m2[ 6]+m1[ 2]*m2[10]+m1[ 3]*m2[14];
        m[ 3]=m1[ 0]*m2[ 3]+m1[ 1]*m2[ 7]+m1[ 2]*m2[11]+m1[ 3]*m2[15];
        m[ 4]=m1[ 4]*m2[ 0]+m1[ 5]*m2[ 4]+m1[ 6]*m2[ 8]+m1[ 7]*m2[12];
        m[ 5]=m1[ 4]*m2[ 1]+m1[ 5]*m2[ 5]+m1[ 6]*m2[ 9]+m1[ 7]*m2[13];
        m[ 6]=m1[ 4]*m2[ 2]+m1[ 5]*m2[ 6]+m1[ 6]*m2[10]+m1[ 7]*m2[14];
        m[ 7]=m1[ 4]*m2[ 3]+m1[ 5]*m2[ 7]+m1[ 6]*m2[11]+m1[ 7]*m2[15];
        m[ 8]=m1[ 8]*m2[ 0]+m1[ 9]*m2[ 4]+m1[10]*m2[ 8]+m1[11]*m2[12];
        m[ 9]=m1[ 8]*m2[ 1]+m1[ 9]*m2[ 5]+m1[10]*m2[ 9]+m1[11]*m2[13];
        m[10]=m1[ 8]*m2[ 2]+m1[ 9]*m2[ 6]+m1[10]*m2[10]+m1[11]*m2[14];
        m[11]=m1[ 8]*m2[ 3]+m1[ 9]*m2[ 7]+m1[10]*m2[11]+m1[11]*m2[15];
        m[12]=m1[12]*m2[ 0]+m1[13]*m2[ 4]+m1[14]*m2[ 8]+m1[15]*m2[12];
        m[13]=m1[12]*m2[ 1]+m1[13]*m2[ 5]+m1[14]*m2[ 9]+m1[15]*m2[13];
        m[14]=m1[12]*m2[ 2]+m1[13]*m2[ 6]+m1[14]*m2[10]+m1[15]*m2[14];
        m[15]=m1[12]*m2[ 3]+m1[13]*m2[ 7]+m1[14]*m2[11]+m1[15]*m2[15];

        return *this;
    }

    inline Matrix4x4& mult3x4(const Matrix4x4 &left, const Matrix4x4 &right)
    {
        GS_ASSERT(this!=&left);
        GS_ASSERT(this!=&right);
        // [i][j]=sum [i][k]*[k][j] for all k

        const float *m2=left.rawConst();
        const float *m1=right.rawConst();

        // reduced for transform-specific usage
        m[ 0]=m1[ 0]*m2[ 0]+m1[ 1]*m2[ 4]+m1[ 2]*m2[ 8];
        m[ 1]=m1[ 0]*m2[ 1]+m1[ 1]*m2[ 5]+m1[ 2]*m2[ 9];
        m[ 2]=m1[ 0]*m2[ 2]+m1[ 1]*m2[ 6]+m1[ 2]*m2[10];
        m[ 3]=0.0f;

        m[ 4]=m1[ 4]*m2[ 0]+m1[ 5]*m2[ 4]+m1[ 6]*m2[ 8];
        m[ 5]=m1[ 4]*m2[ 1]+m1[ 5]*m2[ 5]+m1[ 6]*m2[ 9];
        m[ 6]=m1[ 4]*m2[ 2]+m1[ 5]*m2[ 6]+m1[ 6]*m2[10];
        m[ 7]=0.0f;

        m[ 8]=m1[ 8]*m2[ 0]+m1[ 9]*m2[ 4]+m1[10]*m2[ 8];
        m[ 9]=m1[ 8]*m2[ 1]+m1[ 9]*m2[ 5]+m1[10]*m2[ 9];
        m[10]=m1[ 8]*m2[ 2]+m1[ 9]*m2[ 6]+m1[10]*m2[10];
        m[11]=0.0f;

        m[12]=m1[12]*m2[ 0]+m1[13]*m2[ 4]+m1[14]*m2[ 8] + m2[12];
        m[13]=m1[12]*m2[ 1]+m1[13]*m2[ 5]+m1[14]*m2[ 9] + m2[13];
        m[14]=m1[12]*m2[ 2]+m1[13]*m2[ 6]+m1[14]*m2[10] + m2[14];
        m[15]=1.0f;

        return *this;
    }

    inline void copyTranslation(Vector3& vector) const
    {
        vector.set(m[12],m[13],m[14]);
    }

    inline void copyRotationAndTranslation(Matrix4x4& dest) const
    {
        Vector3 unitX(1,0,0), unitY(0,1,0), unitZ(0,0,1), unitXt, unitYt, unitZt;
        rotateVector(unitX, unitXt);
        rotateVector(unitY, unitYt);
        rotateVector(unitZ, unitZt);
        unitXt.normalize();
        unitYt.normalize();
        unitZt.normalize();
        Vector3 translation;
        copyTranslation(translation);
        dest.set(unitXt, unitYt, unitZt, translation);
    }

    // Note: Not working with negative scale
    inline void copyScale(Vector3& scaleVec) const
    {
        Vector3 unitX(1,0,0), unitY(0,1,0), unitZ(0,0,1), unitXt, unitYt, unitZt;
        rotateVector(unitX, unitXt);
        rotateVector(unitY, unitYt);
        rotateVector(unitZ, unitZt);
        
        scaleVec.set(unitXt.calcMagnitude(), unitYt.calcMagnitude(), unitZt.calcMagnitude());

        const float EPSILON = 0.00001f;

        if (scaleVec.v[0] < 1.0f+EPSILON && scaleVec.v[0] > 1.0f-EPSILON) {
            scaleVec.v[0] = 1.0f;
        }
        if (scaleVec.v[1] < 1.0f+EPSILON && scaleVec.v[1] > 1.0f-EPSILON) {
            scaleVec.v[1] = 1.0f;
        }
        if (scaleVec.v[2] < 1.0f+EPSILON && scaleVec.v[2] > 1.0f-EPSILON) {
            scaleVec.v[2] = 1.0f;
        }

    }

    inline void copyColumn(unsigned int column, Vector3& dest) const
    {
        unsigned int baseIndex = 4*column;
        dest.v[0] = m[baseIndex];
        dest.v[1] = m[baseIndex+1];
        dest.v[2] = m[baseIndex+2];
    }

    inline Matrix4x4& translate3x4(const Vector3 &vector)
    {
        Vector3 result;
        const float *v=result.rawConst();

        //* equivalent when you do the math
        rotateVector(vector,result);

        //* assuming 3x4
        m[12]+=v[0];
        m[13]+=v[1];
        m[14]+=v[2];

        return *this;
    }

    inline Matrix4x4& rotate3x4(float radians, AxisID axis)
    {
        float sina = sinf(radians);
        float cosa = cosf(radians);

        // unrolled
        static const int index[3][2]={{4,8},{8,0},{0,4}};

        unsigned int x1=index[axis][0];
        unsigned int x2=index[axis][1];

        float b=m[x1];
        float c=m[x2];
        m[x1++]=c*sina+b*cosa;
        m[x2++]=c*cosa-b*sina;

        b=m[x1];
        c=m[x2];
        m[x1++]=c*sina+b*cosa;
        m[x2++]=c*cosa-b*sina;

        b=m[x1];
        c=m[x2];
        m[x1++]=c*sina+b*cosa;
        m[x2++]=c*cosa-b*sina;

        b=m[x1];
        c=m[x2];
        m[x1++]=c*sina+b*cosa;
        m[x2++]=c*cosa-b*sina;

        return *this;
    }

    /**
     * Generate rotation around arbitrary axis
     * adapted from Computer Graphics (Hearn and Bker 2nd ed.) p. 420
     */
    inline Matrix4x4& setRotation(float radians, const Vector3 &axis)
    {
        float t = axis.calcMagnitudeSquared();
        if ( t == 0.0f ) {
            return *this;
        }

        t = sinf(radians*0.5f) * MathUtil::invSqrt(t);

        // Fill in the entries of the quaternion.
        float a = cosf(radians*0.5f);
        float b = t * axis.v[0];
        float c = t * axis.v[1];
        float d = t * axis.v[2];

        // Compute all the double products of a, b, c, and d, except a * a.
        float bb = b * b;
        float cc = c * c;
        float dd = d * d;
        float ab = a * b;
        float ac = a * c;
        float ad = a * d;
        float bc = b * c;
        float bd = b * d;
        float cd = c * d;

        m[0] = 1.0f - 2.0f * ( cc + dd );
        m[4] =       2.0f * ( bc + ad );
        m[8] =       2.0f * ( bd - ac );

        m[1] =       2.0f * ( bc - ad );
        m[5] = 1.0f - 2.0f * ( bb + dd );
        m[9] =       2.0f * ( cd + ab );

        m[2] =       2.0f * ( bd + ac );
        m[6] =       2.0f * ( cd - ab );
        m[10] = 1.0f - 2.0f * ( bb + cc );

        return *this;
    }

    inline Matrix4x4& setRotation(const Quaternion& quaternion)
    {
        // Compute all the double products of a, b, c, and d, except a * a.
        float bb = quaternion.x * quaternion.x;
        float cc = quaternion.y * quaternion.y;
        float dd = quaternion.z * quaternion.z;
        float ab = quaternion.w * quaternion.x;
        float ac = quaternion.w * quaternion.y;
        float ad = quaternion.w * quaternion.z;
        float bc = quaternion.x * quaternion.y;
        float bd = quaternion.x * quaternion.z;
        float cd = quaternion.y * quaternion.z;

        m[0] = 1.0f - 2.0f * ( cc + dd );
        m[1] =        2.0f * ( bc + ad );
        m[2] =        2.0f * ( bd - ac );

        m[4] =        2.0f * ( bc - ad );
        m[5] = 1.0f - 2.0f * ( bb + dd );
        m[6] =        2.0f * ( cd + ab );

        m[8] =         2.0f * ( bd + ac );
        m[9] =         2.0f * ( cd - ab );
        m[10] = 1.0f - 2.0f * ( bb + cc );

        return *this;
    }

    inline Matrix4x4& transpose()
    {
        #define swap(a,b) { float temp = m[b]; m[b] = m[a]; m[a] = temp; }

        swap(1, 4);
        swap(2, 8);
        swap(3, 12);

        swap(6, 9);
        swap(7, 13);
        swap(11, 14);

        #undef swap

        return *this;
    }

    inline Matrix4x4& add(const Matrix4x4 &operand)
    {
        const float *m2=operand.rawConst();

        m[0]+=m2[0];
        m[1]+=m2[1];
        m[2]+=m2[2];
        m[3]+=m2[3];

        m[4]+=m2[4];
        m[5]+=m2[5];
        m[6]+=m2[6];
        m[7]+=m2[7];

        m[8]+=m2[8];
        m[9]+=m2[9];
        m[10]+=m2[10];
        m[11]+=m2[11];

        m[12]+=m2[12];
        m[13]+=m2[13];
        m[14]+=m2[14];
        m[15]+=m2[15];

        return *this;
    }

    inline Matrix4x4& scale(const Vector3& vec)
    {
        const float *v=vec.rawConst();

        m[0]*=v[0];
        m[1]*=v[0];
        m[2]*=v[0];
        m[3]*=v[0];

        m[4]*=v[1];
        m[5]*=v[1];
        m[6]*=v[1];
        m[7]*=v[1];

        m[8]*=v[2];
        m[9]*=v[2];
        m[10]*=v[2];
        m[11]*=v[2];

        return *this;
    }

    inline Matrix4x4& scale3x4(const Vector3& vec)
    {
        const float *v=vec.rawConst();

        m[0]*=v[0];
        m[1]*=v[0];
        m[2]*=v[0];

        m[4]*=v[1];
        m[5]*=v[1];
        m[6]*=v[1];

        m[8]*=v[2];
        m[9]*=v[2];
        m[10]*=v[2];

        return *this;
    }

    /**
     * limited 3x3+3 3-component multiplication, not 4x4 full multiplication
     */
    inline void transformVector(const Vector3& source, Vector3& result) const
    {
        GS_ASSERT(&source!=&result);
        
        const float *in=source.rawConst();
        float *out=result.raw();

        // unrolled
        out[0]=in[0]*m[0]+in[1]*m[4]+in[2]*m[8]+m[12];
        out[1]=in[0]*m[1]+in[1]*m[5]+in[2]*m[9]+m[13];
        out[2]=in[0]*m[2]+in[1]*m[6]+in[2]*m[10]+m[14];
    }

    /**
     * limited 3x3+3 3-component multiplication, not 4x4 full multiplication
     */
    inline void transformVector(const Vector4& source, Vector4& result) const
    {
        GS_ASSERT(&source!=&result);
        
        const float *in=source.rawConst();
        float *out=result.raw();

        // unrolled
        out[0]=in[0]*m[0]+in[1]*m[4]+in[2]*m[8]+m[12];
        out[1]=in[0]*m[1]+in[1]*m[5]+in[2]*m[9]+m[13];
        out[2]=in[0]*m[2]+in[1]*m[6]+in[2]*m[10]+m[14];
    }

	/**
     * 4x4 full multiplication
     */
    inline void transformVector4x4(const Vector4& source, Vector4& result) const
    {
        GS_ASSERT(&source!=&result);
        
        const float *in=source.rawConst();
        float *out=result.raw();

        // unrolled
        out[0]=in[0]*m[0]+in[1]*m[4]+in[2]*m[8]+in[3]*m[12];
        out[1]=in[0]*m[1]+in[1]*m[5]+in[2]*m[9]+in[3]*m[13];
        out[2]=in[0]*m[2]+in[1]*m[6]+in[2]*m[10]+in[3]*m[14];
        out[3]=in[0]*m[3]+in[1]*m[7]+in[2]*m[11]+in[3]*m[15];
    }

    /**
     * limited 3x3+3 3-component multiplication, not 4x4 full multiplication
     * Divides by transformed w
     */
    inline void projectVector(const Vector3& source, Vector3& result) const
    {
        const float *in=source.rawConst();
        float *out=result.raw();

        // unrolled - 
        out[0]=in[0]*m[0]+in[1]*m[4]+in[2]*m[8]+m[12];
        out[1]=in[0]*m[1]+in[1]*m[5]+in[2]*m[9]+m[13];
        out[2]=in[0]*m[2]+in[1]*m[6]+in[2]*m[10]+m[14];
        float w=1.0f/(in[0]*m[3]+in[1]*m[7]+in[2]*m[11]+m[15]);

        out[0]*=w;
        out[1]*=w;
        out[2]*=w;

    }

    inline void rotateVector(const Vector3 &source, Vector3 &result) const
    {
        const float *in=source.rawConst();
        float *out=result.raw();

        // unrolled
        out[0]=in[0]*m[0]+in[1]*m[4]+in[2]*m[8];
        out[1]=in[0]*m[1]+in[1]*m[5]+in[2]*m[9];
        out[2]=in[0]*m[2]+in[1]*m[6]+in[2]*m[10];
    }

    /**
     * Invert matrix (limited to 3x4)
     */
    inline bool invert3x4(const Matrix4x4 &operand)
    {
        GS_ASSERT(this!=&operand);
        float detVal = operand.calcDeterminant3x3();

        const float* src = operand.rawConst();
        float* dst = raw();

        if (fabsf(detVal) > NEARZERO) {
            float detInv = 1.0f / detVal;

            dst[0]  =  detInv*(src[5]*src[10] - src[6]*src[9]);
            dst[1]  = -detInv*(src[1]*src[10] - src[2]*src[9]);
            dst[2]  =  detInv*(src[1]*src[6]  - src[2]*src[5]);
            dst[3]  = 0;
            dst[4]  = -detInv*(src[4]*src[10] - src[6]*src[8]);
            dst[5]  =  detInv*(src[0]*src[10] - src[2]*src[8]);
            dst[6]  = -detInv*(src[0]*src[6]  - src[2]*src[4]);
            dst[7]  = 0;
            dst[8]  =  detInv*(src[4]*src[9]  - src[5]*src[8]);
            dst[9]  = -detInv*(src[0]*src[9]  - src[1]*src[8]);
            dst[10] =  detInv*(src[0]*src[5]  - src[1]*src[4]);
            dst[11] = 0;
            dst[12] = -(src[12]*dst[0]+src[13]*dst[4]+src[14]*dst[8]);
            dst[13] = -(src[12]*dst[1]+src[13]*dst[5]+src[14]*dst[9]);
            dst[14] = -(src[12]*dst[2]+src[13]*dst[6]+src[14]*dst[10]);
            dst[15] = 1;
            return true;
        }

        return false;
    }

    /**
     * Invert matrix
     */
    inline bool invert(const Matrix4x4 &operand)
    {
        // Invert 4x4 matrix using cramers rule
        float tmp[12]; // temp array for pairs
        float src[16]; // array of transpose source matrix
        // transpose matrix
        for (unsigned int i = 0; i < 4; i++) {
            src[i] = operand[i*4];
            src[i + 4] = operand[i*4 + 1];
            src[i + 8] = operand[i*4 + 2];
            src[i + 12] = operand[i*4 + 3];
        }
        // calculate pairs for first 8 elements (cofactors)
        tmp[0] = src[10] * src[15];
        tmp[1] = src[11] * src[14];
        tmp[2] = src[9] * src[15];
        tmp[3] = src[11] * src[13];
        tmp[4] = src[9] * src[14];
        tmp[5] = src[10] * src[13];
        tmp[6] = src[8] * src[15];
        tmp[7] = src[11] * src[12];
        tmp[8] = src[8] * src[14];
        tmp[9] = src[10] * src[12];
        tmp[10] = src[8] * src[13];
        tmp[11] = src[9] * src[12];
        // calculate first 8 elements (cofactors)
        m[0] = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
        m[0] -= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
        m[1] = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
        m[1] -= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
        m[2] = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
        m[2] -= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
        m[3] = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
        m[3] -= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];
        m[4] = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3];
        m[4] -= tmp[0]*src[1] + tmp[3]*src[2] + tmp[4]*src[3];
        m[5] = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3];
        m[5] -= tmp[1]*src[0] + tmp[6]*src[2] + tmp[9]*src[3];
        m[6] = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3];
        m[6] -= tmp[2]*src[0] + tmp[7]*src[1] + tmp[10]*src[3];
        m[7] = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2];
        m[7] -= tmp[5]*src[0] + tmp[8]*src[1] + tmp[11]*src[2];
        // calculate pairs for second 8 elements (cofactors)
        tmp[0] = src[2]*src[7];
        tmp[1] = src[3]*src[6];
        tmp[2] = src[1]*src[7];
        tmp[3] = src[3]*src[5];
        tmp[4] = src[1]*src[6];
        tmp[5] = src[2]*src[5];
        tmp[6] = src[0]*src[7];
        tmp[7] = src[3]*src[4];
        tmp[8] = src[0]*src[6];
        tmp[9] = src[2]*src[4];
        tmp[10] = src[0]*src[5];
        tmp[11] = src[1]*src[4];
        // calculate second 8 elements (cofactors)
        m[8] = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15];
        m[8] -= tmp[1]*src[13] + tmp[2]*src[14] + tmp[5]*src[15];
        m[9] = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15];
        m[9] -= tmp[0]*src[12] + tmp[7]*src[14] + tmp[8]*src[15];
        m[10] = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15];
        m[10]-= tmp[3]*src[12] + tmp[6]*src[13] + tmp[11]*src[15];
        m[11] = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14];
        m[11]-= tmp[4]*src[12] + tmp[9]*src[13] + tmp[10]*src[14];
        m[12] = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9];
        m[12]-= tmp[4]*src[11] + tmp[0]*src[9] + tmp[3]*src[10];
        m[13] = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10];
        m[13]-= tmp[6]*src[10] + tmp[9]*src[11] + tmp[1]*src[8];
        m[14] = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8];
        m[14]-= tmp[10]*src[11] + tmp[2]*src[8] + tmp[7]*src[9];
        m[15] = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9];
        m[15]-= tmp[8]*src[9] + tmp[11]*src[10] + tmp[5]*src[8];
        // calculate determinant
        float determinant = src[0]*m[0]+src[1]*m[1]+src[2]*m[2]+src[3]*m[3];
        // calculate matrix inverse
        if(fabs(determinant) < NEARZERO) {
            return false;
        }
        determinant = 1.0f/determinant;
        for (unsigned int j = 0; j < 16; j++) {
            m[j] *= determinant;
        }
        return true;

        /*
        // simd optimized version (210 cycles vs 846 c implementation)
        float* src = operand.m;
        __m128 minor0, minor1, minor2, minor3; // store cofactors
        __m128 row0, row1, row2, row3; // store rows of operand
        __m128 det, tmp1;

        // transpose matrix
        tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src)), (__m64*)(src+ 4));
        row1 = _mm_loadh_pi(_mm_loadl_pi(row1, (__m64*)(src+8)), (__m64*)(src+12));
        row0 = _mm_shuffle_ps(tmp1, row1, 0x88);
        row1 = _mm_shuffle_ps(row1, tmp1, 0xDD);
        tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src+ 2)), (__m64*)(src+ 6));
        row3 = _mm_loadh_pi(_mm_loadl_pi(row3, (__m64*)(src+10)), (__m64*)(src+14));
        row2 = _mm_shuffle_ps(tmp1, row3, 0x88);
        row3 = _mm_shuffle_ps(row3, tmp1, 0xDD);
        // calculate cofactors
        // -----------------------------------------------
        tmp1 = _mm_mul_ps(row2, row3);
        tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
        minor0 = _mm_mul_ps(row1, tmp1);
        minor1 = _mm_mul_ps(row0, tmp1);
        tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
        minor0 = _mm_sub_ps(_mm_mul_ps(row1, tmp1), minor0);
        minor1 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor1);
        minor1 = _mm_shuffle_ps(minor1, minor1, 0x4E);
        // -----------------------------------------------
        tmp1 = _mm_mul_ps(row1, row2);
        tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
        minor0 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor0);
        minor3 = _mm_mul_ps(row0, tmp1);
        tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
        minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row3, tmp1));
        minor3 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor3);
        minor3 = _mm_shuffle_ps(minor3, minor3, 0x4E);
        // -----------------------------------------------
        tmp1 = _mm_mul_ps(_mm_shuffle_ps(row1, row1, 0x4E), row3);
        tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
        row2 = _mm_shuffle_ps(row2, row2, 0x4E);
        minor0 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor0);
        minor2 = _mm_mul_ps(row0, tmp1);
        tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
        minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row2, tmp1));
        minor2 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor2);
        minor2 = _mm_shuffle_ps(minor2, minor2, 0x4E);
        // -----------------------------------------------
        tmp1 = _mm_mul_ps(row0, row1);
        
        tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
        minor2 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor2);
        minor3 = _mm_sub_ps(_mm_mul_ps(row2, tmp1), minor3);
        tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
        minor2 = _mm_sub_ps(_mm_mul_ps(row3, tmp1), minor2);
        minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row2, tmp1));
        // -----------------------------------------------
        tmp1 = _mm_mul_ps(row0, row3);
        tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
        minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row2, tmp1));
        minor2 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor2);
        tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
        minor1 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor1);
        minor2 = _mm_sub_ps(minor2, _mm_mul_ps(row1, tmp1));
        // -----------------------------------------------
        tmp1 = _mm_mul_ps(row0, row2);
        tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
        minor1 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor1);
        minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row1, tmp1));
        tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
        minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row3, tmp1));
        minor3 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor3);
        // evaluate 1/det, using rcp's and newton-raphson
        // -----------------------------------------------
        det = _mm_mul_ps(row0, minor0);
        det = _mm_add_ps(_mm_shuffle_ps(det, det, 0x4E), det);
        det = _mm_add_ss(_mm_shuffle_ps(det, det, 0xB1), det);
        tmp1 = _mm_rcp_ss(det);
        det = _mm_sub_ss(_mm_add_ss(tmp1, tmp1), _mm_mul_ss(det, _mm_mul_ss(tmp1, tmp1)));
        det = _mm_shuffle_ps(det, det, 0x00);

        // store result (inplace)
        // -----------------------------------------------
    #if 0
        minor0 = _mm_mul_ps(det, minor0);
        _mm_storel_pi((__m64*)(src), minor0);
        _mm_storeh_pi((__m64*)(src+2), minor0);
        minor1 = _mm_mul_ps(det, minor1);
        _mm_storel_pi((__m64*)(src+4), minor1);
        _mm_storeh_pi((__m64*)(src+6), minor1);
        minor2 = _mm_mul_ps(det, minor2);
        _mm_storel_pi((__m64*)(src+ 8), minor2);
        _mm_storeh_pi((__m64*)(src+10), minor2);
        minor3 = _mm_mul_ps(det, minor3);
        _mm_storel_pi((__m64*)(src+12), minor3);
        _mm_storeh_pi((__m64*)(src+14), minor3);
    #else
        // store result (not inplace)
        // -----------------------------------------------
        minor0 = _mm_mul_ps(det, minor0);
        _mm_storel_pi((__m64*)(m), minor0);
        _mm_storeh_pi((__m64*)(m+2), minor0);

        minor1 = _mm_mul_ps(det, minor1);
        _mm_storel_pi((__m64*)(m+4), minor1);
        _mm_storeh_pi((__m64*)(m+6), minor1);

        minor2 = _mm_mul_ps(det, minor2);
        _mm_storel_pi((__m64*)(m+ 8), minor2);
        _mm_storeh_pi((__m64*)(m+10), minor2);

        minor3 = _mm_mul_ps(det, minor3);
        _mm_storel_pi((__m64*)(m+12), minor3);
        _mm_storeh_pi((__m64*)(m+14), minor3);
    #endif
        */

        /*const float* src = operand.rawConst();
        float* dst = raw();

        // Inverse sure is alittle costsome. But i feel it's better to do it right than to use all thoose ugly hacks.
        double detVal = src[0]*src[5]*src[10] - src[0]*src[9]*src[6] - src[4]*src[1]*src[10] + src[4]*src[9]*src[2]
                  + src[8]*src[1]*src[6] - src[8]*src[5]*src[2];

        // If Det == 0.0 then the matrix cannot be inversed. In this case we'll get an ugly matrix with infinate terms
        if(fabs(detVal) > NEARZERO){

            double InvDet = 1.0f/detVal;
            
            double TermA,TermB,TermC,TermD,TermE,TermF;

            TermA = src[5]*src[10];
            TermB = src[9]*src[6];
            TermC = src[4]*src[10];
            TermD = src[8]*src[6];
            TermE = src[4]*src[9];
            TermF = src[8]*src[5];

            dst[0] = (float)((TermA - TermB)*InvDet);
            dst[4] = (float)(-(TermC - TermD)*InvDet);
            dst[8] = (float)((TermE - TermF)*InvDet);
            dst[12] = (float)(-(TermE*src[14] - TermC*src[13] - TermF*src[14] + TermD*src[13] + TermA*src[12] - TermB*src[12])*InvDet);

            TermA = src[1]*src[10];
            TermB = src[9]*src[2];
            TermC = src[0]*src[10];
            TermD = src[8]*src[2];
            TermE = src[0]*src[9];
            TermF = src[8]*src[1];

            dst[1] = (float)(-(TermA - TermB)*InvDet);
            dst[5] = (float)((TermC - TermD)*InvDet);
            dst[9] = (float)(-(TermE - TermF)*InvDet);
            dst[13] = (float)((TermE*src[14] - TermC*src[13] - TermF*src[14] + TermD*src[13] + TermA*src[12] - TermB*src[12])*InvDet);

            TermA = src[1]*src[6];
            TermB = src[5]*src[2];
            TermC = src[0]*src[6];
            TermD = src[4]*src[2];
            TermE = src[0]*src[5];
            TermF = src[4]*src[1];

            dst[2] = (float)((TermA - TermB)*InvDet);
            dst[6] = (float)(-(TermC - TermD)*InvDet);
            dst[10] = (float)((TermE - TermF)*InvDet);
            dst[14] = (float)(-(TermE*src[14] - TermC*src[13] - TermF*src[14] + TermD*src[13] + TermA*src[12] - TermB*src[12])*InvDet);
            
            // ???
            dst[3] = dst[7] = dst[11] = 0;
            dst[15] = 1;
        
            return true;
        } 

        return false;*/
    }

    // Algorithm uses Gram-Schmidt orthogonalization.  If 'this' matrix is
    // M = [m0|m1|m2], then orthonormal output matrix is Q = [q0|q1|q2],
    //
    //   q0 = m0/|m0|
    //   q1 = (m1-(q0*m1)q0)/|m1-(q0*m1)q0|
    //   q2 = (m2-(q0*m2)q0-(q1*m2)q1)/|m2-(q0*m2)q0-(q1*m2)q1|
    //
    // where |V| indicates length of vector V and A*B indicates dot
    // product of vectors A and B.
    inline Matrix4x4& orthonormalize3x3()
    {
        // compute q0
        float fInvLength = MathUtil::invSqrt(m[0+0*4]*m[0+0*4] + m[1+0*4]*m[1+0*4] + m[2+0*4]*m[2+0*4]);

        m[0+0*4] *= fInvLength;
        m[1+0*4] *= fInvLength;
        m[2+0*4] *= fInvLength;

        // compute q1
        float fDot0 =
            m[0+0*4]*m[0+1*4] +
            m[1+0*4]*m[1+1*4] +
            m[2+0*4]*m[2+1*4];

        m[0+1*4] -= fDot0*m[0+0*4];
        m[1+1*4] -= fDot0*m[1+0*4];
        m[2+1*4] -= fDot0*m[2+0*4];

        fInvLength = MathUtil::invSqrt(m[0+1*4]*m[0+1*4] +
            m[1+1*4]*m[1+1*4] +
            m[2+1*4]*m[2+1*4]);

        m[0+1*4] *= fInvLength;
        m[1+1*4] *= fInvLength;
        m[2+1*4] *= fInvLength;

        // compute q2
        float fDot1 =
            m[0+1*4]*m[0+2*4] +
            m[1+1*4]*m[1+2*4] +
            m[2+1*4]*m[2+2*4];

        fDot0 =
            m[0+0*4]*m[0+2*4] +
            m[1+0*4]*m[1+2*4] +
            m[2+0*4]*m[2+2*4];

        m[0+2*4] -= fDot0*m[0+0*4] + fDot1*m[0+1*4];
        m[1+2*4] -= fDot0*m[1+0*4] + fDot1*m[1+1*4];
        m[2+2*4] -= fDot0*m[2+0*4] + fDot1*m[2+1*4];

        fInvLength = MathUtil::invSqrt(m[0+2*4]*m[0+2*4] +
            m[1+2*4]*m[1+2*4] +
            m[2+2*4]*m[2+2*4]);

        m[0+2*4] *= fInvLength;
        m[1+2*4] *= fInvLength;
        m[2+2*4] *= fInvLength;
      
        return *this;
    }



    // Find the determinant the affine 3x3 transform (translation does not contribute)
    inline float calcDeterminant3x3() const
    {
        float detVal;
        const float* src = rawConst();

        float d1, d2, d3;
        d1 = src[5]*src[10] - src[6]*src[9];
        d2 = src[1]*src[10] - src[2]*src[9];
        d3 = src[1]*src[6]  - src[2]*src[5];
        detVal = src[0]*d1 - src[4]*d2 + src[8]*d3;

        return detVal;
    }
    
    // Round near-integer
    inline void applyThreshold(float threshold)
    {
        for(int i=0;i<16;i++){
            int ivalue=int((m[i]<0.0f)? m[i]-0.5f: m[i]+0.5f);

            if(fabs(m[i]-(float)ivalue) < threshold){
                m[i]=(float)ivalue;
            }
        }
    }

    inline void setDirection(const Vector3& direction, const Vector3& up)
    {
        Vector3 dirN(direction);
        dirN.normalize();

        Vector3 xN;
        xN.crossProduct(up, dirN);
        xN.normalize();

        Vector3 upN;
        upN.crossProduct(dirN, xN);

        m[0] = xN.v[0];
        m[1] = xN.v[1];
        m[2] = xN.v[2];

        m[4] = upN.v[0];
        m[5] = upN.v[1];
        m[6] = upN.v[2];

        m[8] = dirN.v[0];
        m[9] = dirN.v[1];
        m[10]= dirN.v[2];
    }

    inline static float det(float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33)
    {
        return m11 * (m22 * m33 - m32 * m23) - m21 * (m12 * m33 - m32 * m13) +  m31 * (m12 * m23 - m22 * m13);
    }

    void look(const Vector3& camPosition, const Vector3& direction, const Vector3& up);
    void lookAt(const Vector3& camPosition, const Vector3& center, const Vector3& up);
    void getEulerAngles(Vector3& euler, Vector3* pScale=NULL) const;

    float m[16];
};


