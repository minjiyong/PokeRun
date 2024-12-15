#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#pragma comment(lib, "winmm")

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
#include <math.h>
#include <algorithm>

#include "Importer.h"
#include "ShaderProgram.h"

#include <mmsystem.h>
#include <cstdlib>

using namespace std;

#define SOUND_BGM "Sounds\\Pokemon-Heart-Gold-and-Soul-Silver-Route-27.wav"
#define SOUND_MENU_BGM "Sounds\\Pokemon-BlueRed-Route-1.wav"
#define SOUND_RESTART_BGM "Sounds\\Pokemon-Black-and-White-Pokemon-Center.wav"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

enum GameState {
    MAIN_MENU,
    GAME_PLAY,
    RESTART_MENU
};

GameState currentState = MAIN_MENU;

//--- 함수 선언 추가하기
std::pair<float, float> ConvertWinToGL(int x, int y) {
    float mx = ((float)x - (WINDOW_WIDTH / 2)) / (WINDOW_WIDTH / 2); //gl좌표계로 변경
    float my = -((float)y - (WINDOW_HEIGHT / 2)) / (WINDOW_HEIGHT / 2); //gl좌표계로 변경
    return { mx, my };
}

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);

int game_time = 0;
bool timer_on = true;
void TimerFunction(int value);
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
void SpecialKeyboard(int key, int x, int y);
GLvoid Keyboard(unsigned char key, int x, int y);

float distanceFromOrigin(float x, float y) {
    return std::sqrt(x * x + y * y);  // 또는 std::sqrt(std::pow(x, 2) + std::pow(y, 2));
}
float randomValue() {
    return -1.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 2.0f);
}

void initialize();
void initialize_main_menu();
void initialize_restart_menu();
bool checkCollision(glm::vec3 sizeA, glm::vec3 positionA, glm::vec3 sizeB, glm::vec3 positionB);
void make_berry(float z);
void make_map(float z);
void make_obs(float z);
void clearConsole();
void enableANSI();

Importer importer;
ShaderProgram SP;

class Camera {
public:
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    float rotationAngle = 0.0f;

    float fov = 45.0f;
    float aspectRatio = 800.0f / 800.0f;
    float nearClip = 0.1f;
    float farClip = 100.0f;

    void initialize() {
        cameraPos = glm::vec3(0.0f, 15.0f, 30.0f);
        cameraTarget = glm::vec3(0.0f, 0.0f, -10.0f);
        cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

        rotationAngle = 0.0f;

        fov = 100.0f;
        aspectRatio = 800.0f / 800.0f;
        nearClip = 0.1f;
        farClip = 500.0f;
    }

    void initialize_restart() {
        cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
        cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

        rotationAngle = 0.0f;

        fov = 45.0f;
        aspectRatio = 800.0f / 800.0f;
        nearClip = 0.1f;
        farClip = 100.0f;
    }

    void setRotation(float angle) {
        rotationAngle = glm::radians(angle);  // 입력은 각도, 내부에서는 라디안 사용
    }

