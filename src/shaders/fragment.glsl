#version 450 core
layout (location = 0) out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;
in mat3 TBN;

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;

uniform vec3 viewPos;
uniform Light light;
uniform Material material;

void main()
{
    vec3 objectColor = vec3(texture(texture_diffuse1, TexCoord));
    // ambient
    vec3 ambient = light.ambient * objectColor * material.ambient;
    // diffuse
    //vec3 norm = normalize(Normal);
    vec3 norm = texture(texture_normal1, TexCoord).rgb;
    norm = norm * 2.0f - 1.0f;
    norm = normalize(TBN * norm);

    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = light.diffuse * (diff * objectColor) * material.diffuse;
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflecDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflecDir), 0.0f), material.shininess);
    vec3 specular = light.specular * (spec * vec3(texture(texture_specular1, TexCoord))) * material.specular;

    vec4 result = vec4(ambient + diffuse + specular, 1.0f);
    FragColor = result; 
}

