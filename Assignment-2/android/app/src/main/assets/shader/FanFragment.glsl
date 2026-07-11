#version 300 es

precision mediump float;

in float Shade;
uniform vec3 PARTCOLOR;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(PARTCOLOR * Shade, 1.0);
}
