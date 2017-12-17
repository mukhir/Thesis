/*
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 */

#include "ObjReader.h"

ObjReader::ObjReader()
{
}

ObjReader::~ObjReader()
{
	for(int i=0; i<m_groups.size(); i++ ) {
		delete[] m_groups[i].vertexData;
		delete[] m_groups[i].indices;
	}
}

void ObjReader::objectLoader(std::string filename)
{
	std::ifstream inFile(filename.c_str(), std::ifstream::in);
    std::string line, key, usemtl;
	bool mtlFile = false;
	bool isGroup = false;
	bool firstVertex =  true;
	short groupNumber = -1;

	// Read huge data for portscene
	float *vertexOBJ, *texCoordOBJ, *normalOBJ;
	const int MAX_ALLOCATION = 300000;
	vertexOBJ = new float[MAX_ALLOCATION];
	texCoordOBJ = new float[MAX_ALLOCATION];
	normalOBJ = new float[MAX_ALLOCATION];

	int vIndex, tIndex, nIndex, vertexDataIndex;

	vIndex = tIndex = nIndex = 0;
	vertexDataIndex= -1;

	m_vertices.clear();
	m_texCoords.clear();
	m_normals.clear();
	m_faces.clear();
	m_groups.clear();
	m_material.clear();

	while(inFile.good() && !inFile.eof() && std::getline(inFile, line)) {

		key = "";
		std::stringstream sStream(line);
		sStream>>key>>std::ws;

		if(key == "v") {

			Vector3 vertex;
			int i=0;
			while(!sStream.eof()) {

				sStream>>vertex[i]>>std::ws;

				vertexOBJ[vIndex] = vertex[i];
				vIndex++;
				assert(vIndex<MAX_ALLOCATION);
				
				i++;
			}

			if(!isGroup) {
				m_vertices.push_back(vertex);
			}

		} else if(key == "vt") {

			Vector3 texCoord;
			int i=0;
			while(!sStream.eof()) {

				sStream>>texCoord[i]>>std::ws;

				texCoordOBJ[tIndex] = texCoord[i];
				tIndex++;
				assert(tIndex<MAX_ALLOCATION);

				i++;
			}

			if(!isGroup) {
				m_texCoords.push_back(texCoord);
			}

		} else if(key == "vn") {

			Vector3 normal;
			int i=0;
			while(!sStream.eof()) {

				sStream>>normal[i]>>std::ws;

				normalOBJ[nIndex] = normal[i];
				nIndex++;
				assert(nIndex<MAX_ALLOCATION);

				i++;
			}

			if(!isGroup) {
				m_normals.push_back(normal);
			}

		} else if(key == "g") {

			std::string groupName, temp;
			sStream>>groupName>>std::ws;

			if(groupName.compare("default")) {

				group g;
				g.faces.clear();
				g.groupName = groupName;
				g.materialIndex = -1;

				std::getline(inFile, line);
				std::stringstream sStream(line);
				sStream>>temp>>std::ws;
				sStream>>g.materialName>>std::ws;

				isGroup = true;
				mtlFile = true;
				groupNumber++;
				m_groups.push_back(g);

			} else { // if group name not default then read face data

				if(groupNumber > -1) {

					m_groups[groupNumber].vertexData = NULL;

					m_groups[groupNumber].vertexData = new float[m_groups[groupNumber].faces.size()*24]; // 3 floats for 3 vertices, 3 normals and 2 texcoords
					m_groups[groupNumber].indices = new unsigned int[m_groups[groupNumber].faces.size()*3]; // 3 ints for 3 vertices in each face
					vertexDataIndex = -1;

					unsigned int indices = 0;
					const float SCALE = 1.0f/100.0f;

					for(int i=0; i<m_groups[groupNumber].faces.size(); i++) {

						for(int j=0; j<3; j++) {

							vertexDataIndex++;
							m_groups[groupNumber].vertexData[vertexDataIndex] = vertexOBJ[m_groups[groupNumber].faces[i].vertex[j]*3]*SCALE - 130.0f; // x coordinate
							vertexDataIndex++;
							m_groups[groupNumber].vertexData[vertexDataIndex] = vertexOBJ[m_groups[groupNumber].faces[i].vertex[j]*3+1]*SCALE + 5.0f; // y coordinate
							vertexDataIndex++;
							m_groups[groupNumber].vertexData[vertexDataIndex] = vertexOBJ[m_groups[groupNumber].faces[i].vertex[j]*3+2]*SCALE + 3.0f;  // z coordinate

							if(firstVertex) {
								m_minX = m_maxX = m_groups[0].vertexData[0];
								m_minZ = m_maxZ = m_groups[0].vertexData[2];
								firstVertex = false;
							} else {
								m_minX = std::min(m_minX, m_groups[groupNumber].vertexData[vertexDataIndex-2]);
								m_maxX = std::max(m_maxX, m_groups[groupNumber].vertexData[vertexDataIndex-2]);
								m_minZ = std::min(m_minZ, m_groups[groupNumber].vertexData[vertexDataIndex]);
								m_maxZ = std::max(m_maxZ, m_groups[groupNumber].vertexData[vertexDataIndex]);
							}
					
							vertexDataIndex++;
							m_groups[groupNumber].vertexData[vertexDataIndex] = normalOBJ[m_groups[groupNumber].faces[i].normal[j]*3];
							vertexDataIndex++;
							m_groups[groupNumber].vertexData[vertexDataIndex] = normalOBJ[m_groups[groupNumber].faces[i].normal[j]*3+1];
							vertexDataIndex++;
							m_groups[groupNumber].vertexData[vertexDataIndex] = normalOBJ[m_groups[groupNumber].faces[i].normal[j]*3+2];
					
							vertexDataIndex++;
							m_groups[groupNumber].vertexData[vertexDataIndex] = texCoordOBJ[m_groups[groupNumber].faces[i].texture[j]*2];
							vertexDataIndex++;
							m_groups[groupNumber].vertexData[vertexDataIndex] = -texCoordOBJ[m_groups[groupNumber].faces[i].texture[j]*2+1];

							m_groups[groupNumber].indices[indices] = indices;
							indices++;

						}
					}
				}
			}

		} else if(key == "f") {
			face f;
			f.materialName = usemtl; // usemtl gets populated before coming to faces
			f.materialIndex = -1; 

			int v,n,t;
			int numVertices=0;

			while(!sStream.eof()) {

				sStream>>v>>std::ws;
				f.vertex[numVertices] = v-1;
				if(sStream.peek() == '/') {
					sStream.get();
					if(sStream.peek() == '/') {
						sStream.get();
						sStream>>n>>std::ws;
						f.normal[numVertices] = n-1;
					
					} else {
						sStream>>t>>std::ws;
						f.texture[numVertices] = t-1;
						if(sStream.peek() == '/') {
							sStream.get();
							sStream>>n>>std::ws;
							f.normal[numVertices] = n-1;
						}
					}
				}

				numVertices++;
			}

			f.numVertices = numVertices;

			if(!isGroup) {
				m_faces.push_back(f);
			} else {
				m_groups[groupNumber].faces.push_back(f);
			}
				

		} else if(key  == "mtllib") {

			sStream>>m_mtlFile>>std::ws;
			std::string path = "Data/";
			path.append(m_mtlFile);
			mtlLoader(path);

		} else if(key  == "usemtl") {
			sStream>>usemtl>>std::ws;
			mtlFile = true;
		}
		else {
		}
	}

	inFile.close();

	m_groups.size();

	if(mtlFile) {
		for(int i=0; i<m_groups.size(); i++) {
			for(int j=0; j<m_material.size(); j++) {
				if(m_groups[i].materialName.compare(m_material[j].materialName) == 0) {
					m_groups[i].materialIndex = j;
					continue;
				}
			}
		}
	}
		
	delete[] vertexOBJ;
	delete[] texCoordOBJ;
	delete[] normalOBJ;
		
}

