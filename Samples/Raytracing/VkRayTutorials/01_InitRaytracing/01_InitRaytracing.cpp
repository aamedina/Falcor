#include "../Common/Application.h"
#include "vulkan/vulkan.h"

class TutorialApplication : public Application
{
public:
    // ============================================================
    // RAYTRACING DATA
    // ============================================================

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

public:
    TutorialApplication();

    virtual void CreateDevice() override;                        // Tutorial 01
    virtual void Init() override;                                // Tutorial 01
    virtual void Cleanup() override;
};

TutorialApplication::TutorialApplication()
{
    _appName = L"VkRay Tutorial 01: Initialization";
}

// ============================================================
// Tutorial 01: Create device with raytracing support
// ============================================================
void TutorialApplication::CreateDevice()
{
    std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
    float priority = 0.0f;

    VkDeviceQueueCreateInfo deviceQueueCreateInfo;
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.pNext = nullptr;
    deviceQueueCreateInfo.flags = 0;
    deviceQueueCreateInfo.queueFamilyIndex = _queuesInfo.Graphics.QueueFamilyIndex;
    deviceQueueCreateInfo.queueCount = 1;
    deviceQueueCreateInfo.pQueuePriorities = &priority;
    deviceQueueCreateInfos.push_back(deviceQueueCreateInfo);

    if (_queuesInfo.Compute.QueueFamilyIndex != _queuesInfo.Graphics.QueueFamilyIndex)
    {
        deviceQueueCreateInfo.queueFamilyIndex = _queuesInfo.Compute.QueueFamilyIndex;
        deviceQueueCreateInfos.push_back(deviceQueueCreateInfo);
    }
    if (_queuesInfo.Transfer.QueueFamilyIndex != _queuesInfo.Graphics.QueueFamilyIndex && _queuesInfo.Transfer.QueueFamilyIndex != _queuesInfo.Compute.QueueFamilyIndex)
    {
        deviceQueueCreateInfo.queueFamilyIndex = _queuesInfo.Transfer.QueueFamilyIndex;
        deviceQueueCreateInfos.push_back(deviceQueueCreateInfo);
    }

    // Request NV_RAYTRACING_EXTENSION:
    std::vector<const char*> deviceExtensions({ VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_NVX_RAYTRACING_EXTENSION_NAME });
    VkPhysicalDeviceFeatures features = { };

    VkDeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.queueCreateInfoCount = (uint32_t)deviceQueueCreateInfos.size();
    deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = nullptr;
    deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceCreateInfo.pEnabledFeatures = &features;

    VkResult code = vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_device);
    if (code == VK_ERROR_EXTENSION_NOT_PRESENT)
    {
        NVVK_CHECK_ERROR(code, L"vkCreateDevice failed due to missing extension.\n\nMake sure VK_NV_RAYTRACING_EXTENSION is supported by installed driver!\n\n");
    }
    else
    {
        NVVK_CHECK_ERROR(code, L"vkCreateDevice");
    }
}

void TutorialApplication::Init()
{
    // ============================================================
    // Get raytracing function pointers
    NVVK_RESOLVE_DEVICE_FUNCTION_ADDRESS(_device, vkCreateAccelerationStructureNVX);
    NVVK_RESOLVE_DEVICE_FUNCTION_ADDRESS(_device, vkDestroyAccelerationStructureNVX);
    NVVK_RESOLVE_DEVICE_FUNCTION_ADDRESS(_device, vkGetAccelerationStructureMemoryRequirementsNVX);
    NVVK_RESOLVE_DEVICE_FUNCTION_ADDRESS(_device, vkGetAccelerationStructureScratchMemoryRequirementsNVX);
    NVVK_RESOLVE_DEVICE_FUNCTION_ADDRESS(_device, vkCmdCopyAccelerationStructureNVX);
    NVVK_RESOLVE_DEVICE_FUNCTION_ADDRESS(_device, vkBindAccelerationStructureMemoryNVX);
    NVVK_RESOLVE_DEVICE_FUNCTION_ADDRESS(_device, vkCmdBuildAccelerationStructureNVX);
    NVVK_RESOLVE_DEVICE_FUNCTION_ADDRESS(_device, vkCmdTraceRaysNVX);
    NVVK_RESOLVE_DEVICE_FUNCTION_ADDRESS(_device, vkGetRaytracingShaderHandlesNVX);
    NVVK_RESOLVE_DEVICE_FUNCTION_ADDRESS(_device, vkCreateRaytracingPipelinesNVX);
    NVVK_RESOLVE_DEVICE_FUNCTION_ADDRESS(_device, vkGetAccelerationStructureHandleNVX);

    // ============================================================
    // Query values of shaderHeaderSize and maxRecursionDepth in current implementation 
    _raytracingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAYTRACING_PROPERTIES_NVX;
    _raytracingProperties.pNext = nullptr;
    _raytracingProperties.maxRecursionDepth = 0;
    _raytracingProperties.shaderHeaderSize = 0;
    VkPhysicalDeviceProperties2 props;
    props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    props.pNext = &_raytracingProperties;
    props.properties = { };
    vkGetPhysicalDeviceProperties2(_physicalDevice, &props);
}

void TutorialApplication::Cleanup()
{
    Application::Cleanup();
}

void main(int argc, const char* argv[])
{
    RunApplication<TutorialApplication>();
}