    void make_camera_perspective() {

        // 카메라의 위치를 회전시키기 위해, 회전 행렬을 적용
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));

        glm::vec4 transCameraPos = rotationMatrix * glm::vec4(cameraPos, 1.0f); // 이동된 카메라 위치

        // 회전된 카메라 위치와 목표 지점을 사용하여 LookAt 행렬 계산
        glm::mat4 view = glm::lookAt(glm::vec3(transCameraPos), cameraTarget, cameraUp);

        glm::mat4 projection = glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip);

        glUniformMatrix4fv(glGetUniformLocation(SP.shaderID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(SP.shaderID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    }

    void make_camera_orbit_xy() {
        // 객체와 카메라가 원점에 위치하도록 설정
        glm::vec3 objectCenter = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 cameraTarget = objectCenter;
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

        // 카메라 초기 위치 설정 (z축 양의 방향으로 배치)
        glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.1f);

        // 객체를 감싸는 직교 투영의 경계 설정
        float left = -1.0f;
        float right = 1.0f;
        float bottom = -1.0f;
        float top = 1.0f;
        float nearClip = 0.1f;
        float farClip = 10.0f;

        // 직교 투영 행렬 계산
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
        glUniformMatrix4fv(glGetUniformLocation(SP.shaderID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glm::mat4 projection = glm::ortho(left, right, bottom, top, nearClip, farClip);
        glUniformMatrix4fv(glGetUniformLocation(SP.shaderID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    }
    // 카메라
};

class Light {
public:
    glm::vec3 position{ 0.0f, 0.0f, 0.1f };     // 광원의 위치
    glm::vec3 color{ 1.0f, 1.0f, 1.0f };        // 광원의 색상
    float intensity{ 1.0f };                    // 광원의 강도

    float rotationAngle = 0.0f;

    void setRotation(float angle) {
        rotationAngle = glm::radians(angle);  // 입력은 각도, 내부에서는 라디안 사용
    }

    void sendToShader() {
        // 광원의 위치를 회전시키기 위해, 회전 행렬을 적용
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec4 rotatedLightPos = rotationMatrix * glm::vec4(position, 1.0f); // 회전된 카메라 위치

        // position uniform 전달
        glUniform3fv(glGetUniformLocation(SP.shaderID, "lightPos"), 1, glm::value_ptr(glm::vec3(rotatedLightPos)));
        // color uniform 전달
        glUniform3fv(glGetUniformLocation(SP.shaderID, "lightColor"), 1, glm::value_ptr(color));
        // intensity uniform 전달
        glUniform1f(glGetUniformLocation(SP.shaderID, "lightIntensity"), intensity);
    }
};

class Object {
public:
    int what{};

    GLuint objVAO{};
    GLuint textureID{};
    string objName{};

    glm::vec3 location{};   // 위치(translate 적용)
    glm::vec3 color{};      // 색상

    glm::vec3 scale{ 1.0f, 1.0f, 1.0f };  // 크기 (디폴트로 1)
    float rotationAngle = 0.0f;  // 회전 각도 (라디안)
    float aniAngle = 0.0f;       // 회전 각도 (라디안)
    int aniDir = 1;       // 애니메이션 방향플래그
    int moving = 0;       // 회전 각도 (라디안)
    int movecount = 0;
    bool animation = true;
    bool jump = false;
    float jumpVelocity = 0.0f;
    float gravity = -0.3f;
    float groundLevel = -15.0f;
    float berry_rotate = 0.0f;
    bool collide = false;

    int num_vertices;

    std::vector<glm::vec3> object_vertices;
    std::vector<glm::vec3> object_vertice_normals;

    std::vector<glm::vec2> object_vertice_textures;

    void initialize_Object(string name, glm::vec3 pos) {
        for (auto& v : importer.VertexBuffers) {
            if (name == v->filename) {
                objVAO = v->VAO;
                //cout << "Object VAO: " << objVAO << endl;
                objName = name;
                location = pos;
                color.r = 1.0f;
                color.g = 1.0f;
                color.b = 1.0f;
                scale = glm::vec3(0.5f, 0.5f, 0.5f);  // 기본 크기 설정
                rotationAngle = 0.0f;  // 기본 회전 각도 설정
                aniAngle = 0.0f;      // 기본 회전 각도 설정
                textureID = v->texture;

                num_vertices = v->vertex.size();
            }
        }
    }

    void move_LEFT() {
        location.x -= 2.0f;
    }
    void move_RIGHT() {
        location.x += 2.0f;
    }

    void setScale(const glm::vec3& newScale) {
        scale = newScale;
    }

    void setRotation(float angle) {
        rotationAngle = glm::radians(angle);  // 입력은 각도, 내부에서는 라디안 사용
    }

    void Aniangle(float angle) {
        aniAngle += glm::radians(angle);  // 입력은 각도, 내부에서는 라디안 사용
    }

    void updateJump() {
        if (jump) {
            location.y += jumpVelocity;   // 속도에 따라 높이 변경
            jumpVelocity += gravity;      // 중력 적용

            if (location.y <= groundLevel) {
                location.y = groundLevel; // 바닥에 도달
                jumpVelocity = 0.0f;      // 속도 초기화
                jump = false;             // 점프 상태 종료
            }
        }
    }

    void startJump(float initialVelocity) {
        if (!jump && location.y <= groundLevel) { // 바닥에서만 점프 가능
            jump = true;
            jumpVelocity = initialVelocity;       // 초기 점프 속도 설정
        }
    }

    void updateJumpBB() {
        if (jump) {
            location.y += jumpVelocity;   // 속도에 따라 높이 변경
            jumpVelocity += gravity;      // 중력 적용

            if (location.y <= -10) {
                location.y = -10; // 바닥에 도달
                jumpVelocity = 0.0f;      // 속도 초기화
                jump = false;             // 점프 상태 종료
            }
        }
    }

    void startJumpBB(float initialVelocity) {
        if (!jump && location.y <= -10) { // 바닥에서만 점프 가능
            jump = true;
            jumpVelocity = initialVelocity;       // 초기 점프 속도 설정
        }
    }

    void make_Color_random() {
        color.r = (float)(rand() % 256) / 255.0f;
        color.g = (float)(rand() % 256) / 255.0f;
        color.b = (float)(rand() % 256) / 255.0f;
    }

    void Draw_OBJ() {

        glUseProgram(SP.shaderID);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(glGetUniformLocation(SP.shaderID, "outTexture"), 0);


        //-------
        glm::mat4 transform_Matrix = glm::mat4(1.0f);

        // 이동
        transform_Matrix = glm::translate(transform_Matrix, location);
        // 애니메이션
        transform_Matrix = glm::rotate(transform_Matrix, aniAngle, glm::vec3(0.0f, 0.0f, 1.0f));
        transform_Matrix = glm::translate(transform_Matrix, glm::vec3(-4.0f, 0.0f, 0.0f));  //--- 원점 맞추기
        // 자전
        transform_Matrix = glm::rotate(transform_Matrix, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        // 축소확대
        transform_Matrix = glm::scale(transform_Matrix, scale);

        unsigned int ObjectTransform = glGetUniformLocation(SP.shaderID, "transform");
        glUniformMatrix4fv(ObjectTransform, 1, GL_FALSE, glm::value_ptr(transform_Matrix));

        // color를 셰이더로 전달
        unsigned int ObjectColor = glGetUniformLocation(SP.shaderID, "fColor");
        glUniform3fv(ObjectColor, 1, glm::value_ptr(color));

        glBindVertexArray(objVAO);
        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
        glBindVertexArray(0);
    }

    void make_Square_Polygon(const std::string& filePath) {
        object_vertices.clear();
        object_vertice_normals.clear();
        object_vertice_textures.clear();

        // 위치 좌표 (-1.0 ~ 1.0 범위로 수정)
        object_vertices.push_back(glm::vec3(1.0f, 1.0f, 0.0f));   // 오른쪽 위
        object_vertices.push_back(glm::vec3(-1.0f, 1.0f, 0.0f));  // 왼쪽 위
        object_vertices.push_back(glm::vec3(-1.0f, -1.0f, 0.0f)); // 왼쪽 아래
        object_vertices.push_back(glm::vec3(1.0f, -1.0f, 0.0f));  // 오른쪽 아래

        // 법선 벡터 (정사각형이 정면을 향한다고 가정)
        object_vertice_normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));

        // 텍스처 좌표 (0.0 ~ 1.0 범위로 설정)
        object_vertice_textures.push_back(glm::vec2(1.0f, 1.0f)); // 오른쪽 위
        object_vertice_textures.push_back(glm::vec2(0.0f, 1.0f)); // 왼쪽 위
        object_vertice_textures.push_back(glm::vec2(0.0f, 0.0f)); // 왼쪽 아래
        object_vertice_textures.push_back(glm::vec2(1.0f, 0.0f)); // 오른쪽 아래

        // 텍스처 생성
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // 텍스처 필터링 및 래핑 설정
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // S축 래핑
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // T축 래핑
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // 이미지 로드
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            // 텍스처 이미지 데이터 업로드
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D); // Mipmap 생성
            stbi_image_free(data); // 이미지 메모리 해제
        }
        else {
            std::cerr << "Failed to load texture at " << filePath << std::endl;
        }
    }

    void Draw_stage_square() {
        // VAO, VBO 생성
        unsigned int objectVAO, objectVBOvertex, objectVBOvertexnormal, objectVBOvertextexture{};
        glGenVertexArrays(1, &objectVAO);
        glGenBuffers(1, &objectVBOvertex);
        glGenBuffers(1, &objectVBOvertexnormal);
        glGenBuffers(1, &objectVBOvertextexture);

        glBindVertexArray(objectVAO);

        glBindBuffer(GL_ARRAY_BUFFER, objectVBOvertex);
        glBufferData(GL_ARRAY_BUFFER, object_vertices.size() * sizeof(glm::vec3), &object_vertices[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, objectVBOvertexnormal);
        glBufferData(GL_ARRAY_BUFFER, object_vertice_normals.size() * sizeof(glm::vec3), &object_vertice_normals[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, objectVBOvertextexture);
        glBufferData(GL_ARRAY_BUFFER, object_vertice_textures.size() * sizeof(glm::vec2), &object_vertice_textures[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);


        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glUseProgram(SP.shaderID);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(glGetUniformLocation(SP.shaderID, "outTexture"), 0);

        glm::mat4 transform_Matrix = glm::mat4(1.0f);
        unsigned int ObjectTransform = glGetUniformLocation(SP.shaderID, "transform");
        glUniformMatrix4fv(ObjectTransform, 1, GL_FALSE, glm::value_ptr(transform_Matrix));

        // 이동
        transform_Matrix = glm::translate(transform_Matrix, location);
        // 자전
        transform_Matrix = glm::rotate(transform_Matrix, rotationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
        // 축소확대
        transform_Matrix = glm::scale(transform_Matrix, scale);

        ObjectTransform = glGetUniformLocation(SP.shaderID, "parent_transform");
        glUniformMatrix4fv(ObjectTransform, 1, GL_FALSE, glm::value_ptr(transform_Matrix));

        // 궤적을 그리기 위해 필요한 셰이더와 행렬 설정
        unsigned int ObjectColor = glGetUniformLocation(SP.shaderID, "fColor");
        glUniform3fv(ObjectColor, 1, glm::value_ptr(color));

        glBindVertexArray(objectVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);

        // 메모리 해제
        glDeleteBuffers(1, &objectVBOvertex);
        glDeleteBuffers(1, &objectVBOvertexnormal);
        glDeleteVertexArrays(1, &objectVAO);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
};

Camera camera;
Light light;

Object light_object;

Object xyz_line[3];
Object bottom_stage;
Object start_object;
Object restart_object;

vector<Object> pokemon;
vector<Object> pokemon_bb;
vector<Object> sitrusberry;
vector<Object> pechaberry;
vector<Object> oranberry;

vector<Object> sitrusberry_bb;
vector<Object> pechaberry_bb;
vector<Object> oranberry_bb;

vector<Object> map;

vector<Object> obs;
vector<Object> obs_bb;


bool timer{ true };

bool draw_only_line{ false };
bool draw_enable{ true };

int radian{ 0 };
int direction{ 1 };
float light_radian{ 0.0f };

float now_x{};
float now_y{};

float speed = 1.0f;

int score = 0;
int HP = 3;

int berry_count = 300;
int berry_make_lv = 1;
int min_obs_make_lv = 9;
int max_obs_make_lv = 3;
int map_count = 999;

bool guard = false;
int guard_count = 0;

bool jump_able = true;
int jump_able_count = 0;

int main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
    srand(static_cast<int>(time(NULL)));
    enableANSI();

    //--- 윈도우 생성하기
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("PokeRun!");
    //cout << "윈도우 생성됨" << endl;

    //--- GLEW 초기화하기
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) // glew 초기화
    {
        //std::cerr << "Unable to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }
    //else
    //    std::cout << "GLEW Initialized" << std::endl;

    importer.ReadObj();
    //for (auto& v : importer.VertexBuffers) {
    //   cout << "Filename: " << v->filename << ", VAO: " << v->VAO << ", Vertices: " << v->vertex.size() << ", Colors: " << v->color.size() << endl;
    //}

    SP.make_vertexShaders(); //--- 버텍스 세이더 만들기
    SP.make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
    SP.make_shaderProgram();

    //initialize();
    initialize_main_menu();

    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);

    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);
    glutKeyboardFunc(Keyboard);
    glutSpecialFunc(SpecialKeyboard);

    glutMainLoop();
}

//--- 그리기 콜백 함수
GLvoid drawScene()
{
    switch (currentState) {
    case MAIN_MENU:
        //--- 변경된 배경색 설정
        glClearColor(0.792f, 0.761f, 0.306f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(SP.shaderID);

        camera.make_camera_orbit_xy();
        glEnable(GL_DEPTH_TEST);
        start_object.Draw_stage_square();

        glutSwapBuffers(); //--- 화면에 출력하기
        break;
    case RESTART_MENU:
        //--- 변경된 배경색 설정
        glClearColor(0.792f, 0.761f, 0.306f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(SP.shaderID);

        camera.make_camera_orbit_xy();
        glEnable(GL_DEPTH_TEST);
        restart_object.Draw_stage_square();

        glutSwapBuffers(); //--- 화면에 출력하기
        break;
    case GAME_PLAY:
        //--- 변경된 배경색 설정
        glClearColor(0.3f, 0.5f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //--- 렌더링 파이프라인에 세이더 불러오기
        glUseProgram(SP.shaderID);

        // 음면제거, 라인 따기
        if (draw_enable) {
            glEnable(GL_DEPTH_TEST);
            glCullFace(GL_BACK);
            glFrontFace(GL_CCW);
        }
        else if (!draw_enable) glDisable(GL_CULL_FACE);


        camera.make_camera_perspective();
        light.sendToShader();

        // 플레이어 그리기 (변환 적용)
        for (auto& v : pokemon) {
            v.Draw_OBJ();
        }
        
        // 열매 그리기 (변환 적용)
        for (auto& v : sitrusberry) {
            v.Draw_OBJ();
        }
        for (auto& v : pechaberry) {
            v.Draw_OBJ();
        }
        for (auto& v : oranberry) {
            v.Draw_OBJ();
        }
        
        // 바닥 그리기
        for (auto& v : map) {
            v.Draw_OBJ();
        }
        // 장애물 그리기
        for (auto& v : obs) {
            v.Draw_OBJ();
        }

        // 충돌 박스 그리기 (디버깅용)
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //for (auto& v : pokemon_bb) {
        //   v.Draw_OBJ();
        //}
        //for (auto& v : sitrusberry_bb) {
        //   v.Draw_OBJ();
        //}
        //for (auto& v : pechaberry_bb) {
        //   v.Draw_OBJ();
        //}
        //for (auto& v : oranberry_bb) {
        //   v.Draw_OBJ();
        //}
        //for (auto& v : obs_bb) {
        //    v.Draw_OBJ();
        //}
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


        glutSwapBuffers(); //--- 화면에 출력하기
        break;
    }
}
//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h)
{
    glViewport(0, 0, w, h);
}

void TimerFunction(int value) {

    // 카메라 및 플레이어 이동
    pokemon[0].location.z -= speed;
    pokemon_bb[0].location.z -= speed;
    camera.cameraPos.z -= speed;
    camera.cameraTarget.z -= speed;
    light.position.z -= speed;

    berry_count++;
    map_count++;

    if (pokemon[0].animation) {      //--- 뒤뚱뒤뚱
        if (game_time >= 10) {
            pokemon[0].aniDir = -1;
        }
        else if (game_time <= -10) {
            pokemon[0].aniDir = 1;
        }
        game_time += pokemon[0].aniDir;
        pokemon[0].Aniangle(pokemon[0].aniDir * 0.7);
    }

    if (pokemon[0].moving == -1) {   //--- 좌우이동
        if (pokemon[0].movecount >= 8) {
            pokemon[0].aniAngle = 0;
            pokemon[0].moving = 0;
            pokemon[0].movecount = 0;
            pokemon[0].animation = true;
            game_time = 0;
            pokemon[0].aniDir = -1;
        }
        else {
            pokemon[0].aniAngle = 0.2;
            pokemon[0].movecount++;
            if (pokemon[0].location.x > -16) {
                pokemon[0].move_LEFT();
                pokemon_bb[0].move_LEFT();
            }
        }
    }
    else if (pokemon[0].moving == 1) {
        if (pokemon[0].movecount >= 8) {
            pokemon[0].aniAngle = 0;
            pokemon[0].moving = 0;
            pokemon[0].movecount = 0;
            pokemon[0].animation = true;
            game_time = 0;
            pokemon[0].aniDir = 1;
        }
        else {
            pokemon[0].aniAngle = -0.2;
            pokemon[0].movecount++;
            if (pokemon[0].location.x < 16) {
                pokemon[0].move_RIGHT();
                pokemon_bb[0].move_RIGHT();
            }
        }
    }

    pokemon[0].updateJump();
    pokemon_bb[0].updateJumpBB();

    for (int i = 0; i < sitrusberry.size(); ++i) {
        sitrusberry[i].berry_rotate += 5;
        sitrusberry[i].setRotation(sitrusberry[i].berry_rotate);
    }
    for (int i = 0; i < pechaberry.size(); ++i) {
        pechaberry[i].berry_rotate += 5;
        pechaberry[i].setRotation(pechaberry[i].berry_rotate);
    }
    for (int i = 0; i < oranberry.size(); ++i) {
        oranberry[i].berry_rotate += 5;
        oranberry[i].setRotation(oranberry[i].berry_rotate);
    }

    int obs_rand = rand() % 10;
    if (berry_count % 10 == 0) {
        if (min_obs_make_lv <= obs_rand && obs_rand <= 9) {
            make_obs(-berry_count);
        }
        else if (obs_rand <= max_obs_make_lv) {
            make_berry(-berry_count);
        }
    }

    if (map_count % 1000 == 0) {
        make_map(-(map_count * 2));
    }

    if (guard) {
		guard_count++;
        if (guard_count > 50) {
            guard = false;
        }
    }

    if (!jump_able) {
        jump_able_count++;
        if (jump_able_count > 300) {
            jump_able = true;
        }
    }

    for (auto it = sitrusberry_bb.begin(); it != sitrusberry_bb.end(); ) {
        if (checkCollision(pokemon_bb[0].scale, pokemon_bb[0].location, it->scale, it->location)) {
            if (it->scale.x > 3) {
                score += 500;
                if (HP > 0) {
                    clearConsole();
                    printf("HP: %d\n", HP);
                    printf("SCORE: %d\n", score);
                }
            }
            else {
                score += 10;
                if (HP > 0) {
                    clearConsole();
                    printf("HP: %d\n", HP);
                    printf("SCORE: %d\n", score);
                }
            }
            size_t index = std::distance(sitrusberry_bb.begin(), it);
            sitrusberry.erase(sitrusberry.begin() + index);
            it = sitrusberry_bb.erase(it);
        }
        else if (it->location.z > pokemon_bb[0].location.z + 200) {
            size_t index = std::distance(sitrusberry_bb.begin(), it);
            sitrusberry.erase(sitrusberry.begin() + index);
            it = sitrusberry_bb.erase(it);
        }
        else {
            ++it;
        }
    }
    for (auto it = pechaberry_bb.begin(); it != pechaberry_bb.end(); ) {
        if (checkCollision(pokemon_bb[0].scale, pokemon_bb[0].location, it->scale, it->location)) {
            if (it->scale.x > 3) {
                HP++;
                if (HP > 0) {
                    clearConsole();
                    printf("HP: %d\n", HP);
                    printf("SCORE: %d\n", score);
                }
            }
            else {
                int randnum = rand() % 2;
                if (randnum == 0) score += 200;
                else if (randnum == 1) score -= 200;
                if (HP > 0) {
                    clearConsole();
                    printf("HP: %d\n", HP);
                    printf("SCORE: %d\n", score);
                }
            }
            size_t index = std::distance(pechaberry_bb.begin(), it);
            pechaberry.erase(pechaberry.begin() + index);
            it = pechaberry_bb.erase(it);
        }
        else if (it->location.z > pokemon_bb[0].location.z + 200) {
            size_t index = std::distance(pechaberry_bb.begin(), it);
            pechaberry.erase(pechaberry.begin() + index);
            it = pechaberry_bb.erase(it);
        }
        else {
            ++it;
        }
    }
    for (auto it = oranberry_bb.begin(); it != oranberry_bb.end(); ) {
        if (checkCollision(pokemon_bb[0].scale, pokemon_bb[0].location, it->scale, it->location)) {
            if (it->scale.x > 3) {
                score = 0;
                if (HP > 0) {
                    clearConsole();
                    printf("HP: %d\n", HP);
                    printf("SCORE: %d\n", score);
                }
            }
            else {
                jump_able = false;
                jump_able_count = 0;
            }
            size_t index = std::distance(oranberry_bb.begin(), it);
            oranberry.erase(oranberry.begin() + index);
            it = oranberry_bb.erase(it);
        }
        else if (it->location.z > pokemon_bb[0].location.z + 200) {
            size_t index = std::distance(oranberry_bb.begin(), it);
            oranberry.erase(oranberry.begin() + index);
            it = oranberry_bb.erase(it);
        }
        else {
            ++it;
        }
    }

    for (int i = 0; i < obs_bb.size(); ++i) {
        if (checkCollision(pokemon_bb[0].scale, pokemon_bb[0].location, obs_bb[i].scale, obs_bb[i].location)
            && !guard) {
            HP--;
            guard = true;
            guard_count = 0;
            if (HP > 0) {
                clearConsole();
                printf("HP: %d\n", HP);
                printf("SCORE: %d\n", score);
            }
            else if (HP <= 0) {
                currentState = RESTART_MENU;
                initialize_restart_menu();
                timer_on = false;
                clearConsole();
                printf("피카츄는 몬스터볼로 돌아갔습니다...\n");
                printf("SCORE: %d\n", score);
                printf("'R'을 눌러 재시작하세요");
            }
        }
    }

    for (auto it = obs_bb.begin(); it != obs_bb.end(); ) {
        if (it->location.z > pokemon_bb[0].location.z + 200) {
            size_t index = std::distance(obs_bb.begin(), it);
            obs.erase(obs.begin() + index);
            it = obs_bb.erase(it);
        }
        else {
            ++it;
        }
    }

    glutPostRedisplay();
    if (timer_on) glutTimerFunc(17, TimerFunction, 1);
}

void Mouse(int button, int state, int x, int y) {

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        auto mouse = ConvertWinToGL(x, y);
        now_x = mouse.first;
        now_y = mouse.second;
    }

    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        auto mouse = ConvertWinToGL(x, y);

        glutPostRedisplay();
    }
}

void Motion(int x, int y) {

    auto mouse = ConvertWinToGL(x, y);
    float deltaX{ mouse.first - now_x };
    float deltaY{ mouse.second - now_y };

    now_x = mouse.first;
    now_y = mouse.second;

    glutPostRedisplay();
}

GLvoid Keyboard(unsigned char key, int x, int y) {

    switch (currentState) {
    case MAIN_MENU:
        currentState = GAME_PLAY;
        initialize();
        if (HP > 0) {
            clearConsole();
            printf("HP: %d\n", HP);
            printf("SCORE: %d\n", score);
        }
        glutTimerFunc(17, TimerFunction, 1);
        glutPostRedisplay();
        break;
    case RESTART_MENU:
        if (key == 'r' || key == 'R') {
            currentState = GAME_PLAY;
            initialize();

            timer_on = true;
            game_time = 0;
            speed = 1.0f;

            score = 0;
            HP = 3;

            berry_count = 300;
            map_count = 999;

            guard = false;
            guard_count = 0;

            jump_able = true;
            jump_able_count = 0;

            berry_make_lv = 1;
            min_obs_make_lv = 9;
            max_obs_make_lv = 3;

            if (HP > 0) {
                clearConsole();
                printf("HP: %d\n", HP);
                printf("SCORE: %d\n", score);
            }

            glutTimerFunc(17, TimerFunction, 1);
        }
        glutPostRedisplay();
        break;
    case GAME_PLAY:
        if (key == 'q' || key == 'Q') {
            glutDestroyWindow(true);
        }
        else if (key == 'a' || key == 'A') {
            if (pokemon[0].moving == 0) {
                pokemon[0].animation = false;
                pokemon[0].moving = -1;
            }
        }
        else if (key == 'd' || key == 'D') {
            if (pokemon[0].moving == 0) {
                pokemon[0].animation = false;
                pokemon[0].moving = 1;
            }
        }
        else if (key == 'j' || key == 'J') {
            if (jump_able) {
                pokemon[0].startJump(4.0f);
                pokemon_bb[0].startJumpBB(4.0f);
            }
        }
        glutPostRedisplay();
        break;
    }
}

void SpecialKeyboard(int key, int x, int y) {
    if (key == GLUT_KEY_F1) {
    }

    glutPostRedisplay();
}

void initialize() {
    PlaySound(NULL, NULL, SND_PURGE);

    camera.initialize();
    camera.make_camera_perspective();
    light.position = glm::vec3 { 0.0f, 50.0f, 0.0f };
    light.sendToShader();

    pokemon.clear();
    pokemon_bb.clear();

    sitrusberry.clear();
    sitrusberry_bb.clear();

    pechaberry.clear();
    pechaberry_bb.clear();

    oranberry.clear();
    oranberry_bb.clear();

    obs.clear();
    obs_bb.clear();

    map.clear();

    Object temp_obj;
    temp_obj.initialize_Object("pikachu.obj", { 0.0f, -15.0f, 0.0f });
    temp_obj.setScale({ 0.3f, 0.3f, 0.3f });
    temp_obj.setRotation(180);
    pokemon.push_back(temp_obj);

    temp_obj.initialize_Object("cube.obj", { 4.0f, -10.0f, 0.0f });
    temp_obj.setScale({ 0.3f * 10, 0.3f * 10, 0.3f * 10 });
    pokemon_bb.push_back(temp_obj);

    temp_obj.initialize_Object("cube.obj", { 3.0f, -15.0f, 0.0f });
    temp_obj.setScale({ 32.0f, 0.001f, 1000.0f });
    temp_obj.color = glm::vec3(0.2f, 1.0f, 0.2f);
    map.push_back(temp_obj);

    PlaySound(TEXT(SOUND_BGM), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
}

void initialize_main_menu() {
    PlaySound(NULL, NULL, SND_PURGE);
    
    camera.initialize();
    light.position = glm::vec3{ 0.0f, 0.0f, 0.1f };
    light.sendToShader();
    start_object.make_Square_Polygon("start_mode.jpg");
    start_object.color = glm::vec3(1.0f, 1.0f, 1.0f);
    start_object.setScale({ 1.0f, 0.753f, 1.0f });

    PlaySound(TEXT(SOUND_MENU_BGM), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
}

void initialize_restart_menu() {
    PlaySound(NULL, NULL, SND_PURGE);

    camera.initialize_restart();
    light.position = glm::vec3{ 0.0f, 0.0f, 0.1f };
    light.sendToShader();
    restart_object.make_Square_Polygon("restart_mode.jpg");
    restart_object.color = glm::vec3(1.0f, 1.0f, 1.0f);
    restart_object.setScale({ 1.0f, 1.0f, 1.0f });

    PlaySound(TEXT(SOUND_RESTART_BGM), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
}

bool checkCollision(glm::vec3 sizeA, glm::vec3 positionA, glm::vec3 sizeB, glm::vec3 positionB) {

    glm::vec3 halfSizeA = sizeA * 0.5f;
    glm::vec3 minA = positionA - halfSizeA;
    glm::vec3 maxA = positionA + halfSizeA;

    glm::vec3 halfSizeB = sizeB * 0.5f;
    glm::vec3 minB = positionB - halfSizeB;
    glm::vec3 maxB = positionB + halfSizeB;

    return (minA.x <= maxB.x && maxA.x >= minB.x) &&
        (minA.y <= maxB.y && maxA.y >= minB.y) &&
        (minA.z <= maxB.z && maxA.z >= minB.z);
}

void make_berry(float z) {

    Object temp_obj;
    int temp_num = rand() % 100;
    int temp_num2 = rand() % 3;
    float x = 0.0f;

    if (temp_num2 == 0) x = -13.0f;
    else if (temp_num2 == 1) x = 3.0f;
    else if (temp_num2 == 2) x = 19.0f;

    int randnum = rand() % 15;
    float y = -10.0f;
    float scale_x = 0.1f;
    float scale_y = 0.1f;
    float scale_z = 0.1f;

    if (randnum == 14) {
        y = 20.0f;
        scale_x = 0.2f;
        scale_y = 0.2f;
        scale_z = 0.2f;
    }

    if (berry_make_lv == 1) {
        if (0 <= temp_num && temp_num <= 59) {
            temp_obj.initialize_Object("SitrusBerry.obj", { x, y, z });
            temp_obj.setScale({ scale_x, scale_y, scale_z });
            sitrusberry.push_back(temp_obj);

            temp_obj.initialize_Object("cube.obj", { x, y, z });
            temp_obj.setScale({ scale_x * 30, scale_y * 30, scale_z * 30 });
            sitrusberry_bb.push_back(temp_obj);
        }
        else if (60 <= temp_num && temp_num <= 89) {
            temp_obj.initialize_Object("PechaBerry.obj", { x, y, z });
            temp_obj.setScale({ scale_x, scale_y, scale_z });
            pechaberry.push_back(temp_obj);

            temp_obj.initialize_Object("cube.obj", { x, y, z });
            temp_obj.setScale({ scale_x * 30, scale_y * 30, scale_z * 30 });
            pechaberry_bb.push_back(temp_obj);
        }
        else if (90 <= temp_num && temp_num <= 99) {
            temp_obj.initialize_Object("OranBerry.obj", { x, y, z });
            temp_obj.setScale({ scale_x, scale_y, scale_z });
            oranberry.push_back(temp_obj);

            temp_obj.initialize_Object("cube.obj", { x, y, z });
            temp_obj.setScale({ scale_x * 30, scale_y * 30, scale_z * 30 });
            oranberry_bb.push_back(temp_obj);
        }
    }
    else if (berry_make_lv == 2) {
        if (0 <= temp_num && temp_num <= 54) {
            temp_obj.initialize_Object("SitrusBerry.obj", { x, y, z });
            temp_obj.setScale({ scale_x, scale_y, scale_z });
            sitrusberry.push_back(temp_obj);

            temp_obj.initialize_Object("cube.obj", { x, y, z });
            temp_obj.setScale({ scale_x * 30, scale_y * 30, scale_z * 30 });
            sitrusberry_bb.push_back(temp_obj);
        }
        else if (55 <= temp_num && temp_num <= 79) {
            temp_obj.initialize_Object("PechaBerry.obj", { x, y, z });
            temp_obj.setScale({ scale_x, scale_y, scale_z });
            pechaberry.push_back(temp_obj);

            temp_obj.initialize_Object("cube.obj", { x, y, z });
            temp_obj.setScale({ scale_x * 30, scale_y * 30, scale_z * 30 });
            pechaberry_bb.push_back(temp_obj);
        }
        else if (80 <= temp_num && temp_num <= 99) {
            temp_obj.initialize_Object("OranBerry.obj", { x, y, z });
            temp_obj.setScale({ scale_x, scale_y, scale_z });
            oranberry.push_back(temp_obj);

            temp_obj.initialize_Object("cube.obj", { x, y, z });
            temp_obj.setScale({ scale_x * 30, scale_y * 30, scale_z * 30 });
            oranberry_bb.push_back(temp_obj);
        }
    }
    else if (berry_make_lv == 3) {
        if (0 <= temp_num && temp_num <= 49) {
            temp_obj.initialize_Object("SitrusBerry.obj", { x, y, z });
            temp_obj.setScale({ scale_x, scale_y, scale_z });
            sitrusberry.push_back(temp_obj);

            temp_obj.initialize_Object("cube.obj", { x, y, z });
            temp_obj.setScale({ scale_x * 30, scale_y * 30, scale_z * 30 });
            sitrusberry_bb.push_back(temp_obj);
        }
        else if (50 <= temp_num && temp_num <= 69) {
            temp_obj.initialize_Object("PechaBerry.obj", { x, y, z });
            temp_obj.setScale({ scale_x, scale_y, scale_z });
            pechaberry.push_back(temp_obj);

            temp_obj.initialize_Object("cube.obj", { x, y, z });
            temp_obj.setScale({ scale_x * 30, scale_y * 30, scale_z * 30 });
            pechaberry_bb.push_back(temp_obj);
        }
        else if (70 <= temp_num && temp_num <= 99) {
            temp_obj.initialize_Object("OranBerry.obj", { x, y, z });
            temp_obj.setScale({ scale_x, scale_y, scale_z });
            oranberry.push_back(temp_obj);

            temp_obj.initialize_Object("cube.obj", { x, y, z });
            temp_obj.setScale({ scale_x * 30, scale_y * 30, scale_z * 30 });
            oranberry_bb.push_back(temp_obj);
        }
    }
    else if (berry_make_lv == 4) {
        if (0 <= temp_num && temp_num <= 29) {
            temp_obj.initialize_Object("SitrusBerry.obj", { x, y, z });
            temp_obj.setScale({ scale_x, scale_y, scale_z });
            sitrusberry.push_back(temp_obj);

            temp_obj.initialize_Object("cube.obj", { x, y, z });
            temp_obj.setScale({ scale_x * 30, scale_y * 30, scale_z * 30 });
            sitrusberry_bb.push_back(temp_obj);
        }
        else if (30 <= temp_num && temp_num <= 59) {
            temp_obj.initialize_Object("PechaBerry.obj", { x, y, z });
            temp_obj.setScale({ scale_x, scale_y, scale_z });
            pechaberry.push_back(temp_obj);

            temp_obj.initialize_Object("cube.obj", { x, y, z });
            temp_obj.setScale({ scale_x * 30, scale_y * 30, scale_z * 30 });
            pechaberry_bb.push_back(temp_obj);
        }
        else if (60 <= temp_num && temp_num <= 99) {
            temp_obj.initialize_Object("OranBerry.obj", { x, y, z });
            temp_obj.setScale({ scale_x, scale_y, scale_z });
            oranberry.push_back(temp_obj);

            temp_obj.initialize_Object("cube.obj", { x, y, z });
            temp_obj.setScale({ scale_x * 30, scale_y * 30, scale_z * 30 });
            oranberry_bb.push_back(temp_obj);
        }
    }
    else if (berry_make_lv == 5) {
        if (0 <= temp_num && temp_num <= 24) {
            temp_obj.initialize_Object("SitrusBerry.obj", { x, y, z });
            temp_obj.setScale({ scale_x, scale_y, scale_z });
            sitrusberry.push_back(temp_obj);

            temp_obj.initialize_Object("cube.obj", { x, y, z });
            temp_obj.setScale({ scale_x * 30, scale_y * 30, scale_z * 30 });
            sitrusberry_bb.push_back(temp_obj);
        }
        else if (25 <= temp_num && temp_num <= 49) {
            temp_obj.initialize_Object("PechaBerry.obj", { x, y, z });
            temp_obj.setScale({ scale_x, scale_y, scale_z });
            pechaberry.push_back(temp_obj);

            temp_obj.initialize_Object("cube.obj", { x, y, z });
            temp_obj.setScale({ scale_x * 30, scale_y * 30, scale_z * 30 });
            pechaberry_bb.push_back(temp_obj);
        }
        else if (50 <= temp_num && temp_num <= 99) {
            temp_obj.initialize_Object("OranBerry.obj", { x, y, z });
            temp_obj.setScale({ scale_x, scale_y, scale_z });
            oranberry.push_back(temp_obj);

            temp_obj.initialize_Object("cube.obj", { x, y, z });
            temp_obj.setScale({ scale_x * 30, scale_y * 30, scale_z * 30 });
            oranberry_bb.push_back(temp_obj);
        }
        }
}

void make_map(float z) {

    if (berry_make_lv < 5) berry_make_lv++;
    if (max_obs_make_lv < 5) max_obs_make_lv++;
    if (8 < min_obs_make_lv) min_obs_make_lv--;

    Object temp_obj;
    temp_obj.initialize_Object("cube.obj", { 3.0f, -15.0f, z });
    temp_obj.setScale({ 32.0f, 0.001f, 1000.0f });
    temp_obj.make_Color_random();
    map.push_back(temp_obj);
}

void make_obs(float z) {

	int randnum = rand() % 6;
	Object temp_obj;
	if (randnum == 0) {
		temp_obj.initialize_Object("Cliff.obj", { -13.0f, -15.0f, z });
        temp_obj.setScale({ 3.0f, 3.0f, 3.0f });
		obs.push_back(temp_obj);
        temp_obj.initialize_Object("cube.obj", { -13.0f, -5.0f, z });
        temp_obj.setScale({ 3.0f * 3, 3.0f * 3, 3.0f });
        obs_bb.push_back(temp_obj);
	}
	else if (randnum == 1) {
		temp_obj.initialize_Object("Cliff.obj", { 3.0f, -15.0f, z });
        temp_obj.setScale({ 3.0f, 3.0f, 3.0f });
		obs.push_back(temp_obj);
		temp_obj.initialize_Object("cube.obj", { 3.0f, -5.0f, z });
        temp_obj.setScale({ 3.0f * 3, 3.0f * 3, 3.0f });
        obs_bb.push_back(temp_obj);
	}
	else if (randnum == 2) {
		temp_obj.initialize_Object("Cliff.obj", { 19.0f, -15.0f, z });
        temp_obj.setScale({ 3.0f, 3.0f, 3.0f });
		obs.push_back(temp_obj);
		temp_obj.initialize_Object("cube.obj", { 19.0f, -5.0f, z });
        temp_obj.setScale({ 3.0f * 3, 3.0f * 3, 3.0f });
        obs_bb.push_back(temp_obj);
	}
	else if (randnum == 3) {
		temp_obj.initialize_Object("Cliff.obj", { -13.0f, -15.0f, z });
        temp_obj.setScale({ 3.0f, 3.0f, 3.0f });
		obs.push_back(temp_obj);
        temp_obj.initialize_Object("cube.obj", { -13.0f, -5.0f, z });
        temp_obj.setScale({ 3.0f * 3, 3.0f * 3, 3.0f });
        obs_bb.push_back(temp_obj);

		temp_obj.initialize_Object("Cliff.obj", { 3.0f, -15.0f, z });
        temp_obj.setScale({ 3.0f, 3.0f, 3.0f });
		obs.push_back(temp_obj);
        temp_obj.initialize_Object("cube.obj", { 3.0f, -5.0f, z });
        temp_obj.setScale({ 3.0f * 3, 3.0f * 3, 3.0f });
        obs_bb.push_back(temp_obj);
	}
	else if (randnum == 4) {
		temp_obj.initialize_Object("Cliff.obj", { 3.0f, -15.0f, z });
        temp_obj.setScale({ 3.0f, 3.0f, 3.0f });
		obs.push_back(temp_obj);
        temp_obj.initialize_Object("cube.obj", { 3.0f, -5.0f, z });
        temp_obj.setScale({ 3.0f * 3, 3.0f * 3, 3.0f });
        obs_bb.push_back(temp_obj);

		temp_obj.initialize_Object("Cliff.obj", { 19.0f, -15.0f, z });
        temp_obj.setScale({ 3.0f, 3.0f, 3.0f });
		obs.push_back(temp_obj);
        temp_obj.initialize_Object("cube.obj", { 19.0f, -5.0f, z });
        temp_obj.setScale({ 3.0f * 3, 3.0f * 3, 3.0f });
        obs_bb.push_back(temp_obj);
	}
	else if (randnum == 5) {
		temp_obj.initialize_Object("Cliff.obj", { -13.0f, -15.0f, z });
        temp_obj.setScale({ 3.0f, 3.0f, 3.0f });
		obs.push_back(temp_obj);
        temp_obj.initialize_Object("cube.obj", { -13.0f, -5.0f, z });
        temp_obj.setScale({ 3.0f * 3, 3.0f * 3, 3.0f });
        obs_bb.push_back(temp_obj);

		temp_obj.initialize_Object("Cliff.obj", { 3.0f, -15.0f, z });
        temp_obj.setScale({ 3.0f, 3.0f, 3.0f });
		obs.push_back(temp_obj);
        temp_obj.initialize_Object("cube.obj", { 3.0f, -5.0f, z });
        temp_obj.setScale({ 3.0f * 3, 3.0f * 3, 3.0f });
        obs_bb.push_back(temp_obj);

		temp_obj.initialize_Object("Cliff.obj", { 19.0f, -15.0f, z });
        temp_obj.setScale({ 3.0f, 3.0f, 3.0f });
		obs.push_back(temp_obj);
        temp_obj.initialize_Object("cube.obj", { 19.0f, -5.0f, z });
        temp_obj.setScale({ 3.0f * 3, 3.0f * 3, 3.0f });
        obs_bb.push_back(temp_obj);
	}
}

void clearConsole() {
    std::cout << "\033[2J\033[1;1H";
}

void enableANSI() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}