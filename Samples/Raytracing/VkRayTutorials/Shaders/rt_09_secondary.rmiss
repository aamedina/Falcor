#version 460
#extension GL_NVX_raytracing : require

layout(location = 2) rayPayloadInNVX float secondaryRayHitValue;

void main()
{
    secondaryRayHitValue = gl_RayTmaxNVX;
}
