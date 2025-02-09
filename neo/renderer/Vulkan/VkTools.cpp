#include "VkTools.h"
#include "VkInit.h"
#include "framework/Common.h"
#include <vulkan/vulkan_core.h>


#define ID_VK_ERROR_STRING( x ) case static_cast< int >( x ): return #x

/*
=============
VK_ErrorToString
=============
*/
const char * VK_ErrorToString( VkResult result ) {
	switch ( result ) {
		ID_VK_ERROR_STRING( VK_SUCCESS );
		ID_VK_ERROR_STRING( VK_NOT_READY );
		ID_VK_ERROR_STRING( VK_TIMEOUT );
		ID_VK_ERROR_STRING( VK_EVENT_SET );
		ID_VK_ERROR_STRING( VK_EVENT_RESET );
		ID_VK_ERROR_STRING( VK_INCOMPLETE );
		ID_VK_ERROR_STRING( VK_ERROR_OUT_OF_HOST_MEMORY );
		ID_VK_ERROR_STRING( VK_ERROR_OUT_OF_DEVICE_MEMORY );
		ID_VK_ERROR_STRING( VK_ERROR_INITIALIZATION_FAILED );
		ID_VK_ERROR_STRING( VK_ERROR_DEVICE_LOST );
		ID_VK_ERROR_STRING( VK_ERROR_MEMORY_MAP_FAILED );
		ID_VK_ERROR_STRING( VK_ERROR_LAYER_NOT_PRESENT );
		ID_VK_ERROR_STRING( VK_ERROR_EXTENSION_NOT_PRESENT );
		ID_VK_ERROR_STRING( VK_ERROR_FEATURE_NOT_PRESENT );
		ID_VK_ERROR_STRING( VK_ERROR_INCOMPATIBLE_DRIVER );
		ID_VK_ERROR_STRING( VK_ERROR_TOO_MANY_OBJECTS );
		ID_VK_ERROR_STRING( VK_ERROR_FORMAT_NOT_SUPPORTED );
		ID_VK_ERROR_STRING( VK_ERROR_SURFACE_LOST_KHR );
		ID_VK_ERROR_STRING( VK_ERROR_NATIVE_WINDOW_IN_USE_KHR );
		ID_VK_ERROR_STRING( VK_SUBOPTIMAL_KHR );
		ID_VK_ERROR_STRING( VK_ERROR_OUT_OF_DATE_KHR );
		ID_VK_ERROR_STRING( VK_ERROR_INCOMPATIBLE_DISPLAY_KHR );
		ID_VK_ERROR_STRING( VK_ERROR_VALIDATION_FAILED_EXT );
		ID_VK_ERROR_STRING( VK_ERROR_INVALID_SHADER_NV );
		default: return "UNKNOWN";
	};
}

namespace idVkTools {

