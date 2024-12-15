#define _CRT_SECURE_NO_WARNINGS
#include "Importer.h"

using namespace std;

void Importer::read_newline(char* str) {
    char* pos;
    if ((pos = strchr(str, '\n')) != NULL)
        *pos = '\0';
}


void Importer::read_obj_file(const std::string& filename, VertexData* vertexData) {
    FILE* file;
    fopen_s(&file, filename.c_str(), "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        read_newline(line);

        // Vertex information (v x y z)
        if (line[0] == 'v' && line[1] == ' ') {
            glm::vec3 temp_vertex;
            int result = sscanf_s(line + 2, "%f %f %f", &temp_vertex.x, &temp_vertex.y, &temp_vertex.z);
            if (result == 3) {
                vertexData->vertex.push_back(temp_vertex);
                glm::vec3 temp_color = { (float)(rand() % 256) / 255.0f, (float)(rand() % 256) / 255.0f, (float)(rand() % 256) / 255.0f }; // Assigning a default color (red)

                vertexData->color.push_back(temp_color);
            }
        }
        // ���ؽ� �븻
        else if (line[0] == 'v' && line[1] == 'n' && line[2] == ' ') {
            glm::vec3 temp_vertex;
            int result = sscanf_s(line + 3, "%f %f %f", &temp_vertex.x, &temp_vertex.y, &temp_vertex.z);
            if (result == 3) {
                vertexData->vertexnormal.push_back(temp_vertex);
            }
        }
        // ���ؽ� texture
        else if (line[0] == 'v' && line[1] == 't' && line[2] == ' ') {
            glm::vec2 temp_texture;  // 2D �ؽ�ó ��ǥ�� u, v
            int result = sscanf_s(line + 3, "%f %f", &temp_texture.x, &temp_texture.y);  // 2D ��ǥ (u, v)

            // �� ���� ��(u, v)�� ���������� �����ٸ�
            if (result == 2) {
                vertexData->vertextexture.push_back(temp_texture);  // �ؽ�ó ��ǥ ���Ϳ� �߰�
            }
        }
        else if (line[0] == 'f' && line[1] == ' ') {
            unsigned int v1, v2, v3;
            unsigned int vt1, vt2, vt3;
            unsigned int vn1, vn2, vn3;

            // OBJ�� face ������ �б� (v/vt/vn)
            int result = sscanf_s(line + 2, "%u/%u/%u %u/%u/%u %u/%u/%u",
                &v1, &vt1, &vn1,
                &v2, &vt2, &vn2,
                &v3, &vt3, &vn3);

            if (result == 9) {
                // OBJ�� �ε����� 1���� �����ϹǷ� 0���� ����
                v1--; v2--; v3--;
                vt1--; vt2--; vt3--;
                vn1--; vn2--; vn3--;

                // ������ face ���� ����
                vertexData->face.push_back(glm::ivec3(v1, v2, v3));
                vertexData->vtface.push_back(glm::ivec3(vt1, vt2, vt3));
                vertexData->vnface.push_back(glm::ivec3(vn1, vn2, vn3));
            }
        }
    }

    fclose(file);
    vertexData->filename = filename;
}

void Importer::rearrangeVerticesByFace(VertexData* vertexData) {
    // ���Ӱ� �����ĵ� vertex�� color �����͸� ������ ����
    vector<glm::vec3> rearrangedVertices;
    vector<glm::vec3> rearrangedVN;
    vector<glm::vec2> rearrangedVT;
    vector<glm::vec3> rearrangedColors;

    // face�� �ε����� ������� vertex�� color �����͸� ������
    for (const auto& face : vertexData->face) {
        rearrangedVertices.push_back(vertexData->vertex[face.x]);
        rearrangedVertices.push_back(vertexData->vertex[face.y]);
        rearrangedVertices.push_back(vertexData->vertex[face.z]);

        rearrangedColors.push_back(vertexData->color[face.x]);
        rearrangedColors.push_back(vertexData->color[face.y]);
        rearrangedColors.push_back(vertexData->color[face.z]);
    }

    for (const auto& face : vertexData->vtface) {
        rearrangedVT.push_back(vertexData->vertextexture[face.x]);
        rearrangedVT.push_back(vertexData->vertextexture[face.y]);
        rearrangedVT.push_back(vertexData->vertextexture[face.z]);
    }

    for (const auto& face : vertexData->vnface) {
        rearrangedVN.push_back(vertexData->vertexnormal[face.x]);
        rearrangedVN.push_back(vertexData->vertexnormal[face.y]);
        rearrangedVN.push_back(vertexData->vertexnormal[face.z]);
    }

    // ���� vertex�� color ���͸� �����ĵ� ���ͷ� ��ü
    vertexData->vertex = rearrangedVertices;
    vertexData->color = rearrangedColors;
    vertexData->vertexnormal = rearrangedVN;
    vertexData->vertextexture = rearrangedVT;
}

