#include "VkInit.h"
#include "VkTools.h"

#include "SDL_video.h"
#include "framework/Common.h"

#include "renderer/tr_local.h"

#include <SDL_vulkan.h>

#include <vector>
#include "renderer/VertexCache.h"



class idVulkanDeviceLocal : public idVulkanDevice {
public:
    virtual void       Init( void );
    virtual void       CreateSwapchain( uint32_t width, uint32_t height );
    virtual void       Shutdown( void );
    virtual VmaAllocator& GetGlobalMemoryAllocator( void );
    virtual VkDevice& GetGlobalDevice( void );
    VkSwapchainKHR& GetGlobalSwapchain( void );
    VkQueue& GetGraphicsQueue( void );
    virtual VkExtent2D& GetSwapchainExtent( void );
    virtual VkImageView& GetCurrentSwapchainImageView( uint32_t& imageIndex );
    virtual VkImage&     GetCurrentSwapchainImage( uint32_t& imageIndex );
    virtual VkFormat&     GetCurrentSwapchainImageFormat( void );
    virtual VkImage&     GetDepthImage( void );
    virtual VkImageView& GetDepthImageView( void );
    virtual VkFormat&    GetDepthFormat( void );

    virtual PerFrameData& GetCurrentFrame( int frameNumber ) { return perFrameData[frameNumber % frameOverlap];}
    virtual int GetFrameOverlap( void ) const {return frameOverlap;}
private:
    void CreateCommandBuffer( void );

    void CreateSyncObjects( void );

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties gpuProperties;

    VkQueue graphicsQueue;
    uint32_t graphicsQueueFamily;

    VmaAllocator allocator;

    //swapchain
    VkSwapchainKHR swapchain;
    VkExtent2D swapchainExtent;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    VkFormat swapchainImageFormat;

    idVkTools::AllocatedImage depthImage;

    static const int frameOverlap = 2;
    PerFrameData perFrameData[frameOverlap];
};

idVulkanDeviceLocal vkdeviceLocal;

idVulkanDevice* vkdevice = &vkdeviceLocal;


VmaAllocator& idVulkanDeviceLocal::GetGlobalMemoryAllocator( void ) {
    return allocator;
}

VkDevice& idVulkanDeviceLocal::GetGlobalDevice( void ) {
    return device;
}

VkSwapchainKHR& idVulkanDeviceLocal::GetGlobalSwapchain( void ) {
    return swapchain;
}

VkQueue& idVulkanDeviceLocal::GetGraphicsQueue( void ) {
    return graphicsQueue;
}
VkExtent2D& idVulkanDeviceLocal::GetSwapchainExtent( void ) {
    return swapchainExtent;
}
VkImageView& idVulkanDeviceLocal::GetCurrentSwapchainImageView( uint32_t& imageIndex ) {
    return swapchainImageViews[imageIndex];
}
VkImage& idVulkanDeviceLocal::GetCurrentSwapchainImage( uint32_t& imageIndex ) {
    return swapchainImages[imageIndex];
}

VkFormat& idVulkanDeviceLocal::GetCurrentSwapchainImageFormat( void ) {
    return swapchainImageFormat;
}

VkImage& idVulkanDeviceLocal::GetDepthImage( void ) {
    return depthImage.image;
}
VkImageView& idVulkanDeviceLocal::GetDepthImageView( void ) {
    return depthImage.defaultView;
}
VkFormat& idVulkanDeviceLocal::GetDepthFormat( void ) {
    return depthImage.imageFormat;
}

VkBool32 GetSupportedDepthFormat(VkPhysicalDevice& physicalDevice, VkFormat& depthFormat) {
    // Since all depth formats may be optional, we need to find a suitable depth format to use
    // Start with the highest precision packed format
    std::vector<VkFormat> depthFormats = {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM
    };

    for (auto& format : depthFormats)
    {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
        // Format must support depth stencil attachment for optimal tiling
        if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            depthFormat = format;
            return true;
        }
    }

    return false;
}

VkImageCreateInfo ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent)
{
    VkImageCreateInfo info = { };
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = nullptr;

    info.imageType = VK_IMAGE_TYPE_2D;
    info.format = format;
    info.extent = extent;

    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = usageFlags;

    return info;
}

VkImageViewCreateInfo ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags)
{
    //build a image-view for the depth image to use for rendering
    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = nullptr;

    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.image = image;
    info.format = format;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    info.subresourceRange.aspectMask = aspectFlags;

    return info;
}

void idVulkanDeviceLocal::CreateCommandBuffer( void ) {
    VkCommandPoolCreateInfo command_pool_info = idVkTools::CommandPoolCreateInfo(graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    for(auto& frame : perFrameData){
        ID_VK_CHECK_RESULT(vkCreateCommandPool(device, &command_pool_info, nullptr, &frame.commandPool));

        VkCommandBufferAllocateInfo cmdAllocInfo = idVkTools::CommandBufferAllocateInfo(frame.commandPool);

        ID_VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdAllocInfo, &frame.commandBuffer));
    }

    //TODO: upload commands
	// VkCommandPoolCreateInfo upload_command_pool_info = vkdefaults::CommandPoolCreateInfo(DeviceManager::GetVkDevice().graphics_queue_family);
	// //create pool for upload context
	// VK_CHECK_RESULT(vkCreateCommandPool(DeviceManager::GetVkDevice().device, &upload_command_pool_info, nullptr, &upload_context.command_pool));

    common->Printf("Vulkan Command Buffers created\n");
}

