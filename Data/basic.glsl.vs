#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;
layout(location = 2) in vec3 a_Color;
layout(location = 3) in float a_Alpha;
layout(location = 4) in vec2 a_WorldPos;
layout(location = 5) in vec3 a_Normal;

uniform mat4 u_ViewProjection;

out vec3 v_Position;
out vec2 v_WorldPos;
out vec2 v_TexCoords;
out vec3 v_Color;
out float v_Alpha;
out vec3 v_FragPos;
out vec3 v_Normal;

void main() {
   v_Position = a_Position;
   v_WorldPos = a_WorldPos;
   v_TexCoords = a_TexCoords;
   v_Color = a_Color;
   v_Alpha = a_Alpha;
   v_FragPos = vec3(u_ViewProjection * vec4(a_Position, 1.0));
   v_Normal = vec3(u_ViewProjection * vec4(a_Position, 1.0));
   gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}
