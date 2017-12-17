/** \class FFTSimulation
 * Does FFT simulation for distant water
 *
 * @author  Rahul Mukhi
 * @date 27/03/12
 *
 * Copyright (C) GIANTS Software GmbH, Confidential, All Rights Reserved.
 */

#include <stdlib.h>
#include "fftw/fftw3.h"
#include "PreCompiled.h"
#include "glut/glut.h"
#include "base/math/Vector3.h"

class FFTSimulation
{
public:
	FFTSimulation();
	~FFTSimulation();

	static const unsigned short GRIDSIZE = 64;

	void initFFTSimulation();
	void update();
	void calculateAndFillNormals(unsigned char* normals);

private:

	struct Vec2
	{
		float x,y;
	};

	static const unsigned short L = 256;
	static const float WINDSPEED, A, WAVELENGTH, GRAVITY, PI, CELL_DISTANCE;

	float m_h0Real[GRIDSIZE*GRIDSIZE], m_h0Complex[GRIDSIZE*GRIDSIZE];
	Vec2 m_amplitudePos[GRIDSIZE*GRIDSIZE], m_amplitudeNeg[GRIDSIZE*GRIDSIZE];
	Vec2 m_windDirection,m_waveDirection;

	fftwf_complex *m_pFftIn;
	fftwf_complex *m_pFftOut;
	fftwf_plan m_FftPlan;

	Vec2 calculateH0(short mul);
	Vec2 gaussian(float mean, float stdDeviation);
	float calculatePhillipsSpectrum(short mul);

	inline float getRandom(float min=0.0f, float max=1.0f)
	{
		return min+( (rand()/(RAND_MAX+1.0f)) *(max-min) );  
	}
	
};