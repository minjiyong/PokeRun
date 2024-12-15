//--- fragment shader: fragment.glsl ���Ͽ� ����

#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 lightPos;   // ������ ��ġ
uniform vec3 lightColor; // ������ ����
uniform float lightIntensity; // ������ ����
uniform vec3 viewPos;    // ī�޶� ��ġ
uniform vec3 fColor;     // �����׸�Ʈ ���̴��� ���޵� ���� == ��ü�� ����

uniform sampler2D outTexture; //--- �ؽ�ó ���÷�

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


    // ���� ����
    vec3 result = lightIntensity * (ambient + diffuse + specular) * fColor;

    // �ؽ�ó ����� �⺻ ���� ȥ��
    vec4 textureColor = texture(outTexture, TexCoord); 
    vec3 mixedColor = mix(result, textureColor.rgb, 0.5f);  // ���� ó�� �� ����� �ؽ�ó ������ 50% ������ ȥ��
    FragColor = vec4(mixedColor, textureColor.a);
}