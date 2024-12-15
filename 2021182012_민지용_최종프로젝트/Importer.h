#pragma once
#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
//--- 메인 함수
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <gl/glew.h> //--- 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

#include <vector>
#include <random>
#include <utility>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stb_image.h"

using namespace std;


struct VertexData {
	string filename;
	unsigned int texture;

	GLuint VAO;
	GLuint VBOVertex;
	GLuint VBOVertexNormal;
	GLuint VBOVertexTexture;
	GLuint EBOface;

	vector <glm::vec3> vertex;
	vector <glm::vec3> vertexnormal;
	vector <glm::vec2> vertextexture;

	vector <glm::ivec3> face;
	vector <glm::ivec3> vtface;
	vector <glm::ivec3> vnface;

	vector <glm::vec3> color;
};

class Importer {
public:
	vector<VertexData*> VertexBuffers;

	void read_newline(char* str);
	void read_obj_file(const std::string& filename, VertexData* model);
	void rearrangeVerticesByFace(VertexData* vertexData);

	void ReadTexture(const std::string& filePath, unsigned int& texture);
	void ReadObj();

	void SetupMesh(VertexData* VD);
};