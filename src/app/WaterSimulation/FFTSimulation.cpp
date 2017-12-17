/*
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 */

#include "FFTSimulation.h"

const float FFTSimulation::WINDSPEED = 3.0f;
const float FFTSimulation::A = 3.0f;
const float FFTSimulation::WAVELENGTH = 4.0f;
const float FFTSimulation::GRAVITY = 9.81f;
const float FFTSimulation::PI = 3.14159f;
const float FFTSimulation::CELL_DISTANCE = 100.0f;

FFTSimulation::FFTSimulation()
{
	m_windDirection.x = 1.0f;		m_windDirection.y = .0f;

	m_pFftIn = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*GRIDSIZE*GRIDSIZE);
	m_pFftOut = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*GRIDSIZE*GRIDSIZE);
}

FFTSimulation::~FFTSimulation()
{
	fftwf_destroy_plan(m_FftPlan);
	fftwf_free(m_pFftIn);
	fftwf_free(m_pFftOut);
}

void FFTSimulation::initFFTSimulation()
{
	m_FftPlan = fftwf_plan_dft_1d(GRIDSIZE*GRIDSIZE, m_pFftIn, m_pFftOut, FFTW_BACKWARD, FFTW_ESTIMATE);

	for(int i=0; i<GRIDSIZE; i++) {
		for(int j=0; j<GRIDSIZE; j++)
		{
			const int index = i*GRIDSIZE + j;

			m_waveDirection.x = 2*PI*(i - GRIDSIZE/2.0f)/L;
			m_waveDirection.y = 2*PI*(j - GRIDSIZE/2.0f)/L;

			m_amplitudePos[index] = calculateH0(1);
			m_amplitudeNeg[index] = calculateH0(-1);

		}
	}
	
}

FFTSimulation::Vec2 FFTSimulation::calculateH0(short mul)
{
	Vec2 v = gaussian(0.0f,1.0f);
	float phillips = calculatePhillipsSpectrum(mul);

	float factor = sqrt(phillips/2);

	v.x *= factor;
	v.y *= factor;

	return v;
	
}

float FFTSimulation::calculatePhillipsSpectrum(short mul)
{
	m_waveDirection.x *= mul;
	m_waveDirection.y *= mul;

	float cosineFactor = m_waveDirection.x*m_windDirection.x + m_waveDirection.y*m_windDirection.y;

	float Ph = A*exp((-1.0f * GRAVITY * GRAVITY) / (WAVELENGTH * WAVELENGTH * WINDSPEED * WINDSPEED * WINDSPEED * WINDSPEED));
	Ph /= WAVELENGTH * WAVELENGTH * WAVELENGTH * WAVELENGTH * WAVELENGTH;
	Ph *= cosineFactor*cosineFactor;

	/*float l = 1.0f;
	float modifyFactor = exp(-(WAVELENGTH*WAVELENGTH)*l*l);

	//Ph *= modifyFactor;*/

	return Ph;
}

FFTSimulation::Vec2 FFTSimulation::gaussian(float mean, float stdDeviation)
{
	Vec2 v;

	float x1, x2, w, y1, y2;
 
    do {
		x1 = 2.0f * getRandom() - 1.0f;
        x2 = 2.0f * getRandom() - 1.0f;
         w = x1 * x1 + x2 * x2;
	} while ( w >= 1.0f );

    w = sqrt( (-2.0f * log( w ) ) / w );
    y1 = x1 * w;
    y2 = x2 * w;

	v.x = y1;
	v.y = y2;

	return v;
}

void FFTSimulation::update()
{
	float time = glutGet(GLUT_ELAPSED_TIME)/300.0f;

	float omega, waveDirLength;

	for(int i=0; i<GRIDSIZE; i++)
		for(int j=0; j<GRIDSIZE; j++)
		{
			const int index = i*GRIDSIZE + j;

			m_waveDirection.x = 2*PI*(i - GRIDSIZE/2.0f)/L;
			m_waveDirection.y = 2*PI*(j - GRIDSIZE/2.0f)/L;

			waveDirLength = sqrt(m_waveDirection.x*m_waveDirection.x + m_waveDirection.y*m_waveDirection.y);
			omega = sqrt(GRAVITY*waveDirLength);

			float omegaT = omega*time;

			m_pFftIn[index][0]= (m_amplitudePos[index].x + m_amplitudeNeg[index].x)*cos(omegaT) - (m_amplitudePos[index].y + m_amplitudeNeg[index].y)*sin(omegaT);
			m_pFftIn[index][1] = (m_amplitudePos[index].y - m_amplitudeNeg[index].y)*cos(omegaT) + (m_amplitudePos[index].x - m_amplitudeNeg[index].x)*sin(omegaT);

		}

		fftwf_execute(m_FftPlan);

}

