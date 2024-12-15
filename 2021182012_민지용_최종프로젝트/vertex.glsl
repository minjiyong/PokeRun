//--- vertex shader: vertex.glsl ���Ͽ� ����

#version 330 core
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord; //--- �ؽ�ó ��ǥ

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 transform;		//�� ��ȯ ���
uniform mat4 view;			//�� ��ȯ ���
uniform mat4 projection;	//���� ��ȯ ���

void main(){
   gl_Position = projection * view * transform * vec4(vPos, 1.0);
   FragPos = vec3(transform * vec4(vPos, 1.0));
   Normal = vec3(transform * vec4(vNormal, 1.0));
   TexCoord = vTexCoord; //--- �ؽ�ó ��ǥ ����
}