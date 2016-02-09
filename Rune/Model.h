#ifndef MODEL_H
#define MODEL_H

#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <stdio.h>

#include "glew.h"
#include "GLU.h" //<gl/GLU.h>
#include "GL.h" //<gl/GL.h>
#include "glm/glm.hpp"

#include "Shader.h"
#include <vector>

using std::string;
using std::vector;

class Model
{
public:
	Model();
	~Model();
	static Model * loadModel(string fileName);
	static Model * loadHeightFieldModel(float * heightData, int width, int length);
	int render();
	int getNumVertices() { return numVertices; }
	int getNumTriangles() { return numVertices/3; }
	GLuint getVertexBuffer() { return vertexBuffer; }
	GLuint getUVBuffer() { return uvBuffer; }
	GLuint getNormalBuffer() { return normalBuffer; }
	void setRenderMode(GLenum x) { renderMode = x; }
	void setRenderUVs(bool x) { renderUVs = x; };
	void setRenderNormals(bool x) { renderNormals = x; }
private:
	static Model * loadOBJ(FILE * f);
	static Model * loadErrorModel();
	static Model * loadQuadModel();
	static Model * loadOceanModel();
	#define activeShader Shader::getActiveShader()
	int numVertices;

	bool renderUVs, renderNormals;
	GLenum renderMode;
	GLuint vertexBuffer;
	GLuint uvBuffer;
	GLuint normalBuffer;
};

#endif