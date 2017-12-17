/** \class WaterShape
 * Updates and renders water surface
 *
 * @author  Rahul Mukhi
 * @date 27/03/12
 *
 * Copyright (C) GIANTS Software GmbH, Confidential, All Rights Reserved.
 */

#include "WaterSimulation.h"
#include "FFTSimulation.h"
#include "base/2d/PNGUtil.h"

class WaterShape
{

public:
	WaterShape(PortScene* portScene);
	~WaterShape();

	GLuint m_frameBuffer, m_reflectionTexture;
	GLuint m_reflectionRenderBuffer;

	WaterSimulation m_waterSimulation;

	void initWaterShape();
	
	void update(const Vector3& cameraView);
	void updateSWEGrid();
	void renderWater(const Vector3& cameraPos);
	void addDrop(int x, int y);
	void passModelViewProjectionToGLSL();
	void pressNormalKey(unsigned char key);

	inline float getTotalHeight()
	{
		return m_waterSimulation.TOTAL_HEIGHT;
	}

private:

	struct PlaneDef
	{
		float a,b,c,d;
	};

	struct PortVertex
	{
		float x,y,z;
	};

	PortVertex* m_pPortVertices;
	std::vector <GLushort> m_indexPort;

	bool m_line;

	FILE *m_pNormalMap;
	unsigned char *m_pBufferNormalMap;
	PNGUtil m_pngUtil;
	ImageDesc m_imageDescNormalMap;
	
	PlaneDef m_inclinedPlane;
	FFTSimulation m_fftSimulation;
	
	unsigned int m_numIndicesSWE, m_numIndicesFFT;
	GLuint m_vertexVBOIdSWE, m_indexVBOIdSWE, m_vertexVBOIdFFT, m_indexVBOIdFFT;
	GLuint m_fftFragShader, m_fftVertShader, m_fftShaderProgram;
	GLuint m_sweVertShader, m_sweFragShader, m_sweShaderProgram;
	GLuint m_normalMapTexture;

	unsigned char* m_fftNormals;

	void createVBO();
	void deleteVBO();

	void initShaders();
	void deleteShaders();

	void initializeGrid();
	void initRestScene();
	void initLight();
	void initNormalMap();
	void initFrameBufferObject();
	void initFFTSimulation();
	void deleteFrameBufferObject();

	
	void fillIndices();
	void renderNonSWEquads();
	void renderSWEGrid(const Vector3& cameraPos);
	void renderFFTGrid(const Vector3& cameraPos);

};
