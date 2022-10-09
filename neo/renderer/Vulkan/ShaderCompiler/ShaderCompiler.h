#pragma once 

#include <glslang/Public/ShaderLang.h>

#include "renderer/Vulkan/VkTools.h"

struct ShaderModule {
    std::vector<uint32_t> code;
    VkShaderModule module;
    VkShaderStageFlagBits stage;
};

class idShaderStageBuilder {
    
};

extern idShaderStageBuilder * 	shaderBuilder;