#include "VkInit.h"

#include "framework/Common.h"


class idVulkanDeviceLocal : public idVulkanDevice {
public:
    virtual void       Init( void );
private:
    VkInstance instance;
};

idVulkanDeviceLocal vkdevice_local;

idVulkanDevice* vkdevice = &vkdevice_local;

void idVulkanDeviceLocal::Init( void ){
    common->Printf( "----- Initializing Vulkan -----\n" );

    volkInitialize();


    vkb::InstanceBuilder builder;

    auto inst_ret = builder.set_app_name("dhwem 3")
    .request_validation_layers(true)
    .use_default_debug_messenger()
    .build();

    vkb::Instance vkb_inst = inst_ret.value();

    instance = vkb_inst.instance;
    volkLoadInstance(instance);

    common->Printf("Vulkan instance created\n");
}
