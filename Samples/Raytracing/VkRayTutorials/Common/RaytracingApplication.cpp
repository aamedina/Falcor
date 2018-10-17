#include "RaytracingApplication.h"

RaytracingApplication::RaytracingApplication()
{
}

// ============================================================
// Tutorial 01: Create device with raytracing support
// ============================================================
void RaytracingApplication::CreateDevice()
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

    VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptorIndexing = { };
    descriptorIndexing.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;

    VkPhysicalDeviceFeatures2 features2 = { };
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    auto found = std::find(_deviceExtensions.begin(), _deviceExtensions.end(), VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    if (found != _deviceExtensions.end())
        features2.pNext = &descriptorIndexing;

    vkGetPhysicalDeviceFeatures2( _physicalDevice, &features2 );

    VkDeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = &features2;
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.queueCreateInfoCount = (uint32_t)deviceQueueCreateInfos.size();
    deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = nullptr;
    deviceCreateInfo.enabledExtensionCount = (uint32_t)_deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = _deviceExtensions.data();
    deviceCreateInfo.pEnabledFeatures = nullptr;

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

void RaytracingApplication::InitRaytracing()
{
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