	void InsertImageMemoryBarrier( VkCommandBuffer& cmdbuffer, VkImage& image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange )
	{
		VkImageMemoryBarrier imageMemoryBarrier = ImageMemoryBarrier();
		imageMemoryBarrier.srcAccessMask = srcAccessMask;
		imageMemoryBarrier.dstAccessMask = dstAccessMask;
		imageMemoryBarrier.oldLayout = oldImageLayout;
		imageMemoryBarrier.newLayout = newImageLayout;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		vkCmdPipelineBarrier(
			cmdbuffer,
			srcStageMask,
			dstStageMask,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
	}

	VkImageMemoryBarrier ImageMemoryBarrier( void )
    {
        VkImageMemoryBarrier imageMemoryBarrier {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        return imageMemoryBarrier;
    }

	VkCommandPoolCreateInfo CommandPoolCreateInfo( uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags /*= 0*/ )
    {
        VkCommandPoolCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.pNext = nullptr;

        info.queueFamilyIndex = queueFamilyIndex;
        info.flags = flags;
        return info;
    }

	VkCommandBufferAllocateInfo CommandBufferAllocateInfo( VkCommandPool& pool, uint32_t count /*= 1*/, VkCommandBufferLevel level /*= VK_COMMAND_BUFFER_LEVEL_PRIMARY*/ )
    {
        VkCommandBufferAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.pNext = nullptr;

        info.commandPool = pool;
        info.commandBufferCount = count;
        info.level = level;
        return info;
    }

	VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags /*= 0*/)
    {
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.pNext = nullptr;

        info.pInheritanceInfo = nullptr;
        info.flags = flags;
        return info;
    }

	VkFenceCreateInfo FenceCreateInfo( VkFenceCreateFlags flags /*= 0*/ )
    {
        VkFenceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.flags = flags;
        info.pNext = nullptr;

        return info;
    }
	VkSemaphoreCreateInfo SemaphoreCreateInfo( void )
    {
        VkSemaphoreCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;

        return info;
    }

	VkSubmitInfo SubmitInfo( VkCommandBuffer* p_cmd )
    {
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.pNext = nullptr;

        info.waitSemaphoreCount = 0;
        info.pWaitSemaphores = nullptr;
        info.pWaitDstStageMask = nullptr;
        info.commandBufferCount = 1;
        info.pCommandBuffers = p_cmd;
        info.signalSemaphoreCount = 0;
        info.pSignalSemaphores = nullptr;

        return info;
    }

	VkPresentInfoKHR PresentInfo( void )
    {
        VkPresentInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        info.pNext = nullptr;

        info.swapchainCount = 0;
        info.pSwapchains = nullptr;
        info.pWaitSemaphores = nullptr;
        info.waitSemaphoreCount = 0;
        info.pImageIndices = nullptr;

        return info;
    }


    void AllocatedImage::DestroyImage( void ) {
        if (defaultView != VK_NULL_HANDLE) {
            vkDestroyImageView(vkdevice->GetGlobalDevice(), defaultView, nullptr);
        }
        
        if (image != VK_NULL_HANDLE) {
            vmaDestroyImage(vkdevice->GetGlobalMemoryAllocator(), image, allocation);
        }
    }


    void AllocatedBuffer::AllocateBuffer( const VkBufferUsageFlags& usage, VmaMemoryUsage memoryUsage,const uint32_t& dataSize, const uint32_t& typeSize  ) {
        if ( buffer == VK_NULL_HANDLE ){
            if(dataSize > 0) {
                //allocate vertex buffer
                VkBufferCreateInfo bufferInfo = {};
                bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                //this is the total size, in bytes, of the buffer we are allocating
                bufferSize = dataSize * typeSize;
                bufferInfo.size = bufferSize;
                //this buffer is going to be used as a Vertex Buffer
                bufferInfo.usage = usage;

                //let the VMA library know that this data should be writeable by CPU, but also readable by GPU
                VmaAllocationCreateInfo vmaallocInfo = {};
                vmaallocInfo.usage = memoryUsage;

                //allocate the buffer
                ID_VK_CHECK_RESULT(vmaCreateBuffer(vkdevice->GetGlobalMemoryAllocator(), &bufferInfo, &vmaallocInfo,
                    &buffer,
                    &allocation,
                    nullptr));
                
                if(usage == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT || usage == VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
                    descBuffInfo.buffer = buffer;
                    descBuffInfo.range = bufferSize;
                }
            }
            else{
                common->Warning("couldn't allocate a buffer due to data size = %d", dataSize);
            }
        }
    }

    void AllocatedBuffer::UploadBufferData( const void* pdata ) {
        if(buffer != VK_NULL_HANDLE) {
            //copy buffer data
            void* data;
            vmaMapMemory(vkdevice->GetGlobalMemoryAllocator(), allocation, &data);

            memcpy(data, pdata, bufferSize);

            vmaUnmapMemory(vkdevice->GetGlobalMemoryAllocator(), allocation);
        }
        else {
            common->Warning("cannot upload data to a buffer. Forgot to allocate it");
        }
    }

    void AllocatedBuffer::DestroyBuffer( void ) {
        if (buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(vkdevice->GetGlobalMemoryAllocator(), buffer, allocation);
            //just in case if we accidentaly try to delete this buffer twice
            buffer = VK_NULL_HANDLE;
        }
    }
}