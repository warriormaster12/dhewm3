#version 450

layout (location = 0) in vec3 inColor;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform TriangleBuffer{
	vec3 color;
} triangleData;


void main() {
    vec3 resultColor = 1-(inColor * triangleData.color);
    outColor = vec4(resultColor, 1.0);
} 