void Importer::ReadTexture(const std::string& filePath, unsigned int& texture) {
    // �ؽ�ó ����
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // �ؽ�ó ���͸� �� ���� ����
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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

void Importer::ReadObj() {
    VertexData* newVertexData = new VertexData;

    read_obj_file("pikachu.obj", newVertexData);
    ReadTexture("pikachu.png", newVertexData->texture);
    rearrangeVerticesByFace(newVertexData);

    VertexBuffers.push_back(newVertexData);
    SetupMesh(VertexBuffers.back());


    VertexData* newVertexData1 = new VertexData;

    read_obj_file("SitrusBerry.obj", newVertexData1);
    ReadTexture("SitrusBerry.png", newVertexData1->texture);
    rearrangeVerticesByFace(newVertexData1);

    VertexBuffers.push_back(newVertexData1);
    SetupMesh(VertexBuffers.back());


    VertexData* newVertexData2 = new VertexData;

    read_obj_file("PechaBerry.obj", newVertexData2);
    ReadTexture("PechaBerry.png", newVertexData2->texture);
    rearrangeVerticesByFace(newVertexData2);

    VertexBuffers.push_back(newVertexData2);
    SetupMesh(VertexBuffers.back());


    VertexData* newVertexData3 = new VertexData;

    read_obj_file("OranBerry.obj", newVertexData3);
    ReadTexture("OranBerry.png", newVertexData3->texture);
    rearrangeVerticesByFace(newVertexData3);

    VertexBuffers.push_back(newVertexData3);
    SetupMesh(VertexBuffers.back());


    VertexData* newVertexData4 = new VertexData;

    read_obj_file("cube.obj", newVertexData4);
    //ReadTexture("grass_ishi.png", newVertexData4->texture);
    rearrangeVerticesByFace(newVertexData4);

    VertexBuffers.push_back(newVertexData4);
    SetupMesh(VertexBuffers.back());

    VertexData* newVertexData5 = new VertexData;

    read_obj_file("Cliff.obj", newVertexData5);
    ReadTexture("UV-Cliff.png", newVertexData5->texture);
    rearrangeVerticesByFace(newVertexData5);

    VertexBuffers.push_back(newVertexData5);
    SetupMesh(VertexBuffers.back());
}

void Importer::SetupMesh(VertexData* VD) {
    glGenVertexArrays(1, &VD->VAO);
    glGenBuffers(1, &VD->VBOVertex);
    glGenBuffers(1, &VD->VBOVertexNormal);
    glGenBuffers(1, &VD->VBOVertexTexture);

    glBindVertexArray(VD->VAO);


    glBindBuffer(GL_ARRAY_BUFFER, VD->VBOVertex);
    glBufferData(GL_ARRAY_BUFFER, VD->vertex.size() * sizeof(glm::vec3), &VD->vertex[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    if (!VD->vertexnormal.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, VD->VBOVertexNormal);
        glBufferData(GL_ARRAY_BUFFER, VD->vertexnormal.size() * sizeof(glm::vec3), &VD->vertexnormal[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    }

    if (!VD->vertextexture.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, VD->VBOVertexTexture);
        glBufferData(GL_ARRAY_BUFFER, VD->vertextexture.size() * sizeof(glm::vec2), &VD->vertextexture[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    }

    glBindVertexArray(0);
}
