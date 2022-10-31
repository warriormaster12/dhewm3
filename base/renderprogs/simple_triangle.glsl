#version 450
#ifdef STAGE_VERTEX
#pragma shader_stage(vertex)

layout (location = 0) in vec3 vPosition;

layout(set = 0, binding = 0) uniform TriangleBuffer{
	vec3 color;
} triangleData;


void main() {
    gl_Position = vec4(vPosition,1.0);
}

#elif defined(STAGE_FRAGMENT)
#pragma shader_stage(fragment)
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform TriangleBuffer{
	vec3 color;
} triangleData;


void main() {
    vec3 resultColor = 1-triangleData.color;
    outColor = vec4(resultColor, 1.0);
}
#endif
