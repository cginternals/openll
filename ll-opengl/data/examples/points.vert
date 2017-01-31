#version 330

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;
layout(location = 2) in float size;

out vec3 v_color;

void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
    gl_PointSize = size;
    v_color = color;
}
