#pragma once

#define VK_NO_PROTOTYPES
#include "third-party/vk-bootstrap/VkBootstrap.h"
#include "third-party/volk/volk.h"


class idVulkanDevice {
public:
    virtual void        Init( void ) = 0; 
};



extern idVulkanDevice * 	vkdevice;
