/**
 * Math util fast vector transforms
 *
 * @author  Stefan Geiger
 * @date  09/11/09
 *
 * Copyright (c) 2008-2011 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2011 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

//#define MATHUTIL_STRIDE 0x18 // sizeof(float)*6

//void fastVectorTransform6bytes(float* m, unsigned char* vin, unsigned char* vout, unsigned int num)
{
#if 0
    // simple sse batch processing  (22 cycles/vertex (with 100 vertices)
    __asm {


        mov			esi, vin
            mov			edi, vout
            mov			ecx, num

            // load columns of matrix into xmm4-7
            mov		edx, m
            movups	xmm4, [edx]
        movups	xmm5, [edx+0x10]
        movups	xmm6, [edx+0x20]
        movups	xmm7, [edx+0x30]

BM3_START:
        // load the next input vector into xmm0, and advance the input
        // and output pointers
        movups	xmm0, [esi]
        add			edi, MATHUTIL_STRIDE

            // broadcast y into xmm1, z into xmm2, and w into xmm3 (leaving
            // x in xmm0).
            movups	xmm1, xmm0
            add			esi, MATHUTIL_STRIDE
            movups	xmm2, xmm0
            //movups	xmm3, xmm0
            // instead load ones to xmm3
            //mov		edx, ones
            //movups xmm3, [edx]


            shufps	xmm0, xmm0, 0x00
            shufps	xmm1, xmm1, 0x55
            shufps	xmm2, xmm2, 0xAA
            //shufps	xmm3, xmm3, 0xFF

            // multiply xmm0-3 by the appropriate columns of the matrix
            mulps		xmm0, xmm4
            mulps		xmm1, xmm5
            mulps		xmm2, xmm6
            //mulps		xmm3, xmm7

            // sum the results into xmm1
            addps		xmm1, xmm0
            //addps		xmm2, xmm3
            addps		xmm2, xmm7 // instead add directly the 4th column of m
            addps		xmm1, xmm2

            // write the results to vout, and loop
            //movups	[edi-0x10], xmm1
            movss	[edi-MATHUTIL_STRIDE], xmm1
            shufps xmm1, xmm1, 0x39 // shift towards the least significant bit
            movss	[edi-(MATHUTIL_STRIDE-0x4)], xmm1
            shufps xmm1, xmm1, 0x39 // shift towards the least significant bit
            movss	[edi-(MATHUTIL_STRIDE-0x8)], xmm1

            dec			ecx
            jnz			BM3_START
    }

#elif 0

    // prefetching, and parallel processing of 2 vertices (16 cycles/vertex (with 100 vertices)

    if (num & 0x1) {
        float* in = (float*)vin;
        float* out = (float*)vout;
        // unrolled
        out[0]=in[0]*m[0]+in[1]*m[4]+in[2]*m[8]+m[12];
        out[1]=in[0]*m[1]+in[1]*m[5]+in[2]*m[9]+m[13];
        out[2]=in[0]*m[2]+in[1]*m[6]+in[2]*m[10]+m[14];

        vin += MATHUTIL_STRIDE;
        vout += MATHUTIL_STRIDE;
    }

    num >>= 1; // we process two vectors at a time

    __asm {
        mov		esi, vin
            mov		edi, vout
            mov		ecx, num

            // load columns of matrix into xmm4-7
            mov		edx, m
            movups	xmm4, [edx]
        movups	xmm5, [edx+0x10]
        movups	xmm6, [edx+0x20]
        movups	xmm7, [edx+0x30]

BT2_START:
        // process x (hiding the prefetches in the delays)
        movss		xmm1, [esi+0x00]
        movss		xmm3, [esi+MATHUTIL_STRIDE] //0x10]
        shufps	xmm1, xmm1, 0x00
            prefetchnta [edi+MATHUTIL_STRIDE*3] //0x30]
        shufps	xmm3, xmm3, 0x00
            mulps		xmm1, xmm4
            prefetchnta	[esi+MATHUTIL_STRIDE*3] //0x30]
        mulps		xmm3, xmm4

            // process y
            movss		xmm0, [esi+0x04]
        movss		xmm2, [esi+MATHUTIL_STRIDE+0x4] //0x14]
        shufps	xmm0, xmm0, 0x00
            shufps	xmm2, xmm2, 0x00
            mulps		xmm0, xmm5
            mulps		xmm2, xmm5
            addps		xmm1, xmm0
            addps		xmm3, xmm2

            // process z (hiding some pointer arithmetic between
            // the multiplies)
            movss		xmm0, [esi+0x08]
        movss		xmm2, [esi+MATHUTIL_STRIDE+0x8] //0x18]
        shufps	xmm0, xmm0, 0x00
            shufps	xmm2, xmm2, 0x00
            mulps		xmm0, xmm6
            add			esi, MATHUTIL_STRIDE*2
            mulps		xmm2, xmm6
            add			edi, MATHUTIL_STRIDE*2
            addps		xmm1, xmm0
            addps		xmm3, xmm2

            // process w
            addps		xmm1, xmm7
            addps		xmm3, xmm7

            // write output vectors to memory and loop
            //movups	[edi-stride2], xmm1 //[edi-0x20], xmm1
            //movups	[edi-stide], xmm3 //[edi-0x10], xmm3
            // cant use movups since we only have 3 components
            movss	[edi-MATHUTIL_STRIDE*2], xmm1
            shufps xmm1, xmm1, 0x39 // shift towards the least significant bit
            movss	[edi-(MATHUTIL_STRIDE*2-0x4)], xmm1
            shufps xmm1, xmm1, 0x39 // shift towards the least significant bit
            movss	[edi-(MATHUTIL_STRIDE*2-0x8)], xmm1

            movss	[edi-MATHUTIL_STRIDE], xmm3
            shufps xmm3, xmm3, 0x39 // shift towards the least significant bit
            movss	[edi-(MATHUTIL_STRIDE-0x4)], xmm3
            shufps xmm3, xmm3, 0x39 // shift towards the least significant bit
            movss	[edi-(MATHUTIL_STRIDE-0x8)], xmm3

            dec			ecx
            jnz			BT2_START
    }

#elif 1
    for (unsigned int i=0; i<num; i++) {
        __m128 row0 = _mm_loadu_ps(m);
        __m128 row1 = _mm_loadu_ps(m+4);
        __m128 row2 = _mm_loadu_ps(m+8);
        __m128 row3 = _mm_loadu_ps(m+12);

        __m128 vecX = _mm_loadu_ps((float*)vin);
        vout += MATHUTIL_STRIDE;

        __m128 vecY = vecX;
        vin += MATHUTIL_STRIDE;
        __m128 vecZ = vecX;

        vecX = _mm_shuffle_ps(vecX, vecX, 0x00);
        vecY = _mm_shuffle_ps(vecY, vecY, 0x55);
        vecZ = _mm_shuffle_ps(vecZ, vecZ, 0xAA);

        // multiply vecX/Y/Z by the appropriate columns of the matrix
        vecX = _mm_mul_ps(vecX, row0);
        vecY = _mm_mul_ps(vecY, row1);
        vecZ = _mm_mul_ps(vecZ, row2);

        // sum the results into vecY
        vecY = _mm_add_ps(vecY, vecX);
        vecZ = _mm_add_ps(vecZ, row3);
        vecY = _mm_add_ps(vecY, vecZ);


        _mm_store_ss((float*)(vout-MATHUTIL_STRIDE), vecY);
        vecY = _mm_shuffle_ps(vecY, vecY, 0x39); // shift towards the least significant bit
        _mm_store_ss((float*)(vout-(MATHUTIL_STRIDE-0x4)), vecY);
        vecY = _mm_shuffle_ps(vecY, vecY, 0x39); // shift towards the least significant bit
        _mm_store_ss((float*)(vout-(MATHUTIL_STRIDE-0x8)), vecY);
    }
#endif
}