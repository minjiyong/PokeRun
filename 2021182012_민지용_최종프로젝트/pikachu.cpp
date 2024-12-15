#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#pragma comment(lib, "winmm")

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

//--- �Լ� ���� �߰��ϱ�
std::pair<float, float> ConvertWinToGL(int x, int y) {
    float mx = ((float)x - (WINDOW_WIDTH / 2)) / (WINDOW_WIDTH / 2); //gl��ǥ��� ����
    float my = -((float)y - (WINDOW_HEIGHT / 2)) / (WINDOW_HEIGHT / 2); //gl��ǥ��� ����
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
    return std::sqrt(x * x + y * y);  // �Ǵ� std::sqrt(std::pow(x, 2) + std::pow(y, 2));
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
        rotationAngle = glm::radians(angle);  // �Է��� ����, ���ο����� ���� ���
    }

    void make_camera_perspective() {

        // ī�޶��� ��ġ�� ȸ����Ű�� ����, ȸ�� ����� ����
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));

        glm::vec4 transCameraPos = rotationMatrix * glm::vec4(cameraPos, 1.0f); // �̵��� ī�޶� ��ġ

        // ȸ���� ī�޶� ��ġ�� ��ǥ ������ ����Ͽ� LookAt ��� ���
        glm::mat4 view = glm::lookAt(glm::vec3(transCameraPos), cameraTarget, cameraUp);

        glm::mat4 projection = glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip);

        glUniformMatrix4fv(glGetUniformLocation(SP.shaderID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(SP.shaderID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    }

    void make_camera_orbit_xy() {
        // ��ü�� ī�޶� ������ ��ġ�ϵ��� ����
        glm::vec3 objectCenter = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 cameraTarget = objectCenter;
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

        // ī�޶� �ʱ� ��ġ ���� (z�� ���� �������� ��ġ)
        glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.1f);

        // ��ü�� ���δ� ���� ������ ��� ����
        float left = -1.0f;
        float right = 1.0f;
        float bottom = -1.0f;
        float top = 1.0f;
        float nearClip = 0.1f;
        float farClip = 10.0f;

        // ���� ���� ��� ���
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
        glUniformMatrix4fv(glGetUniformLocation(SP.shaderID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glm::mat4 projection = glm::ortho(left, right, bottom, top, nearClip, farClip);
        glUniformMatrix4fv(glGetUniformLocation(SP.shaderID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    }
    // ī�޶�
};

class Light {
public:
    glm::vec3 position{ 0.0f, 0.0f, 0.1f };     // ������ ��ġ
    glm::vec3 color{ 1.0f, 1.0f, 1.0f };        // ������ ����
    float intensity{ 1.0f };                    // ������ ����

    float rotationAngle = 0.0f;

    void setRotation(float angle) {
        rotationAngle = glm::radians(angle);  // �Է��� ����, ���ο����� ���� ���
    }

    void sendToShader() {
        // ������ ��ġ�� ȸ����Ű�� ����, ȸ�� ����� ����
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec4 rotatedLightPos = rotationMatrix * glm::vec4(position, 1.0f); // ȸ���� ī�޶� ��ġ

        // position uniform ����
        glUniform3fv(glGetUniformLocation(SP.shaderID, "lightPos"), 1, glm::value_ptr(glm::vec3(rotatedLightPos)));
        // color uniform ����
        glUniform3fv(glGetUniformLocation(SP.shaderID, "lightColor"), 1, glm::value_ptr(color));
        // intensity uniform ����
        glUniform1f(glGetUniformLocation(SP.shaderID, "lightIntensity"), intensity);
    }
};

class Object {
public:
    int what{};

    GLuint objVAO{};
    GLuint textureID{};
    string objName{};

    glm::vec3 location{};   // ��ġ(translate ����)
    glm::vec3 color{};      // ����

    glm::vec3 scale{ 1.0f, 1.0f, 1.0f };  // ũ�� (����Ʈ�� 1)
    float rotationAngle = 0.0f;  // ȸ�� ���� (����)
    float aniAngle = 0.0f;       // ȸ�� ���� (����)
    int aniDir = 1;       // �ִϸ��̼� �����÷���
    int moving = 0;       // ȸ�� ���� (����)
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
                scale = glm::vec3(0.5f, 0.5f, 0.5f);  // �⺻ ũ�� ����
                rotationAngle = 0.0f;  // �⺻ ȸ�� ���� ����
                aniAngle = 0.0f;      // �⺻ ȸ�� ���� ����
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
        rotationAngle = glm::radians(angle);  // �Է��� ����, ���ο����� ���� ���
    }

    void Aniangle(float angle) {
        aniAngle += glm::radians(angle);  // �Է��� ����, ���ο����� ���� ���
    }

    void updateJump() {
        if (jump) {
            location.y += jumpVelocity;   // �ӵ��� ���� ���� ����
            jumpVelocity += gravity;      // �߷� ����

            if (location.y <= groundLevel) {
                location.y = groundLevel; // �ٴڿ� ����
                jumpVelocity = 0.0f;      // �ӵ� �ʱ�ȭ
                jump = false;             // ���� ���� ����
            }
        }
    }

    void startJump(float initialVelocity) {
        if (!jump && location.y <= groundLevel) { // �ٴڿ����� ���� ����
            jump = true;
            jumpVelocity = initialVelocity;       // �ʱ� ���� �ӵ� ����
        }
    }

    void updateJumpBB() {
        if (jump) {
            location.y += jumpVelocity;   // �ӵ��� ���� ���� ����
            jumpVelocity += gravity;      // �߷� ����

            if (location.y <= -10) {
                location.y = -10; // �ٴڿ� ����
                jumpVelocity = 0.0f;      // �ӵ� �ʱ�ȭ
                jump = false;             // ���� ���� ����
            }
        }
    }

    void startJumpBB(float initialVelocity) {
        if (!jump && location.y <= -10) { // �ٴڿ����� ���� ����
            jump = true;
            jumpVelocity = initialVelocity;       // �ʱ� ���� �ӵ� ����
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

        // �̵�
        transform_Matrix = glm::translate(transform_Matrix, location);
        // �ִϸ��̼�
        transform_Matrix = glm::rotate(transform_Matrix, aniAngle, glm::vec3(0.0f, 0.0f, 1.0f));
        transform_Matrix = glm::translate(transform_Matrix, glm::vec3(-4.0f, 0.0f, 0.0f));  //--- ���� ���߱�
        // ����
        transform_Matrix = glm::rotate(transform_Matrix, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        // ���Ȯ��
        transform_Matrix = glm::scale(transform_Matrix, scale);

        unsigned int ObjectTransform = glGetUniformLocation(SP.shaderID, "transform");
        glUniformMatrix4fv(ObjectTransform, 1, GL_FALSE, glm::value_ptr(transform_Matrix));

        // color�� ���̴��� ����
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

        // ��ġ ��ǥ (-1.0 ~ 1.0 ������ ����)
        object_vertices.push_back(glm::vec3(1.0f, 1.0f, 0.0f));   // ������ ��
        object_vertices.push_back(glm::vec3(-1.0f, 1.0f, 0.0f));  // ���� ��
        object_vertices.push_back(glm::vec3(-1.0f, -1.0f, 0.0f)); // ���� �Ʒ�
        object_vertices.push_back(glm::vec3(1.0f, -1.0f, 0.0f));  // ������ �Ʒ�

        // ���� ���� (���簢���� ������ ���Ѵٰ� ����)
        object_vertice_normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));

        // �ؽ�ó ��ǥ (0.0 ~ 1.0 ������ ����)
        object_vertice_textures.push_back(glm::vec2(1.0f, 1.0f)); // ������ ��
        object_vertice_textures.push_back(glm::vec2(0.0f, 1.0f)); // ���� ��
        object_vertice_textures.push_back(glm::vec2(0.0f, 0.0f)); // ���� �Ʒ�
        object_vertice_textures.push_back(glm::vec2(1.0f, 0.0f)); // ������ �Ʒ�

        // �ؽ�ó ����
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // �ؽ�ó ���͸� �� ���� ����
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // S�� ����
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // T�� ����
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // �̹��� �ε�
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            // �ؽ�ó �̹��� ������ ���ε�
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D); // Mipmap ����
            stbi_image_free(data); // �̹��� �޸� ����
        }
        else {
            std::cerr << "Failed to load texture at " << filePath << std::endl;
        }
    }

    void Draw_stage_square() {
        // VAO, VBO ����
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

        // �̵�
        transform_Matrix = glm::translate(transform_Matrix, location);
        // ����
        transform_Matrix = glm::rotate(transform_Matrix, rotationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
        // ���Ȯ��
        transform_Matrix = glm::scale(transform_Matrix, scale);

        ObjectTransform = glGetUniformLocation(SP.shaderID, "parent_transform");
        glUniformMatrix4fv(ObjectTransform, 1, GL_FALSE, glm::value_ptr(transform_Matrix));

        // ������ �׸��� ���� �ʿ��� ���̴��� ��� ����
        unsigned int ObjectColor = glGetUniformLocation(SP.shaderID, "fColor");
        glUniform3fv(ObjectColor, 1, glm::value_ptr(color));

        glBindVertexArray(objectVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);

        // �޸� ����
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

int main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
    srand(static_cast<int>(time(NULL)));
    enableANSI();

    //--- ������ �����ϱ�
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("PokeRun!");
    //cout << "������ ������" << endl;

    //--- GLEW �ʱ�ȭ�ϱ�
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) // glew �ʱ�ȭ
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

    SP.make_vertexShaders(); //--- ���ؽ� ���̴� �����
    SP.make_fragmentShaders(); //--- �����׸�Ʈ ���̴� �����
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

//--- �׸��� �ݹ� �Լ�
GLvoid drawScene()
{
    switch (currentState) {
    case MAIN_MENU:
        //--- ����� ���� ����
        glClearColor(0.792f, 0.761f, 0.306f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(SP.shaderID);

        camera.make_camera_orbit_xy();
        glEnable(GL_DEPTH_TEST);
        start_object.Draw_stage_square();

        glutSwapBuffers(); //--- ȭ�鿡 ����ϱ�
        break;
    case RESTART_MENU:
        //--- ����� ���� ����
        glClearColor(0.792f, 0.761f, 0.306f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(SP.shaderID);

        camera.make_camera_orbit_xy();
        glEnable(GL_DEPTH_TEST);
        restart_object.Draw_stage_square();

        glutSwapBuffers(); //--- ȭ�鿡 ����ϱ�
        break;
    case GAME_PLAY:
        //--- ����� ���� ����
        glClearColor(0.3f, 0.5f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //--- ������ ���������ο� ���̴� �ҷ�����
        glUseProgram(SP.shaderID);

        // ��������, ���� ����
        if (draw_enable) {
            glEnable(GL_DEPTH_TEST);
            glCullFace(GL_BACK);
            glFrontFace(GL_CCW);
        }
        else if (!draw_enable) glDisable(GL_CULL_FACE);


        camera.make_camera_perspective();
        light.sendToShader();

        // �÷��̾� �׸��� (��ȯ ����)
        for (auto& v : pokemon) {
            v.Draw_OBJ();
        }
        
        // ���� �׸��� (��ȯ ����)
        for (auto& v : sitrusberry) {
            v.Draw_OBJ();
        }
        for (auto& v : pechaberry) {
            v.Draw_OBJ();
        }
        for (auto& v : oranberry) {
            v.Draw_OBJ();
        }
        
        // �ٴ� �׸���
        for (auto& v : map) {
            v.Draw_OBJ();
        }
        // ��ֹ� �׸���
        for (auto& v : obs) {
            v.Draw_OBJ();
        }

        // �浹 �ڽ� �׸��� (������)
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


        glutSwapBuffers(); //--- ȭ�鿡 ����ϱ�
        break;
    }
}
//--- �ٽñ׸��� �ݹ� �Լ�
GLvoid Reshape(int w, int h)
{
    glViewport(0, 0, w, h);
}

void TimerFunction(int value) {

    // ī�޶� �� �÷��̾� �̵�
    pokemon[0].location.z -= speed;
    pokemon_bb[0].location.z -= speed;
    camera.cameraPos.z -= speed;
    camera.cameraTarget.z -= speed;
    light.position.z -= speed;

    berry_count++;
    map_count++;

    if (pokemon[0].animation) {      //--- �ڶ׵ڶ�
        if (game_time >= 10) {
            pokemon[0].aniDir = -1;
        }
        else if (game_time <= -10) {
            pokemon[0].aniDir = 1;
        }
        game_time += pokemon[0].aniDir;
        pokemon[0].Aniangle(pokemon[0].aniDir * 0.7);
    }

    if (pokemon[0].moving == -1) {   //--- �¿��̵�
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
                printf("��ī��� ���ͺ��� ���ư����ϴ�...\n");
                printf("SCORE: %d\n", score);
                printf("'R'�� ���� ������ϼ���");
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