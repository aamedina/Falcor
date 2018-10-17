#include "../Common/RaytracingApplication.h"

class TutorialApplication : public RaytracingApplication
{
public:
    VkDeviceMemory _topASMemory = VK_NULL_HANDLE;
    VkAccelerationStructureNVX _topAS = VK_NULL_HANDLE;
    VkDeviceMemory _bottomASMemory = VK_NULL_HANDLE;
    VkAccelerationStructureNVX _bottomAS = VK_NULL_HANDLE;

public:
    TutorialApplication();

    virtual void Init() override;
    virtual void Cleanup() override;

    void CreateAccelerationStructures();         // Tutorial 02
};

TutorialApplication::TutorialApplication()
{
    _appName = L"VkRay Tutorial 02: Building Acceleration Structure";
    _deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    _deviceExtensions.push_back(VK_NVX_RAYTRACING_EXTENSION_NAME);
}

void TutorialApplication::Init()
{
    InitRaytracing();

    CreateAccelerationStructures();              // Tutorial 02
}

// ============================================================
// Tutorial 02: Create Raytracing Acceleration Structures
// ============================================================
void TutorialApplication::CreateAccelerationStructures()
{
    // ============================================================
    // 1. CREATE GEOMETRY
    // Convert vertex/index data into buffers and then use the
    // buffers to create VkGeometryNV struct
    // ============================================================

    // Notice that vertex/index buffers have to be alive while
    // geometry is used because it references them
    BufferResource vertexBuffer;
    BufferResource indexBuffer;
    std::vector<VkGeometryNVX> geometries;

    {
        struct Vertex
        {
            float X, Y, Z;
        };

        std::vector<Vertex> vertices
        {
            { -0.5f, -0.5f, 0.0f },
            { +0.0f, +0.5f, 0.0f },
            { +0.5f, -0.5f, 0.0f }
        };
        const uint32_t vertexCount = (uint32_t)vertices.size();
        const VkDeviceSize vertexSize = sizeof(Vertex);
        const VkDeviceSize vertexBufferSize = vertexCount * vertexSize;

        std::vector<uint16_t> indices
        {
            { 0, 1, 2 }
        };
        const uint32_t indexCount = (uint32_t)indices.size();
        const VkDeviceSize indexSize = sizeof(uint16_t);
        const VkDeviceSize indexBufferSize = indexCount * indexSize;

        VkResult code;
        bool copied;

        code = vertexBuffer.Create(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        NVVK_CHECK_ERROR(code, L"rt vertexBuffer.Create");
        copied = vertexBuffer.CopyToBufferUsingMapUnmap(vertices.data(), vertexBufferSize);
        if (!copied)
        {
            ExitError(L"Failed to copy vertex buffer");
        }

        code = indexBuffer.Create(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        NVVK_CHECK_ERROR(code, L"rt indexBuffer.Create");
        copied = indexBuffer.CopyToBufferUsingMapUnmap(indices.data(), indexBufferSize);
        if (!copied)
        {
            ExitError(L"Failed to copy index buffer");
        }

        VkGeometryNVX geometry;
        geometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NVX;
        geometry.pNext = nullptr;
        geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NVX;
        geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NVX;
        geometry.geometry.triangles.pNext = nullptr;
        geometry.geometry.triangles.vertexData = vertexBuffer.Buffer;
        geometry.geometry.triangles.vertexOffset = 0;
        geometry.geometry.triangles.vertexCount = vertexCount;
        geometry.geometry.triangles.vertexStride = vertexSize;
        geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        geometry.geometry.triangles.indexData = indexBuffer.Buffer;
        geometry.geometry.triangles.indexOffset = 0;
        geometry.geometry.triangles.indexCount = indexCount;
        geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT16;
        geometry.geometry.triangles.transformData = VK_NULL_HANDLE;
        geometry.geometry.triangles.transformOffset = 0;
        geometry.geometry.aabbs = { };
        geometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NVX;
        geometry.flags = 0;

        geometries.emplace_back(geometry);
    }

    // ============================================================
    // 2. CREATE BOTTOM LEVEL ACCELERATION STRUCTURES
    // Bottom level AS corresponds to the geometry.
    // ============================================================

    auto CreateAccelerationStructure = [&](VkAccelerationStructureTypeNVX type, uint32_t geometryCount,
        VkGeometryNVX* geometries, uint32_t instanceCount, VkAccelerationStructureNVX& AS, VkDeviceMemory& memory)
    {
        VkAccelerationStructureCreateInfoNVX accelerationStructureInfo;
        accelerationStructureInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NVX;
        accelerationStructureInfo.pNext = nullptr;
        accelerationStructureInfo.type = type;
        accelerationStructureInfo.flags = 0;
        accelerationStructureInfo.compactedSize = 0;
        accelerationStructureInfo.instanceCount = instanceCount;
        accelerationStructureInfo.geometryCount = geometryCount;
        accelerationStructureInfo.pGeometries = geometries;

        VkResult code = vkCreateAccelerationStructureNVX(_device, &accelerationStructureInfo, nullptr, &AS);
        NVVK_CHECK_ERROR(code, L"vkCreateAccelerationStructureNV");

        VkAccelerationStructureMemoryRequirementsInfoNVX memoryRequirementsInfo;
        memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NVX;
        memoryRequirementsInfo.pNext = nullptr;
        memoryRequirementsInfo.accelerationStructure = AS;

        VkMemoryRequirements2 memoryRequirements;
        vkGetAccelerationStructureMemoryRequirementsNVX(_device, &memoryRequirementsInfo, &memoryRequirements);

        VkMemoryAllocateInfo memoryAllocateInfo;
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.pNext = nullptr;
        memoryAllocateInfo.allocationSize = memoryRequirements.memoryRequirements.size;
        memoryAllocateInfo.memoryTypeIndex = ResourceBase::GetMemoryType(memoryRequirements.memoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        code = vkAllocateMemory(_device, &memoryAllocateInfo, nullptr, &memory);
        NVVK_CHECK_ERROR(code, L"rt AS vkAllocateMemory");

        VkBindAccelerationStructureMemoryInfoNVX bindInfo;
        bindInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NVX;
        bindInfo.pNext = nullptr;
        bindInfo.accelerationStructure = AS;
        bindInfo.memory = memory;
        bindInfo.memoryOffset = 0;
        bindInfo.deviceIndexCount = 0;
        bindInfo.pDeviceIndices = nullptr;

        code = vkBindAccelerationStructureMemoryNVX(_device, 1, &bindInfo);
        NVVK_CHECK_ERROR(code, L"vkBindAccelerationStructureMemoryNV");
    };

    CreateAccelerationStructure(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NVX,
        (uint32_t)geometries.size(), geometries.data(), 0,
        _bottomAS, _bottomASMemory);

    // ============================================================
    // 3. CREATE INSTANCE BUFFER
    // There can be many instances of the single geometry. Create
    // instances using various transforms.
    // ============================================================

    BufferResource instanceBuffer;

    {
        uint64_t accelerationStructureHandle;
        VkResult code = vkGetAccelerationStructureHandleNVX(_device, _bottomAS, sizeof(uint64_t), &accelerationStructureHandle);
        NVVK_CHECK_ERROR(code, L"vkGetAccelerationStructureHandleNV");

        float transform[12] =
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
        };

        VkGeometryInstance instance;
        memcpy(instance.transform, transform, sizeof(instance.transform));
        instance.instanceId = 0;
        instance.mask = 0xff;
        instance.instanceOffset = 0;
        instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NVX;
        instance.accelerationStructureHandle = accelerationStructureHandle;

        uint32_t instanceBufferSize = (uint32_t)sizeof(VkGeometryInstance);

        code = instanceBuffer.Create(instanceBufferSize, VK_BUFFER_USAGE_RAYTRACING_BIT_NVX, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        NVVK_CHECK_ERROR(code, L"rt instanceBuffer.Create");
        instanceBuffer.CopyToBufferUsingMapUnmap(&instance, instanceBufferSize);
    }

    // ============================================================
    // 4. CREATE TOP LEVEL ACCELERATION STRUCTURES
    // Top level AS encompasses bottom level acceleration structures.
    // ============================================================

    CreateAccelerationStructure(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NVX,
        0, nullptr, 1,
        _topAS, _topASMemory);

    // ============================================================
    // 5. BUILD ACCELERATION STRUCTURES
    // Finally fill acceleration structures using all the data.
    // ============================================================

    auto GetScratchBufferSize = [&](VkAccelerationStructureNVX handle)
    {
        VkAccelerationStructureMemoryRequirementsInfoNVX memoryRequirementsInfo;
        memoryRequirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NVX;
        memoryRequirementsInfo.pNext = nullptr;
        memoryRequirementsInfo.accelerationStructure = handle;

        VkMemoryRequirements2 memoryRequirements;
        vkGetAccelerationStructureScratchMemoryRequirementsNVX(_device, &memoryRequirementsInfo, &memoryRequirements);

        VkDeviceSize result = memoryRequirements.memoryRequirements.size;
        return result;
    };

    {
        VkDeviceSize bottomAccelerationStructureBufferSize = GetScratchBufferSize(_bottomAS);
        VkDeviceSize topAccelerationStructureBufferSize = GetScratchBufferSize(_topAS);
        VkDeviceSize scratchBufferSize = std::max(bottomAccelerationStructureBufferSize, topAccelerationStructureBufferSize);

        BufferResource scratchBuffer;
        scratchBuffer.Create(scratchBufferSize, VK_BUFFER_USAGE_RAYTRACING_BIT_NVX, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


        VkCommandBufferAllocateInfo commandBufferAllocateInfo;
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.pNext = nullptr;
        commandBufferAllocateInfo.commandPool = _commandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        VkResult code = vkAllocateCommandBuffers(_device, &commandBufferAllocateInfo, &commandBuffer);
        NVVK_CHECK_ERROR(code, L"rt vkAllocateCommandBuffers");

        VkCommandBufferBeginInfo beginInfo;
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        beginInfo.pInheritanceInfo = nullptr;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkMemoryBarrier memoryBarrier;
        memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.pNext = nullptr;
        memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NVX | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NVX;
        memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NVX | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NVX;


        vkCmdBuildAccelerationStructureNVX(commandBuffer, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NVX, 0, VK_NULL_HANDLE, 0,
            (uint32_t)geometries.size(), &geometries[0], 0, VK_FALSE, _bottomAS, VK_NULL_HANDLE, scratchBuffer.Buffer, 0);

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_RAYTRACING_BIT_NVX, VK_PIPELINE_STAGE_RAYTRACING_BIT_NVX, 0, 1, &memoryBarrier, 0, 0, 0, 0);

        vkCmdBuildAccelerationStructureNVX(commandBuffer, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NVX, 1, instanceBuffer.Buffer, 0,
            0, nullptr, 0, VK_FALSE, _topAS, VK_NULL_HANDLE, scratchBuffer.Buffer, 0);

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_RAYTRACING_BIT_NVX, VK_PIPELINE_STAGE_RAYTRACING_BIT_NVX, 0, 1, &memoryBarrier, 0, 0, 0, 0);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo;
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = nullptr;
        submitInfo.pWaitDstStageMask = nullptr;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;

        vkQueueSubmit(_queuesInfo.Graphics.Queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(_queuesInfo.Graphics.Queue);
        vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
    }
}

void TutorialApplication::Cleanup()
{
    if (_topAS)
    {
        vkDestroyAccelerationStructureNVX(_device, _topAS, nullptr);
    }
    if (_topASMemory)
    {
        vkFreeMemory(_device, _topASMemory, nullptr);
    }
    if (_bottomAS)
    {
        vkDestroyAccelerationStructureNVX(_device, _bottomAS, nullptr);
    }
    if (_bottomASMemory)
    {
        vkFreeMemory(_device, _bottomASMemory, nullptr);
    }

    Application::Cleanup();
}

void main(int argc, const char* argv[])
{
    RunApplication<TutorialApplication>();
}
