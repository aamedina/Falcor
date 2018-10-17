#version 460
#extension GL_NVX_raytracing : require

layout(location = 0) rayPayloadInNVX vec3 hitValue;

void main()
{
    hitValue = vec3(0.0, 0.1, 0.3);
}
