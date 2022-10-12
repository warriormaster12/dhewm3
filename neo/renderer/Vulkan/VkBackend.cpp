#include "VkInit.h"
#include "VkTools.h"

#include "idlib/math/Vector.h"

class idVulkanRBELocal : public idVulkanRBE {
public: 
    virtual void PrepareFrame( void );
    virtual void SubmitFrame( void );
    virtual void BeginRenderLayer( void );
    virtual void EndRenderLayer( void );
    virtual void CleanUp( void );
private:
    uint32_t imageIndex = 0;
};

idVulkanRBELocal vkrbeLocal;

idVulkanRBE* vkrbe = &vkrbeLocal;

idPipeline testPipeline;

idVkTools::AllocatedBuffer vertexBuffer;

bool doOnce = true;

struct Vertex {
    idVec3 position;
    idVec3 color;
};


void idVulkanRBELocal::PrepareFrame( void ){
    auto& currentFrame = vkdevice->GetCurrentFrame(frameCount);
    auto& device = vkdevice->GetGlobalDevice();

    //wait until the gpu has finished rendering the last frame. Timeout of 1 second
    ID_VK_CHECK_RESULT(vkWaitForFences(device, 1, &currentFrame.renderFence, true, 1000000000));
    ID_VK_CHECK_RESULT(vkResetFences(device, 1, &currentFrame.renderFence));

    //now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
	ID_VK_CHECK_RESULT(vkResetCommandBuffer(currentFrame.commandBuffer, 0));

    VkResult draw_result = vkAcquireNextImageKHR(
        device, 
        vkdevice->GetGlobalSwapchain(),
        1000000000,
        currentFrame.presentSemaphore, 
        nullptr,
        &imageIndex
    );

    //We won't handle swapchain resize since dhewm3 requires a restart anyway
    if (draw_result == VK_ERROR_OUT_OF_DATE_KHR) {
        return;
	} else if (draw_result != VK_SUCCESS && draw_result != VK_SUBOPTIMAL_KHR) {
		common->FatalError("failed to acquire swap chain image!");
		abort();
	}

    //begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = idVkTools::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	ID_VK_CHECK_RESULT(vkBeginCommandBuffer(currentFrame.commandBuffer, &cmdBeginInfo));
}

