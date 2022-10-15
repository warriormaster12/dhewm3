#pragma once

#include <cstdint>
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
        if(commandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device,commandPool, nullptr);
        }

        if(renderFence != VK_NULL_HANDLE) {
            vkDestroyFence(device, renderFence, nullptr);
        }

        if(presentSemaphore != VK_NULL_HANDLE) {
            vkDestroySemaphore(device, presentSemaphore, nullptr);
        }

        if(renderSemaphore != VK_NULL_HANDLE) {
            vkDestroySemaphore(device, renderSemaphore, nullptr);
        }

    }
};

/*
===============
PipelineBuilder
===============
*/
struct idPipeline {
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineLayout pipelineLayout;
    bool depthEnabled;
    struct DescriptorInfo {
        struct BindingInfo {
            VkShaderStageFlags shaderStageFlags;
            VkDescriptorType descriptorTypes;
            uint32_t binding = 0;
        };
        
        std::vector<BindingInfo> bindingInfo;
    };
    std::vector<DescriptorInfo> descriptorSets;
    void DestroyPipeline();
};

class idPipelineBuilder {
public:
	virtual bool BuildGraphicsPipeline(std::vector<const char*> files, std::vector<VkShaderStageFlagBits> shaderStageFlags, idPipeline& idpipeline) = 0;
};

extern idPipelineBuilder* 	pipelinebuilder;

class idVulkanDevice {
public:
    virtual void        Init( void ) = 0;
    virtual void        CreateSwapchain( uint32_t width, uint32_t height ) = 0; 
    virtual void        Shutdown( void ) = 0;
    virtual VmaAllocator& GetGlobalMemoryAllocator( void ) = 0;
    virtual VkDevice& GetGlobalDevice( void ) = 0;
    virtual VkSwapchainKHR& GetGlobalSwapchain( void ) = 0;
    virtual VkQueue& GetGraphicsQueue( void ) = 0;
    virtual VkExtent2D& GetSwapchainExtent( void ) = 0;
    virtual VkImageView& GetCurrentSwapchainImageView( uint32_t& imageIndex ) = 0;
    virtual VkImage&     GetCurrentSwapchainImage( uint32_t& imageIndex ) = 0;
    virtual VkFormat&     GetCurrentSwapchainImageFormat( void ) = 0;
    virtual VkImage&     GetDepthImage( void ) = 0;
    virtual VkImageView& GetDepthImageView( void ) = 0;
    virtual VkFormat&    GetDepthFormat( void ) = 0;

    virtual PerFrameData& GetCurrentFrame( int frameNumber ) = 0;
    virtual int GetFrameOverlap( void ) const = 0;


    bool vkInitialized = false;
};

class idVulkanRBE {
public: 
    virtual void        PrepareFrame( void ) = 0;
    virtual void        SubmitFrame( void ) = 0;
    
    //framebuffer and renderpass replacement
    virtual void        BeginRenderLayer( uint32_t width = 0, uint32_t height = 0 ) = 0;
    virtual void        EndRenderLayer( void ) = 0; 

    virtual void        CleanUp( void ) = 0;

    int frameCount = 0;
};

extern idVulkanDevice * 	vkdevice;
extern idVulkanRBE *        vkrbe;

class DescriptorLayoutCache {
public: 
    virtual VkDescriptorSetLayout CreateDescriptorLayout( VkDescriptorSetLayoutCreateInfo& info ) = 0;
    virtual void CleanUp( void ) = 0;
};

extern DescriptorLayoutCache* descriptorLayoutCache;