void FFTSimulation::calculateAndFillNormals(unsigned char* normals)
{
	Vector3 n1, n2, N; // right and forward vectors for calculating normal

	for(int i=0; i<GRIDSIZE; i++)
		for(int j=0; j<GRIDSIZE; j++)
		{
			const int index = i*GRIDSIZE + j;

			//m_pFftOut[index][0] += 40.0f;
			//m_pFftOut[index][0] /= 20.0f;

			if(i==0)
				n1 = Vector3(CELL_DISTANCE, m_pFftOut[index][0]-m_pFftOut[index+GRIDSIZE][0], 0.0f );
			else if(i == GRIDSIZE-1)
				n1 = Vector3(CELL_DISTANCE, m_pFftOut[index-GRIDSIZE][0]-m_pFftOut[index][0], 0.0f );
			else
				n1 = Vector3(2.0f*CELL_DISTANCE, m_pFftOut[index-GRIDSIZE][0]-m_pFftOut[index+GRIDSIZE][0], 0.0f );

			if(j==0)
				n2 = Vector3(0.0f, m_pFftOut[index+1][0]-m_pFftOut[index][0], CELL_DISTANCE );
			else if(j == GRIDSIZE-1)
				n2 = Vector3(0.0f, m_pFftOut[index][0]-m_pFftOut[index-1][0], CELL_DISTANCE );
			else 
				n2 = Vector3(0.0f, m_pFftOut[index+1][0]-m_pFftOut[index-1][0], 2.0f*CELL_DISTANCE );

			N.crossProduct(n2,n1);
			N.normalize();

			int offset = 4*index;

			*(normals + offset) = ((N[0]+1)/2.0f)*255;
			*(normals + offset+1) =((N[2]+1)/2.0f)*255; //Tangent Space 
			*(normals + offset+2) =  ((N[1]+1)/2.0f)*255;
			*(normals + offset+3) = m_pFftOut[index][0];
		}

	for(int i=0; i<GRIDSIZE; i++)
		for(int j=0; j<GRIDSIZE; j++)
		{
			const int index = i*GRIDSIZE + j;

			if(i==0)
			{
				const int indexOpposite = (GRIDSIZE-1)*GRIDSIZE + j;

				const int offset = 4*index;
				const int offsetOpposite = 4*indexOpposite;

				*(normals + offset) = *(normals + offset)*(2.0f/3.0f) + *(normals + offsetOpposite)*(1.0f/3.0f);
				*(normals + offset+1) = *(normals + offset+1)*(2.0f/3.0f) + *(normals + offsetOpposite+1)*(1.0f/3.0f);
				*(normals + offset+2) = *(normals + offset+2)*(2.0f/3.0f) + *(normals + offsetOpposite+2)*(1.0f/3.0f);

				*(normals + offsetOpposite) = *(normals + offset);
				*(normals + offsetOpposite+1) = *(normals + offset+1);
				*(normals + offsetOpposite+2) = *(normals + offset+2);
			}

			if(j==0)
			{
				const int indexOpposite = i*GRIDSIZE + GRIDSIZE-1;
				const int indexNeighbour1 = index + 1;
				const int indexNeighbour2 = indexOpposite-1;

				const int offset = 4*index;
				const int offsetOpposite = 4*indexOpposite;
				const int offsetNeighbour1 = 4*indexNeighbour1;
				const int offsetNeighbour2 = 4*indexNeighbour2;

				*(normals + offset) = *(normals + offsetNeighbour1)*0.5f + *(normals + offsetNeighbour2)*0.5f;
				*(normals + offset+1) = *(normals + offsetNeighbour1+1)*0.5f + *(normals + offsetNeighbour2+1)*0.5f;
				*(normals + offset+2) = *(normals + offsetNeighbour1+2)*0.5f + *(normals + offsetNeighbour2+2)*0.5f;

				*(normals + offsetOpposite) = *(normals + offset);
				*(normals + offsetOpposite+1) = *(normals + offset+1);
				*(normals + offsetOpposite+2) = *(normals + offset+2);
			}
		}
}
