/*
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 */

#include "WaterShape.h"

using namespace std;


WaterShape::WaterShape(PortScene* portScene): m_waterSimulation(portScene)
{
	initWaterShape();
	m_line = false;
}

WaterShape::~WaterShape()
{
	delete[] m_fftNormals;
	deleteVBO();
	deleteShaders();
	deleteFrameBufferObject();
}

void WaterShape::initWaterShape()
{
	initializeGrid();
	initShaders();
	initNormalMap();
	initFrameBufferObject();
	initFFTSimulation();

	createVBO();

	m_fftNormals = new unsigned char[m_fftSimulation.GRIDSIZE * m_fftSimulation.GRIDSIZE * 4];
}

void WaterShape::createVBO()
{
	std::vector<GLuint> indexVectSWE;
	std::vector<GLuint> indexVectFFT;
	indexVectSWE.clear();
	indexVectFFT.clear();

	m_waterSimulation.fillIndicesSWE(indexVectSWE);
	m_waterSimulation.fillIndicesFFT(indexVectFFT);

	m_numIndicesSWE = indexVectSWE.size();
	m_numIndicesFFT = indexVectFFT.size();

	glGenBuffers(1, &m_vertexVBOIdSWE);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBOIdSWE);
	glBufferData(GL_ARRAY_BUFFER, m_waterSimulation.NUM_GRIDS*6*sizeof(float), NULL, GL_STREAM_DRAW);

	glGenBuffers(1, &m_indexVBOIdSWE);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBOIdSWE);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_numIndicesSWE*sizeof(GLuint), &indexVectSWE[0], GL_STATIC_DRAW);

	glGenBuffers(1, &m_vertexVBOIdFFT);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBOIdFFT);
	float* fftVertices;
	fftVertices = new float[(m_waterSimulation.NUM_CELLS*4 + 4)*3];
	m_waterSimulation.fillFFTVertexBuffer(fftVertices);
	glBufferData(GL_ARRAY_BUFFER, (m_waterSimulation.NUM_CELLS*4 + 4)*3*sizeof(float), fftVertices, GL_STATIC_DRAW);
	delete[] fftVertices;

	glGenBuffers(1, &m_indexVBOIdFFT);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBOIdFFT);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_numIndicesFFT*sizeof(GLuint), &indexVectFFT[0], GL_STATIC_DRAW);
	
}

void WaterShape::deleteVBO()
{
	glDeleteBuffers(1, &m_vertexVBOIdSWE);
	glDeleteBuffers(1, &m_indexVBOIdSWE);
	glDeleteBuffers(1, &m_vertexVBOIdFFT);
	glDeleteBuffers(1, &m_indexVBOIdFFT);
}

void WaterShape::deleteFrameBufferObject()
{
	glDeleteFramebuffers(1, &m_frameBuffer);
	glDeleteRenderbuffers(1, &m_reflectionRenderBuffer);
}

void WaterShape::initializeGrid()
{
	m_waterSimulation.initializeGrid();
}

