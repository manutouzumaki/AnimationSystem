#version 450 core
layout (location = 0) out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform sampler2D texture_diffuse1; 

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = vec3(1.0f, 0.0f, 0.5f);
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 lightColor = vec3(1.0f, 1.0f, 0.5f);
    vec3 ambient = vec3(0.2f, 0.2f, 0.2f);
    vec3 diffuse = diff * lightColor;
    vec3 objectColor = vec3(texture(texture_diffuse1, TexCoord));
    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0f);
    
}