void idVulkanDeviceLocal::CreateSyncObjects( void ) {
    //create syncronization structures
	//one fence to control when the gpu has finished rendering the frame,
	//and 2 semaphores to syncronize rendering with swapchain
	//we want the fence to start signalled so we can wait on it on the first frame
	VkFenceCreateInfo fenceCreateInfo = idVkTools::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

	VkSemaphoreCreateInfo semaphoreCreateInfo = idVkTools::SemaphoreCreateInfo();

	for (auto& frame : perFrameData) {

		ID_VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &frame.renderFence));


		ID_VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frame.presentSemaphore));
		ID_VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frame.renderSemaphore));
		
	}

    //TODO: upload sync
	// VkFenceCreateInfo uploadFenceCreateInfo = vkdefaults::FenceCreateInfo();

	// ID_VK_CHECK_RESULT(vkCreateFence(DeviceManager::GetVkDevice().device, &uploadFenceCreateInfo, nullptr, &upload_context.upload_fence));

    common->Printf("Vulkan Sync Objects created\n");
}


void idVulkanDeviceLocal::Init( void ){
    common->Printf( "Initializing Vulkan Device\n" );

    volkInitialize();


    vkb::InstanceBuilder builder;

    auto inst_ret = builder.set_app_name("dhwem 3")
    .request_validation_layers(true)
    .use_default_debug_messenger()
    .build();

    vkb::Instance vkb_inst = inst_ret.value();

    instance = vkb_inst.instance;
    volkLoadInstance(instance);
    debugMessenger = vkb_inst.debug_messenger;

    common->Printf("Vulkan instance created\n");

    // get the surface of the window we opened with SDL
	SDL_Vulkan_CreateSurface(static_cast<SDL_Window*>(GLimp_GetWindowPointer()), instance, &surface);

	//use vkbootstrap to select a GPU.
	//We want a GPU that can write to the SDL surface and supports Vulkan 1.3
	vkb::PhysicalDeviceSelector selector{ vkb_inst };

    VkPhysicalDeviceFeatures feats{};
    
    VkPhysicalDeviceDynamicRenderingFeatures dynamicFeatures = {};
    dynamicFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    dynamicFeatures.dynamicRendering = VK_TRUE;

    selector.set_required_features(feats);
    selector.add_desired_extension(VK_KHR_MAINTENANCE2_EXTENSION_NAME)
    .add_desired_extension(VK_KHR_MULTIVIEW_EXTENSION_NAME)
    .add_desired_extension(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME)
    .add_desired_extension(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME)
    .add_desired_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
    .add_desired_extension(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);

	vkb::PhysicalDevice physicalDeviceSelector = selector
		.set_minimum_version(1, 3)
		.set_surface(surface)
		.select()
		.value();
    
    //create the final Vulkan device
	vkb::DeviceBuilder deviceBuilder{ physicalDeviceSelector };

	vkb::Device vkbDevice = deviceBuilder.add_pNext(&dynamicFeatures).build().value();


	// Get the VkDevice handle used in the rest of a Vulkan application
	device = vkbDevice.device;
	physicalDevice = physicalDeviceSelector.physical_device;

    // use vkbootstrap to get a Graphics queue
	graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();

	graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    common->Printf("Vulkan Device created\n");

    //we need to provide instance and device address so that vma library could work properly with volk
    VmaVulkanFunctions volkFunctions = {};
    volkFunctions.vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)vkGetDeviceProcAddr;
    volkFunctions.vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)vkGetInstanceProcAddr;
    volkFunctions.vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)vkGetPhysicalDeviceProperties;
    volkFunctions.vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)vkGetPhysicalDeviceMemoryProperties;
    volkFunctions.vkAllocateMemory = (PFN_vkAllocateMemory)vkAllocateMemory;
    volkFunctions.vkFreeMemory = (PFN_vkFreeMemory)vkFreeMemory;
    volkFunctions.vkMapMemory = (PFN_vkMapMemory)vkMapMemory;
    volkFunctions.vkUnmapMemory = (PFN_vkUnmapMemory)vkUnmapMemory;
    volkFunctions.vkGetDeviceBufferMemoryRequirements = (PFN_vkGetDeviceBufferMemoryRequirements)vkGetDeviceBufferMemoryRequirements;
    volkFunctions.vkGetDeviceImageMemoryRequirements = (PFN_vkGetDeviceImageMemoryRequirements)vkGetDeviceImageMemoryRequirements;
    volkFunctions.vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges)vkFlushMappedMemoryRanges;
    volkFunctions.vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges)vkInvalidateMappedMemoryRanges;
    volkFunctions.vkBindBufferMemory = (PFN_vkBindBufferMemory)vkBindBufferMemory;
    volkFunctions.vkBindImageMemory = (PFN_vkBindImageMemory)vkBindImageMemory;
    volkFunctions.vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)vkGetBufferMemoryRequirements;
    volkFunctions.vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)vkGetImageMemoryRequirements;
    volkFunctions.vkCreateBuffer = (PFN_vkCreateBuffer)vkCreateBuffer;
    volkFunctions.vkCreateImage = (PFN_vkCreateImage)vkCreateImage;
    volkFunctions.vkDestroyBuffer = (PFN_vkDestroyBuffer)vkDestroyBuffer;
    volkFunctions.vkDestroyImage = (PFN_vkDestroyImage)vkDestroyImage;
    volkFunctions.vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer)vkCmdCopyBuffer;
    volkFunctions.vkGetBufferMemoryRequirements2KHR = (PFN_vkGetBufferMemoryRequirements2KHR)vkGetBufferMemoryRequirements2KHR;
    volkFunctions.vkGetImageMemoryRequirements2KHR = (PFN_vkGetImageMemoryRequirements2KHR)vkGetImageMemoryRequirements2KHR;
    volkFunctions.vkBindBufferMemory2KHR = (PFN_vkBindBufferMemory2KHR)vkBindBufferMemory2KHR;
    volkFunctions.vkBindImageMemory2KHR = (PFN_vkBindImageMemory2KHR)vkBindImageMemory2KHR;
    volkFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)vkGetPhysicalDeviceMemoryProperties2KHR;
    volkFunctions.vkGetDeviceBufferMemoryRequirements = (PFN_vkGetDeviceBufferMemoryRequirements)vkGetDeviceBufferMemoryRequirements;
    volkFunctions.vkGetDeviceImageMemoryRequirements = (PFN_vkGetDeviceImageMemoryRequirements)vkGetDeviceImageMemoryRequirements;

    //initialize the memory allocator
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    allocatorInfo.pVulkanFunctions = &volkFunctions;
    vmaCreateAllocator(&allocatorInfo, &allocator);

    common->Printf("Vulkan Memory Allocator created");


    vkGetPhysicalDeviceProperties(physicalDevice, &gpuProperties);
    common->Printf("GPU Vendor:%s Vulkan Version: %d\n", physicalDeviceSelector.properties.deviceName,physicalDeviceSelector.properties.apiVersion);

    //creating and allocating at the application startup time should be enough
    CreateCommandBuffer();
    CreateSyncObjects();
}

