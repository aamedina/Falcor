#pragma once

#include "Application.h"
#include "vulkan/vulkan.h"

struct VkGeometryInstance
{
    float transform[12];
    uint32_t instanceId : 24;
    uint32_t mask : 8;
    uint32_t instanceOffset : 24;
    uint32_t flags : 8;
    uint64_t accelerationStructureHandle;
};

class RaytracingApplication : public Application
{
public:
    RaytracingApplication();

protected:
    std::vector<const char*> _deviceExtensions;

    PFN_vkCreateAccelerationStructureNVX vkCreateAccelerationStructureNVX = VK_NULL_HANDLE;
    PFN_vkDestroyAccelerationStructureNVX vkDestroyAccelerationStructureNVX = VK_NULL_HANDLE;
    PFN_vkGetAccelerationStructureMemoryRequirementsNVX vkGetAccelerationStructureMemoryRequirementsNVX = VK_NULL_HANDLE;
    PFN_vkGetAccelerationStructureScratchMemoryRequirementsNVX vkGetAccelerationStructureScratchMemoryRequirementsNVX = VK_NULL_HANDLE;
    PFN_vkCmdCopyAccelerationStructureNVX vkCmdCopyAccelerationStructureNVX = VK_NULL_HANDLE;
    PFN_vkBindAccelerationStructureMemoryNVX vkBindAccelerationStructureMemoryNVX = VK_NULL_HANDLE;
    PFN_vkCmdBuildAccelerationStructureNVX vkCmdBuildAccelerationStructureNVX = VK_NULL_HANDLE;
    PFN_vkCmdTraceRaysNVX vkCmdTraceRaysNVX = VK_NULL_HANDLE;
    PFN_vkGetRaytracingShaderHandlesNVX vkGetRaytracingShaderHandlesNVX = VK_NULL_HANDLE;
    PFN_vkCreateRaytracingPipelinesNVX vkCreateRaytracingPipelinesNVX = VK_NULL_HANDLE;
    PFN_vkGetAccelerationStructureHandleNVX vkGetAccelerationStructureHandleNVX = VK_NULL_HANDLE;

    VkPhysicalDeviceRaytracingPropertiesNVX _raytracingProperties = { };

    void InitRaytracing();
    virtual void CreateDevice() override; // Tutorial 01
};
