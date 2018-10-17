#version 460
#extension GL_NVX_raytracing : require

layout(location = 0) rayPayloadInNVX vec3 hitValue;
layout(location = 1) hitAttributeNVX vec3 attribs;

void main()
{
    const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    hitValue = barycentrics;
}
