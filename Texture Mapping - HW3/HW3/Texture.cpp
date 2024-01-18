#define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>
#include "Texture.h"
#include <vector>
#include "VulkanLaunchpad.h"

extern int aoConfig;
extern int pnConfig;
extern int texConfig;

extern VkPhysicalDevice vk_physical_device;
const int MAX_FRAMES_IN_FLIGHT = 1;

VkCommandPool commandPool;
VkCommandBuffer commandBuffer;

std::vector<VkWriteDescriptorSet> descriptorWrite(4);
std::array<VkDescriptorSetLayoutBinding, 4> bindings;

VkDescriptorSetLayoutBinding uboLayoutBinding{};
VkDescriptorSetLayoutBinding samplerLayoutBinding{};
VkDescriptorSetLayoutBinding aosamplerLayoutBinding{};
VkDescriptorSetLayoutBinding pnsamplerLayoutBinding{};

// Uniform Buffer
VkBuffer uniformBuffers;
VkDeviceMemory uniformBuffersMemory;
std::vector<void*> uniformBuffersMapped;

struct UniformBufferObject {
	int aoState = 0;
	int pnState = 0;
	int texState = 0;
} uniformBufferStruct;

// UniformBufferObject uniformBuff{};

void initVulkan() {
}

VkImage createTextureImage(std::string imagePath, VkQueue vkQueue) {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(imagePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    commandPool = vklGetCommandPool();

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    stagingBuffer = vklCreateHostCoherentBufferWithBackingMemory(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vklCopyDataIntoHostCoherentBuffer(stagingBuffer, pixels, static_cast<size_t>(imageSize));

    // void* data;
    // vkMapMemory(vklGetDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
    //     memcpy(data, pixels, static_cast<size_t>(imageSize));
    // vkUnmapMemory(vklGetDevice(), stagingBufferMemory);

    stbi_image_free(pixels); // free pixels
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

    commandBuffer = beginSingleTimeCommands();
    // begin single time commands
    // 
    // then do first barrier
    // 

    // BARRIERS HERE
    auto oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    auto newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = textureImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0; // TODO
    barrier.dstAccessMask = 0; // TODO
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_HOST_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } 
    
    else 
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    VKL_LOG(":::::: HERE 3 ::::::");
    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    /// AFTER BARRIER
    // transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, 
        //VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vkQueue);
    copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), 
        static_cast<uint32_t>(texHeight), vkQueue);
    // transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, vkQueue);
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; // these are what you pass to transitionImageLayout
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    vkCmdPipelineBarrier( // giving me errors
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    
    endSingleTimeCommands(commandBuffer, vkQueue);

    VKL_LOG(":::::: HERE 6 ::::::");
    // CLEANUP
    // vkDestroyBuffer(vklGetDevice(), stagingBuffer, nullptr);
    // vkFreeMemory(vklGetDevice(), stagingBufferMemory, nullptr);
    VKL_LOG(":::::: HERE 7 ::::::");
    vklDestroyHostCoherentBufferAndItsBackingMemory(stagingBuffer);

    return textureImage;

}


void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
    VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(vklGetDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vklGetDevice(), image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(vklGetDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(vklGetDevice(), image, imageMemory, 0);
}


uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) 
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vk_physical_device, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

// Copying buffer to image
void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkQueue vkQueue) {
    VKL_LOG(":::::: HERE 9 ::::::");
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };
    // endSingleTimeCommands(commandBuffer, vkQueue);
    vkCmdCopyBufferToImage( // giving me errors
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );
}



// ================================
// Layout Transition
// ================================
// void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkQueue vkQueue) {
//     VkCommandBuffer commandBuffer = beginSingleTimeCommands();

//     VkBufferCopy copyRegion{};
//     copyRegion.size = size;
//     vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

//     endSingleTimeCommands(commandBuffer, vkQueue);
// }

void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkQueue vkQueue) 
{
    commandBuffer = beginSingleTimeCommands();
    endSingleTimeCommands(commandBuffer, vkQueue);
}

VkCommandBuffer beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(vklGetDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue vkQueue) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(vkQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vkQueue);

    vkFreeCommandBuffers(vklGetDevice(), commandPool, 1, &commandBuffer);
}


/// Clean up



// TEXTURE IMAGE VIEW 

VkImageView createTextureImageView(VkImage textureImage) {
    VkImageView textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
    return textureImageView;
}




