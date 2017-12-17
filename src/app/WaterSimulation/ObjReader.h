/** \class Object
 * Reads object file and contains object features
 *
 * @author  Rahul Mukhi
 * @date 04/05/12
 *
 * Copyright (C) GIANTS Software GmbH, Confidential, All Rights Reserved.
 */
#include <algorithm>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <string>
#include <assert.h>
#include "base/math/Vector3.h"

#pragma once 

class ObjReader
{
	struct face {
		int vertex[10];
		int texture[10];
		int normal[10];
		short numVertices;
		std::string materialName;
		short materialIndex;
	};

	struct group {
		std::string groupName;
		std::vector<face> faces;
		float *vertexData;
		unsigned int *indices;
		std::string materialName;
		short materialIndex;
	};

	struct material {
		std::string materialName;
		std::string textureFileName;
		/*Vector3 kAmbient;
		Vector3 kDiffuse;
		Vector3 kSpecular;
		Vector3 transmissionFilter;
		float specularExponent;
		float refractionIndex;
		short illuminationType;*/
	};

public: 
	ObjReader();
	~ObjReader();

	void objectLoader(std::string filename);

	std::vector<Vector3> m_vertices;
    std::vector<Vector3> m_texCoords;
    std::vector<Vector3> m_normals;
    std::vector<face> m_faces;
	std::vector<group> m_groups;
	std::vector<material> m_material;

	float m_minX, m_maxX, m_minZ, m_maxZ;
    
private:

	std::string m_mtlFile;

	void mtlLoader(std::string mtlFile);

};