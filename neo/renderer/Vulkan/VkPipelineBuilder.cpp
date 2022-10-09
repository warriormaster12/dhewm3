#include "VkTools.h"
#include "VkInit.h"

#include <fstream>


class idPipelineBuilderLocal : public idPipelineBuilder {
public:
	virtual idPipeline BuildGraphicsPipeline(std::vector<const char*> files, std::vector<VkShaderStageFlagBits> shaderStageFlags);
private:
    VkShaderModule LoadShaderModule(const char* shaderFile);
};

idPipelineBuilderLocal pipelinebuilderLocal;
idPipelineBuilder* pipelinebuilder = &pipelinebuilderLocal;

VkShaderModule idPipelineBuilderLocal::LoadShaderModule(const char* shaderFile) {

    //open the file. With cursor at the end
	std::ifstream file(shaderFile, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		return VK_NULL_HANDLE;
	}


    //find what the size of the file is by looking up the location of the cursor
    //because the cursor is at the end, it gives the size directly in bytes
    size_t fileSize = (size_t)file.tellg();

    //spirv expects the buffer to be on uint32, so make sure to reserve an int vector big enough for the entire file
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    //put file cursor at beginning
    file.seekg(0);

    //load the entire file into the buffer
    file.read((char*)buffer.data(), fileSize);

    //now that the file is loaded into the buffer, we can close it
    file.close();


    //create a new shader module, using the buffer we loaded
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;

    //codeSize has to be in bytes, so multiply the ints in the buffer by size of int to know the real size of the buffer
    createInfo.codeSize = buffer.size() * sizeof(uint32_t);
    createInfo.pCode = buffer.data();

    //check that the creation goes well.
    VkShaderModule module;
    if (vkCreateShaderModule(vkdevice->GetGlobalDevice(), &createInfo, nullptr, &module) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    return module;
}


VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule) {

    VkPipelineShaderStageCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.pNext = nullptr;

    //shader stage
    info.stage = stage;
    //module containing the code for this shader stage
    info.module = shaderModule;
    //the entry point of the shader
    info.pName = "main";
    return info;
}

VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo() {
    VkPipelineLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pNext = nullptr;

    //empty defaults
    info.flags = 0;
    info.setLayoutCount = 0;
    info.pSetLayouts = nullptr;
    info.pushConstantRangeCount = 0;
    info.pPushConstantRanges = nullptr;
    return info;
}

VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo() {
    VkPipelineVertexInputStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.pNext = nullptr;

    //no vertex bindings or attributes
    info.vertexBindingDescriptionCount = 0;
    info.vertexAttributeDescriptionCount = 0;
    return info;
}

VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo(VkPrimitiveTopology topology) {
    VkPipelineInputAssemblyStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.pNext = nullptr;

    info.topology = topology;
    //we are not going to use primitive restart on the entire tutorial so leave it on false
    info.primitiveRestartEnable = VK_FALSE;
    return info;
}

VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo(VkPolygonMode polygonMode)
{
    VkPipelineRasterizationStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.pNext = nullptr;

    info.depthClampEnable = VK_FALSE;
    //discards all primitives before the rasterization stage if enabled which we don't want
    info.rasterizerDiscardEnable = VK_FALSE;

    info.polygonMode = polygonMode;
    info.lineWidth = 1.0f;
    //no backface cull
    info.cullMode = VK_CULL_MODE_NONE;
    info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    //no depth bias
    info.depthBiasEnable = VK_FALSE;
    info.depthBiasConstantFactor = 0.0f;
    info.depthBiasClamp = 0.0f;
    info.depthBiasSlopeFactor = 0.0f;

    return info;
}


VkPipelineMultisampleStateCreateInfo MultisamplingStateCreateInfo()
{
    VkPipelineMultisampleStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    info.pNext = nullptr;

    info.sampleShadingEnable = VK_FALSE;
    //multisampling defaulted to no multisampling (1 sample per pixel)
    info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    info.minSampleShading = 1.0f;
    info.pSampleMask = nullptr;
    info.alphaToCoverageEnable = VK_FALSE;
    info.alphaToOneEnable = VK_FALSE;
    return info;
}

