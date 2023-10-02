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
   float brightness;
   vec2 origin;
   vec3 color;
};

uniform vec3 u_CameraPos;
uniform sampler2D u_SpriteSheet;
uniform int u_numLights;
uniform float u_AmbientIntensity;
uniform vec3 u_AmbientColor;
uniform bool u_DarkAmbience;
uniform Light lights[MAX_MAP_LIGHTS];

#if 0
const float gamma = 2.2;

vec3 rgb_to_hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv_to_rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
#endif

/*
DO NOT TOUCH!!! spent an entire afternoon making this one work
*/
#if 0
void applyLights(in vec4 color)
{
    vec2 lightPos = lights[0].origin;
    vec3 lightColor = lights[0].color;
    float brightness = lights[0].brightness;
    for (int i = 0; i < u_numLights; i++) {
        lightPos = lights[i].origin;
        lightColor = lights[i].color;
        brightness = lights[i].brightness;

        float dis = distance(lightPos, vec3(v_WorldPos.xy, 0.0));
        float diffuse = 1.0;

        if (diffuse <= brightness) {
            diffuse = 1.0 - abs(dis / brightness);
        }

        color = vec4(min(color.rgb * ((lightColor.rgb * diffuse) + (u_AmbientColor * u_AmbientIntensity)), color.rgb), 1.0);
    }
}
#endif

vec4 toGreyscale(in vec4 color) {
    float avg = (color.r + color.g + color.b) / 3.0;
    return vec4(avg, avg, avg, 1.0);
}

vec4 colorize(in vec4 greyscale, in vec4 color) {
    return (greyscale * color);
}

#if 0
vec4 applyGamma(vec4 color) {
    return vec4(pow(color.rgb, vec3(1.0 / gamma)), 1.0);
}

vec4 applyBrightness(vec4 color, float brightness) {
    return vec4((color - 0.5) * 1.0 + 0.5 + brightness);
}
#endif

void applyLighting()
{
    vec3 ambient = u_AmbientColor * u_AmbientIntensity;
    a_Color.rgb *= ambient;

    for (int i = 0; i < u_numLights; i++) {
//        vec3 pos = normalize(v_Normal - v_FragPos);
        float dist = distance(normalize(lights[i].origin), v_Position.xy);
        float diffuse = 0.0;

        if (dist <= lights[i].brightness) {
            diffuse = 1.0 - abs(dist / lights[i].brightness);
        }

        a_Color = vec4(min(a_Color.rgb * ((lights[i].color * diffuse) + ambient), a_Color.rgb), 1.0);
    }
    if (a_Color.r < ambient.r && a_Color.g < ambient.g && a_Color.b < ambient.b) {
        a_Color.rgb *= ambient;
    }
}

void main()
{
    a_Color = texture(u_SpriteSheet, v_TexCoords);
    if (a_Color.a < 1.0)
        discard;
    applyLighting();
}