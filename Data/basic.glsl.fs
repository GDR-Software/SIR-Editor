#version 330 core

#define MAX_MAP_LIGHTS 256

layout(location = 0) out vec4 a_Color;

in vec3 v_Position;
in vec2 v_WorldPos;
in vec2 v_TexCoords;
in vec3 v_Color;
in float v_Alpha;
in vec3 v_FragPos;
in vec3 v_Normal;

struct Light {
    vec2 intensity; // x is brightness, y is range
    vec2 origin;
    vec3 color;
};

uniform sampler2D u_DiffuseMap;
uniform int u_numLights;
uniform float u_CameraZoom;
uniform float u_AmbientIntensity;
uniform vec3 u_AmbientColor;
uniform Light lights[MAX_MAP_LIGHTS];

/*
NOTE: looks really cool, but not really functional for 2d...
*/
vec3 CalcPointLight(Light light)
{
    vec2 lightDir = normalize(light.origin - v_FragPos.xy);
    float diff = max(dot(v_Normal.xy, lightDir), 0.0);

    float constant = 1.0;
    float linear = 0.09;
    float quadratic = 0.032;

    float dist = length(light.origin - v_FragPos.xy);
    float attenuation = 1.0 / (constant + linear * dist + quadratic * (dist * dist));

    vec3 diffuse = diff * texture(u_DiffuseMap, v_TexCoords).rgb;
    vec3 ambient = u_AmbientColor * texture(u_DiffuseMap, v_TexCoords).rgb;

    ambient *= attenuation;
    diffuse *= attenuation;

    return (ambient + diffuse);
}

void applyLighting()
{
    vec3 ambient = u_AmbientColor + u_AmbientIntensity;

    for (int i = 0; i < u_numLights; i++) {
        float diffuse = 0.0;
        mat4 model = mat4(1.0);
        float dist = distance(lights[i].origin, v_WorldPos.xy);
        vec3 color = lights[i].color + lights[i].intensity.x;
        float range = lights[i].intensity.y;

        if (dist <= range) {
            diffuse = 1.0 - abs(dist / range);
        }

        a_Color *= vec4(min(a_Color.rgb * ((lights[i].color * diffuse) + ambient), a_Color.rgb), 1.0);
    }
}

void main()
{
    if (texture(u_DiffuseMap, v_TexCoords).a < 1.0) {
        discard;
    }
    
    a_Color = texture(u_DiffuseMap, v_TexCoords);
    if (u_numLights == 0) {
        a_Color.rgb *= u_AmbientColor + u_AmbientIntensity;
    }
    applyLighting();
}