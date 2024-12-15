#pragma once
#define _CRT_SECURE_NO_WARNINGS
//--- ���� �Լ�
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

#include <vector>
#include <random>
#include <utility>
#include <string>

using namespace std;

class ShaderProgram {
public:
	GLuint shaderID; //--- ���̴� ���α׷� �̸�
	GLuint vertexShader, fragmentShader; //--- ���̴� ��ü

	char* filetobuf(const char* file);

	void make_shaderProgram();
	void make_vertexShaders();
	void make_fragmentShaders();
};