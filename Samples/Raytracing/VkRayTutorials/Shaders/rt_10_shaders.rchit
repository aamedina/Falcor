#version 460
#extension GL_NVX_raytracing : require
#extension GL_EXT_nonuniform_qualifier : require

layout(binding = 0) uniform accelerationStructureNVX topLevelAS;

layout(binding = 2) uniform UniformBuffer
{
    vec3 color;
} uniformBuffers[];

layout(location = 0) rayPayloadInNVX vec3 hitValue;
layout(location = 1) hitAttributeNVX vec3 attribs;

layout(shaderRecordNVX) buffer InlineData {
    vec4 inlineData;
};

void main()
{
    // gl_InstanceCustomIndex = VkGeometryInstance::instanceId
    hitValue = inlineData.rgb + uniformBuffers[nonuniformEXT(gl_InstanceCustomIndexNVX)].color;
}
