#pragma once
#define _CRT_SECURE_NO_WARNINGS
//--- 메인 함수
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
	GLuint shaderID; //--- 세이더 프로그램 이름
	GLuint vertexShader, fragmentShader; //--- 세이더 객체

	char* filetobuf(const char* file);

	void make_shaderProgram();
	void make_vertexShaders();
	void make_fragmentShaders();
};