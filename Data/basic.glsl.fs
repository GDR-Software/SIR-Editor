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
//uniform float u_AmbientIntensity;
uniform vec3 u_AmbientColor;
uniform Light lights[MAX_MAP_LIGHTS];

void applyLighting()
{
    vec3 ambient = u_AmbientColor;

    for (int i = 0; i < u_numLights; i++) {
        float diffuse = 0.0;
        float dist = distance(lights[i].origin, v_FragPos.xy);
        vec3 color = lights[i].color + lights[i].intensity.x;

        if (dist <= lights[i].intensity.y) {
            diffuse = 1.0 - abs(dist / lights[i].intensity.y);
        }
        vec3 final = clamp((color * diffuse) + u_AmbientColor, a_Color.rgb * u_AmbientColor, vec3(256));

        a_Color.rgb = min(a_Color.rgb * final, a_Color.rgb);
    }
}

void main()
{
    if (texture(u_DiffuseMap, v_TexCoords).a < 1.0) {
        discard;
    }
    
    a_Color = texture(u_DiffuseMap, v_TexCoords);
    applyLighting();

    if (v_Color.r != 1.0 && v_Color.g != 1.0 && v_Color.b != 1.0) {
        a_Color.rgb *= v_Color;
    }
}