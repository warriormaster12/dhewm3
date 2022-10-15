#pragma once 

#include <cstdint>
#define VK_NO_PROTOTYPES
#include "renderer/Vulkan/third-party/volk/volk.h"

#include "renderer/Vulkan/third-party/vma/vk_mem_alloc.h"
#include <assert.h>

#include "framework/Common.h"

#include <vector>




namespace idVkTools {

	struct AllocatedImage {
		VkImage image = VK_NULL_HANDLE;
		VmaAllocation allocation;
		VkImageView defaultView = VK_NULL_HANDLE;
        VkFormat imageFormat;
		int mipLevels;

		void DestroyImage( void );
	};

	class AllocatedBuffer {
	private:
		VmaAllocation allocation;
		uint32_t bufferSize;

	public:
		VkBuffer buffer = VK_NULL_HANDLE;
		void AllocateBuffer( const VkBufferUsageFlags& usage, const uint32_t& dataSize, const uint32_t& typeSize );

		void UploadBufferData( const void* pdata );

		void DestroyBuffer( void );
	};

	void InsertImageMemoryBarrier( VkCommandBuffer& cmdbuffer, VkImage& image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange );

	VkImageMemoryBarrier ImageMemoryBarrier( void );

	VkCommandPoolCreateInfo CommandPoolCreateInfo( uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0 );

	VkCommandBufferAllocateInfo CommandBufferAllocateInfo( VkCommandPool& pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY );

	VkCommandBufferBeginInfo CommandBufferBeginInfo( VkCommandBufferUsageFlags flags = 0 );

	VkFenceCreateInfo FenceCreateInfo( VkFenceCreateFlags flags = 0 );

	VkSemaphoreCreateInfo SemaphoreCreateInfo( void );

	VkSubmitInfo SubmitInfo( VkCommandBuffer* p_cmd );

	VkPresentInfoKHR PresentInfo( void );
	
}

#define ID_VK_CHECK_RESULT(f)																				\
{																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS)																				\
	{																									\
		common->FatalError("Fatal : VkResult is %s in %s at line %d",VK_ErrorToString( res ), __FILE__, __LINE__);                                  \
		assert(res == VK_SUCCESS);																		\
	}																									\
}

const char * VK_ErrorToString( VkResult result );