void ObjReader::mtlLoader(std::string mtlFile)
{
	std::ifstream inFile(mtlFile.c_str(), std::ifstream::in);
    std::string line, key;

	bool first = 1;
	short materialNumber = -1;

	while(inFile.good() && !inFile.eof() && std::getline(inFile, line))
	{
		key = "";
		std::stringstream sStream(line);
		sStream>>key>>std::ws;

		if(key  == "newmtl") {

			materialNumber++;
			material m;
			m_material.push_back(m);
			sStream>>m_material[materialNumber].materialName>>std::ws;

		} /*else if(key  == "Ka") {

			Vector3 Ka;
			int i=0;
			while(!sStream.eof())
			{
				sStream>>Ka[i]>>std::ws;
				i++;
			}
			m_material[materialNumber].kAmbient = Ka;

		} else if(key  == "Kd") {

			Vector3 Kd;
			int i=0;
			while(!sStream.eof())
			{
				sStream>>Kd[i]>>std::ws;
				i++;
			}
			m_material[materialNumber].kDiffuse = Kd;

		} else if(key  == "Ks") {

			Vector3 Ks;
			int i=0;
			while(!sStream.eof())
			{
				sStream>>Ks[i]>>std::ws;
				i++;
			}
			m_material[materialNumber].kSpecular = Ks;

		} else if (key  == "Tf") {

			Vector3 Tf;
			int i=0;
			while(!sStream.eof())
			{
				sStream>>Tf[i]>>std::ws;
				i++;
			}
			m_material[materialNumber].transmissionFilter = Tf;

		} else if(key  == "Ns") {

			sStream>>m_material[materialNumber].specularExponent>>std::ws;

		} else if(key  == "Ni") {

			sStream>>m_material[materialNumber].refractionIndex>>std::ws;

		}  else if(key  == "illum") {
			
			sStream>>m_material[materialNumber].illuminationType>>std::ws;
			
		}*/else if(key  == "map_Kd") {

			std::string textureFileName;
			sStream>>textureFileName>>std::ws;
			std::string path = "Data/";
			path.append(textureFileName);
			m_material[materialNumber].textureFileName = path;
	

		}
	}
}