void idVulkanRBELocal::BeginRenderLayer( void ) {
    auto& currentFrame = vkdevice->GetCurrentFrame(frameCount);
    idVkTools::InsertImageMemoryBarrier(
        currentFrame.commandBuffer,
        vkdevice->GetCurrentSwapchainImage(imageIndex),
        0,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
    
    VkImageSubresourceRange depth_subresource_range = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
    // Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
    if (vkdevice->GetDepthFormat() >= VK_FORMAT_D16_UNORM_S8_UINT) {
		depth_subresource_range.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
    idVkTools::InsertImageMemoryBarrier(
        currentFrame.commandBuffer,
        vkdevice->GetDepthImage(),
        0,
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        depth_subresource_range);

    // New structures are used to define the attachments used in dynamic rendering
    VkRenderingAttachmentInfoKHR colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    colorAttachment.imageView = vkdevice->GetCurrentSwapchainImageView(imageIndex);
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = { 0.0f, 1.0f, 0.0f,1.0f };

    // A single depth stencil attachment info can be used, but they can also be specified separately.
    // When both are specified separately, the only requirement is that the image view is identical.			
    VkRenderingAttachmentInfoKHR depthStencilAttachment{};
    depthStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    depthStencilAttachment.imageView = vkdevice->GetDepthImageView();
    depthStencilAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
    depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthStencilAttachment.clearValue.depthStencil = { 1.0f,  0 };

    VkRenderingInfoKHR renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    renderingInfo.renderArea = { 0, 0, vkdevice->GetSwapchainExtent().width, vkdevice->GetSwapchainExtent().height };
    renderingInfo.layerCount = 1;
    renderingInfo.viewMask = 0;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    testPipeline.depthEnabled = false;
    if (testPipeline.depthEnabled) {
        renderingInfo.pDepthAttachment = &depthStencilAttachment;
        renderingInfo.pStencilAttachment = &depthStencilAttachment;
    }

    if(doOnce) {
        std::vector<Vertex> vertices;
        vertices.resize(3);
        //vertex positions
        vertices[0].position.Set(1.f, 1.f, 0.0f );
        vertices[1].position.Set(-1.f, 1.f, 0.0f );
        vertices[2].position.Set(0.f,-1.f, 0.0f);

        //vertex colors, all green
        vertices[0].color.Set(1.f,0.f, 0.0f); //pure red
        vertices[1].color.Set(0.f,1.f, 0.0f); //pure green
        vertices[2].color.Set(0.f,0.f, 1.0f); //pure blue
        vertexBuffer.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        vertexBuffer.AllocateBuffer(vertices.data(),vertices.size(), sizeof(Vertex));
        doOnce = false;
    }

    // Begin dynamic rendering
    vkCmdBeginRenderingKHR(currentFrame.commandBuffer, &renderingInfo);
    
    if(pipelinebuilder->BuildGraphicsPipeline({"base/renderprogs/simple_triangle.vert.spv","base/renderprogs/simple_triangle.frag.spv"}
        , {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT}, testPipeline)) 
    {
        testPipeline.viewport.width = vkdevice->GetSwapchainExtent().width;
        testPipeline.viewport.height = vkdevice->GetSwapchainExtent().height;
        testPipeline.viewport.x = 0.0f;
        testPipeline.viewport.y = 0.0f;
        testPipeline.viewport.minDepth = 0.0f;
        testPipeline.viewport.maxDepth = 1.0f;
        
        testPipeline.scissor.extent = vkdevice->GetSwapchainExtent();
        testPipeline.scissor.offset = {0, 0};

        vkCmdSetViewport(currentFrame.commandBuffer, 0, 1, &testPipeline.viewport);
        vkCmdSetScissor(currentFrame.commandBuffer, 0, 1,&testPipeline.scissor);

        //bind the mesh vertex buffer with offset 0
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(currentFrame.commandBuffer, 0, 1, &vertexBuffer.buffer, &offset);

        vkCmdBindPipeline(currentFrame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, testPipeline.pipeline);
        vkCmdDraw(currentFrame.commandBuffer, 3, 1, 0, 0);
    }
}

void idVulkanRBELocal::EndRenderLayer( void ) {
    auto& currentFrame = vkdevice->GetCurrentFrame(frameCount);

    // End dynamic rendering
    vkCmdEndRenderingKHR(currentFrame.commandBuffer);
    // Transition color image for presentation
    idVkTools::InsertImageMemoryBarrier(
        currentFrame.commandBuffer,
        vkdevice->GetCurrentSwapchainImage(imageIndex),
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
}

void idVulkanRBELocal::SubmitFrame( void ) {
    auto& currentFrame = vkdevice->GetCurrentFrame(frameCount);

    //finalize the command buffer (we can no longer add commands, but it can now be executed)
	ID_VK_CHECK_RESULT(vkEndCommandBuffer(currentFrame.commandBuffer));

	//prepare the submission to the queue. 
	//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the _renderSemaphore, to signal that rendering has finished

	VkSubmitInfo submit = idVkTools::SubmitInfo(&currentFrame.commandBuffer);
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit.pWaitDstStageMask = &waitStage;

	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &currentFrame.presentSemaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &currentFrame.renderSemaphore;

	//submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	ID_VK_CHECK_RESULT(vkQueueSubmit(vkdevice->GetGraphicsQueue(), 1, &submit, currentFrame.renderFence));

	//prepare present
	// this will put the image we just rendered to into the visible window.
	// we want to wait on the _renderSemaphore for that, 
	// as its necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = idVkTools::PresentInfo();

	presentInfo.pSwapchains = &vkdevice->GetGlobalSwapchain();
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &currentFrame.renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &imageIndex;

	VkResult draw_result = vkQueuePresentKHR(vkdevice->GetGraphicsQueue(), &presentInfo);

	if (draw_result == VK_ERROR_OUT_OF_DATE_KHR || draw_result == VK_SUBOPTIMAL_KHR /*|| Window::IsFrameBufferResized()*/) {
		// Window::SetFrameBufferResizeState(false);
		// RecreateSwapchain();
        return;
		
	} else if (draw_result != VK_SUCCESS) {
		common->FatalError("failed to present swap chain image!");
		abort();
	}
}

void idVulkanRBELocal::CleanUp() {
    testPipeline.DestroyPipeline();
    vertexBuffer.DestroyBuffer(vkdevice->GetGlobalMemoryAllocator());
}