void WaterShape::initShaders()
{
	m_fftVertShader = glCreateShader(GL_VERTEX_SHADER);
	m_fftFragShader = glCreateShader(GL_FRAGMENT_SHADER);
	
	{
		ifstream fftVertShaderFile("FFTVertexShader.glsl", ios::binary|ios::in);
		fftVertShaderFile.seekg(0,ios::end);
		int size = fftVertShaderFile.tellg();
	
		fftVertShaderFile.seekg(0,ios::beg);
		char *vShader = new char[size+1];
		fftVertShaderFile.read(vShader, size);
		vShader[size]=0;
		fftVertShaderFile.close();

		glShaderSource(m_fftVertShader, 1, (const char**)&vShader, &size);
		glCompileShader(m_fftVertShader);
		GLint vertCompiled = 0;
		glGetShaderiv(m_fftVertShader, GL_COMPILE_STATUS, &vertCompiled);
		if(!vertCompiled) {

			char message[1024];
			int logsize;
			glGetShaderInfoLog(m_fftVertShader, 1024, &logsize, message);
			message[logsize] = 0;
			std::cout<< message << std::endl;
			exit(1);
		}
		delete[] vShader;
	}

	{
		ifstream fftFragShaderFile("FFTFragmentShader.glsl", ios::binary|ios::in);
		fftFragShaderFile.seekg(0,ios::end);
		int size = fftFragShaderFile.tellg();
		
		fftFragShaderFile.seekg(0,ios::beg);
		char *fShader = new char[size+1];
		fftFragShaderFile.read(fShader, size);
		fShader[size]=0;
		fftFragShaderFile.close();

		glShaderSource(m_fftFragShader, 1, (const char**)&fShader, &size);
		glCompileShader(m_fftFragShader);
		GLint fragCompiled = 0;
		glGetShaderiv(m_fftFragShader, GL_COMPILE_STATUS, &fragCompiled);
		if(!fragCompiled){

			char message[1024];
			int logsize;
			glGetShaderInfoLog(m_fftFragShader, 1024, &logsize, message);
			message[logsize] = 0;
			std::cout<< message << std::endl;
			exit(1);
		}
		delete[] fShader;
	}

	m_fftShaderProgram = glCreateProgram();
	glAttachShader(m_fftShaderProgram, m_fftVertShader);
	glAttachShader(m_fftShaderProgram, m_fftFragShader);
	glLinkProgram(m_fftShaderProgram);

	{
		GLint status;
		glGetProgramiv( m_fftShaderProgram, GL_LINK_STATUS, &status );
		if(!status){

			char message[1024];
			int logsize;
			glGetProgramInfoLog(m_fftShaderProgram, 1024, &logsize, message);
			message[logsize] = 0;
			std::cout<< message << std::endl;
			exit(1);
		}
	}


	m_sweVertShader = glCreateShader(GL_VERTEX_SHADER);
	m_sweFragShader = glCreateShader(GL_FRAGMENT_SHADER);

	{
		ifstream sweVertShaderFile("SWEVertexShader.glsl", ios::binary|ios::in);
		sweVertShaderFile.seekg(0,ios::end);
		int size = sweVertShaderFile.tellg();
		
		sweVertShaderFile.seekg(0,ios::beg);
		char *vShader = new char[size+1];
		sweVertShaderFile.read(vShader, size);
		vShader[size]=0;
		sweVertShaderFile.close();
		
		glShaderSource(m_sweVertShader, 1, (const char**)&vShader, &size);
		glCompileShader(m_sweVertShader);
		GLint vertCompiled = 0;
		glGetShaderiv(m_sweVertShader, GL_COMPILE_STATUS, &vertCompiled);
		if(!vertCompiled){

			char message[1024];
			int logsize;
			glGetShaderInfoLog(m_sweVertShader, 1024, &logsize, message);
			message[logsize] = 0;
			std::cout<< message << std::endl;
			exit(1);
		}

		delete[] vShader;
	}

	{
		ifstream sweFragShaderFile("SWEFragmentShader.glsl", ios::binary);
		sweFragShaderFile.seekg(0,ios::end);
		int size = sweFragShaderFile.tellg();
		
		sweFragShaderFile.seekg(0,ios::beg);
		char *fShader = new char[size+1];
		sweFragShaderFile.read(fShader, size);
		fShader[size] = 0;
		sweFragShaderFile.close();

		char defines[200];
		sprintf(defines, "#define GRIDSTARTX %f;\n #define GRIDSTARTZ %f;\n #define INV_GRIDLENGTH %f;\n ", m_waterSimulation.getgridStartX(), m_waterSimulation.getgridStartZ(), 1.0f/(m_waterSimulation.NUM_CELLS*m_waterSimulation.CELL_EDGE));
		char *fragmentShader[2] = {defines, fShader};

		glShaderSource(m_sweFragShader, 2, (const char**)&fragmentShader, NULL);
		glCompileShader(m_sweFragShader);
		GLint fragCompiled = 0;
		glGetShaderiv(m_sweFragShader, GL_COMPILE_STATUS, &fragCompiled);
		if(!fragCompiled){

			char message[1024];
			int logsize;
			glGetShaderInfoLog(m_fftFragShader, 1024, &logsize, message);
			message[logsize] = 0;
			std::cout<< message << std::endl;
			exit(1);
		}
		delete[] fShader;
	}

	m_sweShaderProgram = glCreateProgram();
	glAttachShader(m_sweShaderProgram, m_sweVertShader);
	glAttachShader(m_sweShaderProgram, m_sweFragShader);
	glLinkProgram(m_sweShaderProgram);

	{
		GLint status;
		glGetProgramiv( m_sweShaderProgram, GL_LINK_STATUS, &status );
		if(!status){

			char message[1024];
			int logsize;
			glGetProgramInfoLog(m_sweShaderProgram, 1024, &logsize, message);
			message[logsize] = 0;
			std::cout<< message << std::endl;
			exit(1);
		}
	}
}