void idVulkanDeviceLocal::CreateSwapchain( uint32_t width, uint32_t height ) {
    vkb::SwapchainBuilder swapchainBuilder{physicalDevice,device,surface };
	vkb::Swapchain vkbSwapchain = swapchainBuilder
		.use_default_format_selection()
		//use vsync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(width, height)
		.build()
		.value();

	//store swapchain and its related images
	swapchain = vkbSwapchain.swapchain;
	swapchainImages = vkbSwapchain.get_images().value();
	swapchainImageViews = vkbSwapchain.get_image_views().value();
    swapchainImageFormat = vkbSwapchain.image_format;

	//we get actual resolution of the displayed content
	swapchainExtent = vkbSwapchain.extent;
	//depth image size will match the window
	VkExtent3D depth_image_extent = {
		swapchainExtent.width,
		swapchainExtent.height,
		1
	};

	//Creating proper depth format for swapchain image
	GetSupportedDepthFormat(physicalDevice, depthImage.imageFormat);

	//the depth image will be a image with the format we selected and Depth Attachment usage flag
	VkImageCreateInfo dimg_info = ImageCreateInfo(depthImage.imageFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depth_image_extent);

	//for the depth image, we want to allocate it from gpu local memory
	VmaAllocationCreateInfo dimg_allocinfo = {};
	dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//allocate and create the image
	vmaCreateImage(allocator, &dimg_info, &dimg_allocinfo, &depthImage.image, &depthImage.allocation,nullptr);

	//build a image-view for the depth image to use for rendering
	VkImageViewCreateInfo dview_info = ImageViewCreateInfo(depthImage.imageFormat, depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);
	ID_VK_CHECK_RESULT(vkCreateImageView(device, &dview_info, nullptr, &depthImage.defaultView));
}


void idVulkanDeviceLocal::Shutdown( void ) {
    vkInitialized = false;
    vkDeviceWaitIdle(device);
    vkrbe->CleanUp();
    for ( auto frame : perFrameData ) {
        frame.CleanUp(device);
    }
    

    depthImage.DestroyImage();
    for (int i = 0; i < swapchainImageViews.size(); i++)
    {
        vkDestroyImageView(device,  swapchainImageViews[i], nullptr);
    }

    vertexCache.Shutdown();

    vkDestroySwapchainKHR(device,swapchain, nullptr);

    vmaDestroyAllocator(allocator);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    vkDestroyInstance(instance, nullptr);
}
