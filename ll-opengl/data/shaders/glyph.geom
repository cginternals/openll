#version 330

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 viewProjection;

in vec4 v_tangent[];
in vec4 v_bitangent[];

in vec4 v_uvRect[];

in vec4 v_fontColor[];

in uint v_superSampling[];

out vec2 g_uv;
out vec4 g_fontColor;
flat out uint g_superSampling;

void main()
{
    vec4 origin = gl_in[0].gl_Position;

    // lower right
    gl_Position = origin + v_tangent[0];
    g_uv = v_uvRect[0].zy;
    gl_Position = viewProjection * gl_Position;
    g_fontColor = v_fontColor[0];
    g_superSampling = v_superSampling[0];

    EmitVertex();
    
    // upper right
    gl_Position = origin + v_bitangent[0] + v_tangent[0];
    g_uv = v_uvRect[0].zw;
    gl_Position = viewProjection * gl_Position;
    g_fontColor = v_fontColor[0];
    g_superSampling = v_superSampling[0];

    EmitVertex();
    
    // lower left
    gl_Position = origin;
    g_uv = v_uvRect[0].xy;
    gl_Position = viewProjection * gl_Position;
    g_fontColor = v_fontColor[0];
    g_superSampling = v_superSampling[0];

    EmitVertex();

    // upper left
    gl_Position = origin + v_bitangent[0];
    g_uv = v_uvRect[0].xw;
    gl_Position = viewProjection * gl_Position;
    g_fontColor = v_fontColor[0];
    g_superSampling = v_superSampling[0];

    EmitVertex();

    EndPrimitive();
}