void WaterShape::initNormalMap()
{
	/*long size;
	size_t readSize;

	m_pNormalMap = fopen("Data/water_normal.png", "rb");
	fseek(m_pNormalMap, 0, SEEK_END);
	size = ftell(m_pNormalMap);
	rewind(m_pNormalMap);
	m_pBufferNormalMap = (unsigned char*)malloc(sizeof(unsigned char)*size);
	readSize = fread(m_pBufferNormalMap, 1, size, m_pNormalMap);
	m_pngUtil.loadImage(m_pBufferNormalMap, readSize, m_imageDescNormalMap);
	fclose(m_pNormalMap);
	free(m_pBufferNormalMap);*/

	glGenTextures(1, &m_normalMapTexture);
	glBindTexture(GL_TEXTURE_2D, m_normalMapTexture);

	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );  
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ); 

	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_imageDescNormalMap.m_width, m_imageDescNormalMap.m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_imageDescNormalMap.m_pBuffer);

	int normal_location = glGetUniformLocation(m_fftShaderProgram, "normalMap_texture");
	glUseProgram(m_fftShaderProgram);
	glUniform1i(normal_location, 0);
	glUseProgram(0);

	normal_location = glGetUniformLocation(m_sweShaderProgram, "normalMap_texture");
	glUseProgram(m_sweShaderProgram);
	glUniform1i(normal_location, 0);
	glUseProgram(0);

}

void WaterShape::initFrameBufferObject()
{
	glGenTextures(1, &m_reflectionTexture);
	glBindTexture(GL_TEXTURE_2D, m_reflectionTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE  );  
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE  ); 
	int reflection_location = glGetUniformLocation(m_fftShaderProgram, "reflection_texture");

	glUseProgram(m_fftShaderProgram);
	glUniform1i(reflection_location, 1);
	glUseProgram(0);

	reflection_location = glGetUniformLocation(m_sweShaderProgram, "reflection_texture");

	glUseProgram(m_sweShaderProgram);
	glUniform1i(reflection_location, 1);
	glUseProgram(0);
	
	glGenFramebuffers(1, &m_frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);

	glGenRenderbuffers(1, &m_reflectionRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_reflectionRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_reflectionRenderBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_reflectionTexture, 0);
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void WaterShape::initFFTSimulation()
{
	m_fftSimulation.initFFTSimulation();
}

void WaterShape::deleteShaders()
{
	glDetachShader(m_fftShaderProgram, m_fftVertShader);
	glDetachShader(m_fftShaderProgram, m_fftFragShader);

	glDeleteShader(m_fftVertShader);
	glDeleteShader(m_fftFragShader);

	glDeleteProgram(m_fftShaderProgram);

	glDetachShader(m_sweShaderProgram, m_sweVertShader);
	glDetachShader(m_sweShaderProgram, m_sweFragShader);

	glDeleteShader(m_sweVertShader);
	glDeleteShader(m_sweFragShader);

	glDeleteProgram(m_sweShaderProgram);
}

void WaterShape::update(const Vector3& cameraView)
{
	m_waterSimulation.update(cameraView);
	m_fftSimulation.update();
}

void WaterShape::addDrop(int x, int y)
{
	m_waterSimulation.addDrop(x, y);
}

void WaterShape::renderWater(const Vector3& cameraPos)
{

	glEnableClientState(GL_VERTEX_ARRAY);

	renderFFTGrid(cameraPos);

	glEnableClientState(GL_NORMAL_ARRAY);

	renderSWEGrid(cameraPos);

	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void WaterShape::updateSWEGrid()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBOIdSWE);
	float* pVertices = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	m_waterSimulation.fillVertexBufferandUpdateNormals(pVertices);
	glUnmapBuffer(GL_ARRAY_BUFFER); 

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

