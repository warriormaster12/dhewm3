#include "VkInit.h"

#include "SDL_video.h"
#include "framework/Common.h"

#include "renderer/tr_local.h"

#include <SDL_vulkan.h>


class idVulkanDeviceLocal : public idVulkanDevice {
public:
    virtual void       Init( void );
private:
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties gpuProperties;

    VkQueue graphicsQueue;
    uint32_t graphicsQueueFamily;
};

idVulkanDeviceLocal vkdevice_local;

idVulkanDevice* vkdevice = &vkdevice_local;

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
	vkb::PhysicalDevice physicalDeviceSelector = selector
		.set_minimum_version(1, 3)
		.set_surface(surface)
		.select()
		.value();
    
    //create the final Vulkan device
	vkb::DeviceBuilder deviceBuilder{ physicalDeviceSelector };

	vkb::Device vkbDevice = deviceBuilder.build().value();


	// Get the VkDevice handle used in the rest of a Vulkan application
	device = vkbDevice.device;
	physicalDevice = physicalDeviceSelector.physical_device;

    // use vkbootstrap to get a Graphics queue
	graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();

	graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    common->Printf("Vulkan Device created\n");

    vkGetPhysicalDeviceProperties(physicalDevice, &gpuProperties);
    common->Printf("GPU Vendor:%s Vulkan Version: %d\n", physicalDeviceSelector.properties.deviceName,physicalDeviceSelector.properties.apiVersion);

}