VkImageView createImageView(VkImage image, VkFormat format) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    VkImageView imageView;
    if (vkCreateImageView(vklGetDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
    return imageView;
}

// void createImageViews() { // DO I need to do swap chain stuff??
//     swapChainImageViews.resize(swapChainImages.size());
//     for (uint32_t i = 0; i < swapChainImages.size(); i++) {
//         swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat);
//     }
// }



// // Image view and sampler code:

VkSampler createTextureSampler() {

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(vk_physical_device, &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;

    VkSampler textureSampler;
    if (vkCreateSampler(vklGetDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    return textureSampler;
}

// bool isDeviceSuitable(VkPhysicalDevice device) { What is this?
//     VkPhysicalDeviceFeatures supportedFeatures;
//     vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
//     return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
// }


// // =============================================
// // Uniform Buffers: Descriptor layout and buffer
// // =============================================
VkDescriptorSetLayout createDescriptorSetLayout() {
    //VkDescriptorSetLayoutBinding uboLayoutBinding{}; // pass this into config
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

   //  VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    aosamplerLayoutBinding.binding = 2;
    aosamplerLayoutBinding.descriptorCount = 1;
    aosamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    aosamplerLayoutBinding.pImmutableSamplers = nullptr;
    aosamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    pnsamplerLayoutBinding.binding = 3;
    pnsamplerLayoutBinding.descriptorCount = 1;
    pnsamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pnsamplerLayoutBinding.pImmutableSamplers = nullptr;
    pnsamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    // ao.binding = 4;
    // uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // uboLayoutBinding.descriptorCount = 1;
    // uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    // uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    bindings = {uboLayoutBinding, samplerLayoutBinding, aosamplerLayoutBinding, pnsamplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    // VkPipelineLayout pipelineLayout;
    // VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    // pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // pipelineLayoutInfo.setLayoutCount = 1;
    // pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    VkDescriptorSetLayout descriptorSetLayout;
    if (vkCreateDescriptorSetLayout(vklGetDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    return descriptorSetLayout;
}

void createAODescriptorSetLayout() {
    aosamplerLayoutBinding.binding = 2;
    aosamplerLayoutBinding.descriptorCount = 1;
    aosamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    aosamplerLayoutBinding.pImmutableSamplers = nullptr;
    aosamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
}

std::array<VkDescriptorSetLayoutBinding,4> createBindings(VkDescriptorSetLayout descriptorSetLayout)
{
    // std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding}; //, aosamplerLayoutBinding};
    // VkDescriptorSetLayoutCreateInfo layoutInfo{};
    // layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    // layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    // layoutInfo.pBindings = bindings.data();

    // if (vkCreateDescriptorSetLayout(vklGetDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) 
    // {
    //     throw std::runtime_error("failed to create descriptor set layout!");
    // }

    return bindings;
}

// void creatUniformBuffer(){
//     uniformBufferStruct.aoState = aoConfig;
//     uniformBufferStruct.pnState = pnConfig;
//     uniformBufferStruct.texState = texConfig;

//     uniformBuffers = vklCreateHostCoherentBufferAndUploadData(
// 			&uniformBufferStruct, sizeof(uniformBufferStruct),
// 			VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
// }




// void updateUniformBuffer()
// {
//     uniformBufferStruct.aoState = aoConfig;
//     uniformBufferStruct.pnState = pnConfig;
//     uniformBufferStruct.texState = texConfig;
//     vklCopyDataIntoHostCoherentBuffer(uniformBuffers, &uniformBufferStruct, sizeof(uniformBufferStruct));
// }

// // =============================================
// // Uniform Buffers: Descriptor pool and sets
// // =============================================

VkDescriptorPool createDescriptorPool() //std::string type) {
{
    std::array<VkDescriptorPoolSize, 4> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[3].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPool descriptorPool;
    if (vkCreateDescriptorPool(vklGetDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    return descriptorPool;
    
}

VkDescriptorSet createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkImageView textureImageView, 
    VkSampler textureSampler, VkImageView aotextureImageView, VkSampler aotextureSampler, 
    VkImageView pntextureImageView, VkSampler pntextureSampler, VkDescriptorPool descriptorPool) 
{
    // std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = &descriptorSetLayout;

    VkDescriptorSet descriptorSets;
    // descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(vklGetDevice(), &allocInfo, &descriptorSets) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
    // need this later but without forloop
    
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformBuffers;
    bufferInfo.offset = 0;
    bufferInfo.range = 19UL;

    // THIS IS DOING UNIFORM BUFFER
    descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite[0].dstSet = descriptorSets;
    descriptorWrite[0].dstBinding = 0;
    descriptorWrite[0].dstArrayElement = 0;
    descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite[0].descriptorCount = 1;
    descriptorWrite[0].pBufferInfo = &bufferInfo;
    descriptorWrite[0].pImageInfo = nullptr; // Optional
    descriptorWrite[0].pTexelBufferView = nullptr; // Optional

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = textureImageView;
    imageInfo.sampler = textureSampler;
    descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite[1].dstSet = descriptorSets;
    descriptorWrite[1].dstBinding = 1;
    descriptorWrite[1].dstArrayElement = 0;
    descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite[1].descriptorCount = 1;
    descriptorWrite[1].pTexelBufferView = nullptr; // Optional
    descriptorWrite[1].pImageInfo = &imageInfo;

    VkDescriptorImageInfo aoimageInfo{};
    aoimageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    aoimageInfo.imageView = aotextureImageView;
    aoimageInfo.sampler = aotextureSampler;
    descriptorWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite[2].dstSet = descriptorSets;
    descriptorWrite[2].dstBinding = 2;
    descriptorWrite[2].dstArrayElement = 0;
    descriptorWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite[2].descriptorCount = 1;
    descriptorWrite[2].pTexelBufferView = nullptr; // Optional
    descriptorWrite[2].pImageInfo = &aoimageInfo;

    VkDescriptorImageInfo pnimageInfo{};
    pnimageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    pnimageInfo.imageView = pntextureImageView;
    pnimageInfo.sampler = pntextureSampler;
    descriptorWrite[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite[3].dstSet = descriptorSets;
    descriptorWrite[3].dstBinding = 3;
    descriptorWrite[3].dstArrayElement = 0;
    descriptorWrite[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite[3].descriptorCount = 1;
    descriptorWrite[3].pTexelBufferView = nullptr; // Optional
    descriptorWrite[3].pImageInfo = &pnimageInfo;

    vkUpdateDescriptorSets(vklGetDevice(), descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
    return descriptorSets; 
}





VkImage createTextureImagefor2DGrid(const std::vector<std::vector<int>>& integerGrid, VkQueue vkQueue) {
    // int texWidth, texHeight, texChannels;
    // stbi_uc* pixels = stbi_load(imagePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    // VkDeviceSize imageSize = texWidth * texHeight * 4;

    // Image properties
    const uint32_t width = static_cast<uint32_t>(integerGrid.size());
    const uint32_t height = static_cast<uint32_t>(integerGrid[0].size());
    // Convert the 2D integer grid to stbi_uc pixels
    std::vector<int> pixels = flattenGrid(integerGrid);
    VkDeviceSize imageSize = width * height * 4;

    commandPool = vklGetCommandPool();

    // Create a staging buffer to hold the image data
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    stagingBuffer = vklCreateHostCoherentBufferWithBackingMemory(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vklCopyDataIntoHostCoherentBuffer(stagingBuffer, &pixels, static_cast<size_t>(imageSize));

    // void* data;
    // vkMapMemory(vklGetDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
    // memcpy(data, &pixels, static_cast<size_t>(imageSize));
    // vkUnmapMemory(vklGetDevice(), stagingBufferMemory);

    // stbi_image_free(pixels); // free pixels
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    createImage(width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

    commandBuffer = beginSingleTimeCommands();
    // begin single time commands
    // 
    // then do first barrier
    // 

    // BARRIERS HERE
    auto oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    auto newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = textureImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0; // TODO
    barrier.dstAccessMask = 0; // TODO
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_HOST_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } 
    
    else 
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    VKL_LOG(":::::: HERE 3 ::::::");
    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    /// AFTER BARRIER
    // transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, 
        //VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vkQueue);
    copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(width), 
        static_cast<uint32_t>(height), vkQueue);
    // transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, vkQueue);
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; // these are what you pass to transitionImageLayout
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    vkCmdPipelineBarrier( // giving me errors
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    
    endSingleTimeCommands(commandBuffer, vkQueue);

    // CLEANUP
    // vkDestroyBuffer(vklGetDevice(), stagingBuffer, nullptr);
    // vkFreeMemory(vklGetDevice(), stagingBufferMemory, nullptr);
    vklDestroyHostCoherentBufferAndItsBackingMemory(stagingBuffer);

    return textureImage;

}

std::vector<int> flattenGrid(const std::vector<std::vector<int>>& grid) {
    std::vector<int> flattenedPixels;

    for (const auto& row : grid) {
        flattenedPixels.insert(flattenedPixels.end(), row.begin(), row.end());
    }

    return flattenedPixels;
}