void WaterShape::renderSWEGrid(const Vector3& cameraPos)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBOIdSWE);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBOIdSWE);
	
	glUseProgram(m_sweShaderProgram);

	int xTranslateLocationSWE = glGetUniformLocation(m_sweShaderProgram, "x_translate");
	glUniform1f(xTranslateLocationSWE, m_waterSimulation.getTranslationX());

	int zTranslateLocationSWE = glGetUniformLocation(m_sweShaderProgram, "z_translate");
	glUniform1f(zTranslateLocationSWE, m_waterSimulation.getTranslationZ());

	int cameraPosLocationSWE = glGetUniformLocation(m_sweShaderProgram, "cameraPos");
	glUniform3fv(cameraPosLocationSWE, 1, &cameraPos[0]);


	glVertexPointer(	3,   //3 components per vertex (x,y,z)
						GL_FLOAT,
						6*sizeof(float),
						NULL);
	glNormalPointer(	GL_FLOAT,
						6*sizeof(float),
						(void*)(3*sizeof(float)));

	if(m_line) {

		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );


		glDrawElements(	GL_TRIANGLES, //mode
							m_numIndicesSWE,  //count, ie. how many indices
							GL_UNSIGNED_INT, //type of the index array
							NULL);

		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	} else {

		glDrawElements(	GL_TRIANGLES, //mode
						m_numIndicesSWE,  //count, ie. how many indices
						GL_UNSIGNED_INT, //type of the index array
						NULL);
	}


	glUseProgram(0);
}

void WaterShape::renderFFTGrid(const Vector3& cameraPos)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBOIdFFT);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexVBOIdFFT);

	glUseProgram(m_fftShaderProgram);

	int xTranslateLocationFFT = glGetUniformLocation(m_fftShaderProgram, "x_translate");
	glUniform1f(xTranslateLocationFFT, m_waterSimulation.getTranslationX());

	int zTranslateLocationFFT = glGetUniformLocation(m_fftShaderProgram, "z_translate");
	glUniform1f(zTranslateLocationFFT, m_waterSimulation.getTranslationZ());

	int cameraPosLocationFFT = glGetUniformLocation(m_fftShaderProgram, "cameraPos");
	glUniform3fv(cameraPosLocationFFT, 1, &cameraPos[0]);

	m_fftSimulation.calculateAndFillNormals(m_fftNormals);

	glBindTexture(GL_TEXTURE_2D, m_normalMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fftSimulation.GRIDSIZE, m_fftSimulation.GRIDSIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_fftNormals);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);  
	glBindTexture(GL_TEXTURE_2D, m_normalMapTexture); 

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);  
	glBindTexture(GL_TEXTURE_2D, m_reflectionTexture);

	glVertexPointer(	3,   //3 components per vertex (x,y,z)
						GL_FLOAT,
						3*sizeof(float),
						NULL);

	glDrawElements(	GL_TRIANGLES, //mode
						m_numIndicesFFT,  //count, ie. how many indices
						GL_UNSIGNED_INT, //type of the index array
						NULL);

	glDisable(GL_TEXTURE_2D);  
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);  

	glUseProgram(0);
}

void WaterShape::passModelViewProjectionToGLSL()
{
	float modelViewMatrix[16], projectionMatrix[16];

	glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMatrix);
	glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);

	glUseProgram(m_fftShaderProgram);

	int modelViewLocationFFT = glGetUniformLocation(m_fftShaderProgram, "modelView");
	glUniformMatrix4fv(modelViewLocationFFT, 1, GL_FALSE, &modelViewMatrix[0]);

	int projectionLocationFFT = glGetUniformLocation(m_fftShaderProgram, "projection");
	glUniformMatrix4fv(projectionLocationFFT, 1, GL_FALSE, &projectionMatrix[0]);

	glUseProgram(m_sweShaderProgram);

	int modelViewLocationSWE = glGetUniformLocation(m_fftShaderProgram, "modelView");
	glUniformMatrix4fv(modelViewLocationSWE, 1, GL_FALSE, &modelViewMatrix[0]);

	int projectionLocationSWE = glGetUniformLocation(m_fftShaderProgram, "projection");
	glUniformMatrix4fv(projectionLocationSWE, 1, GL_FALSE, &projectionMatrix[0]);

	glUseProgram(0);
}

void WaterShape::pressNormalKey(unsigned char key) 
{
	if(key == 'l') {

		if(m_line) {
			m_line = false;
		} else {
			m_line = true;
		}

	}
}


