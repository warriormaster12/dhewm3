#pragma once

#define VK_NO_PROTOTYPES
#include "third-party/vk-bootstrap/VkBootstrap.h"
#include "third-party/volk/volk.h"

#include "third-party/vma/vk_mem_alloc.h"


class idVulkanDevice {
public:
    virtual void        Init( void ) = 0;
    virtual void        CreateSwapchain( uint32_t width, uint32_t height ) = 0; 
    virtual void        Shutdown( void ) = 0;
    virtual VmaAllocator& GetGlobalMemoryAllocator( void ) = 0;
};



extern idVulkanDevice * 	vkdevice;