VkPipelineColorBlendAttachmentState ColorBlendAttachmentState() {
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    return colorBlendAttachment;
}





idPipeline idPipelineBuilderLocal::BuildGraphicsPipeline(std::vector<const char *> files, std::vector<VkShaderStageFlagBits> shaderStageFlags) {
    idPipeline idPipeline;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineMultisampleStateCreateInfo multisampling;

    // layout and shader modules creation
    std::vector<VkShaderModule> shaderModules;
    for(int i = 0; i < files.size(); i++) {
        shaderModules.push_back(LoadShaderModule(files[i]));
    }

    //build the pipeline layout that controls the inputs/outputs of the shader
	//we are not using descriptor sets or other systems yet, so no need to use anything other than empty default
	VkPipelineLayoutCreateInfo PipelineLayoutInfo = PipelineLayoutCreateInfo();

	ID_VK_CHECK_RESULT(vkCreatePipelineLayout(vkdevice->GetGlobalDevice(), &PipelineLayoutInfo, nullptr, &idPipeline.pipelineLayout));

	//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage

    for(int i = 0; i < shaderStageFlags.size(); i++) {
        shaderStages.push_back(PipelineShaderStageCreateInfo(shaderStageFlags[i],shaderModules[i]));
    }
	//vertex input controls how to read vertices from vertex buffers. We aren't using it yet
	vertexInputInfo = VertexInputStateCreateInfo();

	//input assembly is the configuration for drawing triangle lists, strips, or individual points.
	//we are just going to draw triangle list
	inputAssembly = InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	//configure the rasterizer to draw filled triangles
	rasterizer = RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);

	//we don't use multisampling, so just run the default one
	multisampling = MultisamplingStateCreateInfo();

	//a single blend attachment with no blending and writing to RGBA
	colorBlendAttachment = ColorBlendAttachmentState();

    //make viewport state from our stored viewport and scissor.
    //at the moment we won't support multiple viewports or scissors
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;

    viewportState.viewportCount = 1;
    viewportState.pViewports = &idPipeline.viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &idPipeline.scissor;

    //setup dummy color blending. We aren't using transparent objects yet
    //the blending is just "no blend", but we do write to the color attachment
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext = nullptr;

    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    //dynamic states
    std::vector <VkDynamicState> dStates = {VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR}; 
    if(rasterizer.depthBiasEnable == true)
    {
        dStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
    }
    VkPipelineDynamicStateCreateInfo dStateInfo = {};
    dStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dStateInfo.dynamicStateCount = dStates.size();
    dStateInfo.pDynamicStates = dStates.data();

    //build the actual pipeline
	//we now use all of the info structs we have been writing into into this one to create the pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = nullptr;

	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = idPipeline.pipelineLayout;
    pipelineInfo.pDynamicState = &dStateInfo;
	//pipelineInfo.renderPass = pass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    // New create info to define color, depth and stencil attachments at pipeline create time
    VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo = {};
    pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    pipelineRenderingCreateInfo.colorAttachmentCount = 1;
    pipelineRenderingCreateInfo.pColorAttachmentFormats = &vkdevice->GetCurrentSwapchainImageFormat();
    
    // pipelineRenderingCreateInfo.depthAttachmentFormat = vkdevice->GetDepthFormat();
    // pipelineRenderingCreateInfo.stencilAttachmentFormat =  vkdevice->GetDepthFormat();
    pipelineRenderingCreateInfo.viewMask = 0;
    
    pipelineInfo.pNext = &pipelineRenderingCreateInfo;

	//it's easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
	if (vkCreateGraphicsPipelines(vkdevice->GetGlobalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &idPipeline.pipeline) == VK_SUCCESS) {
        for(auto& shaderModule : shaderModules) {
            vkDestroyShaderModule(vkdevice->GetGlobalDevice(), shaderModule, nullptr);
        }
		return idPipeline;
	}
    common->Error("failed to create a graphics pipeline");
    return idPipeline;
}


idPipeline::~idPipeline() {
    vkDestroyPipelineLayout(vkdevice->GetGlobalDevice(), pipelineLayout, nullptr);
    vkDestroyPipeline(vkdevice->GetGlobalDevice(), pipeline, nullptr);
}