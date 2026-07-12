#version 300 es

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in float VertexShade;

out float Shade;

uniform mat4 MODELVIEWPROJECTIONMATRIX;

void main()
{
    gl_Position = MODELVIEWPROJECTIONMATRIX * vec4(VertexPosition, 1.0);
    Shade = VertexShade;
}
