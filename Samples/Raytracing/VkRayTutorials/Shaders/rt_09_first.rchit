#version 460
#extension GL_NVX_raytracing : require

layout(binding = 0) uniform accelerationStructureNVX topLevelAS;

layout(location = 0) rayPayloadInNVX vec3 hitValue;
layout(location = 1) hitAttributeNVX vec3 attribs;
layout(location = 2) rayPayloadNVX float secondaryRayHitValue;

void main()
{
    const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

    vec3 origin = gl_WorldRayOriginNVX + gl_WorldRayDirectionNVX * gl_HitTNVX;
    vec3 direction = normalize(vec3(0.8, 1, 0));
    uint rayFlags = gl_RayFlagsOpaqueNVX | gl_RayFlagsTerminateOnFirstHitNVX;
    uint cullMask = 0xff;
    float tmin = 0.001;
    float tmax = 100.0;

    traceNVX(topLevelAS, rayFlags, cullMask, 1 /* sbtRecordOffset */, 0 /* sbtRecordStride */, 1 /* missIndex */, origin, tmin, direction, tmax, 2 /*payload location*/);

    hitValue = barycentrics * ( secondaryRayHitValue < tmax ? 0.25 : 1.0 );
}
