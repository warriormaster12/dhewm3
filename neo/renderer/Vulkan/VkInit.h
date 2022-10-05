#pragma once

#define VK_NO_PROTOTYPES
#include "third-party/vk-bootstrap/VkBootstrap.h"
#include "third-party/volk/volk.h"

#include "third-party/vma/vk_mem_alloc.h"


struct PerFrameData {
    VkCommandBuffer commandBuffer;
    VkFence renderFence;
    VkSemaphore presentSemaphore, renderSemaphore;
    VkCommandPool commandPool;

    void CleanUp(VkDevice& device) {
        vkDestroyCommandPool(device,commandPool, nullptr);
    }
};

class idVulkanDevice {
public:
    virtual void        Init( void ) = 0;
    virtual void        CreateSwapchain( uint32_t width, uint32_t height ) = 0; 
    virtual void        Shutdown( void ) = 0;
    virtual VmaAllocator& GetGlobalMemoryAllocator( void ) = 0;

    virtual PerFrameData& GetCurrentFrame( int frameNumber ) = 0;
    virtual int GetFrameOverlap( void ) const = 0;
};



extern idVulkanDevice * 	vkdevice;
