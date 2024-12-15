//--- fragment shader: fragment.glsl 파일에 저장

#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 lightPos;   // 광원의 위치
uniform vec3 lightColor; // 광원의 색상
uniform float lightIntensity; // 광원의 강도
uniform vec3 viewPos;    // 카메라 위치
uniform vec3 fColor;     // 프래그먼트 셰이더로 전달된 색상 == 물체의 색상

uniform sampler2D outTexture; //--- 텍스처 샘플러

void main (){
    // Ambient
    float ambientLight = 0.5;
    vec3 ambient = ambientLight * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    float specularStrength = 1.0;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;


    // 최종 색상
    vec3 result = lightIntensity * (ambient + diffuse + specular) * fColor;

    // 텍스처 색상과 기본 색상 혼합
    vec4 textureColor = texture(outTexture, TexCoord); 
    vec3 mixedColor = mix(result, textureColor.rgb, 0.5f);  // 광원 처리 후 색상과 텍스처 색상을 50% 비율로 혼합
    FragColor = vec4(mixedColor, textureColor.a);
}