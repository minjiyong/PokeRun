//--- vertex shader: vertex.glsl 파일에 저장

#version 330 core
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord; //--- 텍스처 좌표

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 transform;		//모델 변환 행렬
uniform mat4 view;			//뷰 변환 행렬
uniform mat4 projection;	//투영 변환 행렬

void main(){
   gl_Position = projection * view * transform * vec4(vPos, 1.0);
   FragPos = vec3(transform * vec4(vPos, 1.0));
   Normal = vec3(transform * vec4(vNormal, 1.0));
   TexCoord = vTexCoord; //--- 텍스처 좌표 전달
}