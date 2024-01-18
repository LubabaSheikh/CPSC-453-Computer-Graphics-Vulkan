#include <vulkan/vulkan.h>
#include <string>
#include "stb_image.h"
#include <array>
#include <vector>

VkImage createTextureImage(std::string imagePath, VkQueue vkQueue);
VkImageView createTextureImageView(VkImage textureImage);
VkSampler createTextureSampler();
void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
    VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkQueue vkQueue);
void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkQueue  vkQueue);
void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkQueue vkQueue);
VkCommandBuffer beginSingleTimeCommands();
void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue  vkQueue);
VkImageView createImageView(VkImage image, VkFormat format);
void cleanup();



VkDescriptorSetLayout createDescriptorSetLayout();
void createAODescriptorSetLayout();
std::array<VkDescriptorSetLayoutBinding,4> createBindings(VkDescriptorSetLayout descriptorSetLayout);
VkDescriptorPool createDescriptorPool(); //std::string type);
VkDescriptorSet createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkImageView textureImageView, 
    VkSampler textureSampler, VkImageView aotextureImageView, VkSampler aotextureSampler, 
     VkImageView pntextureImageView, VkSampler pntextureSampler, VkDescriptorPool descriptorPool);
VkDescriptorSet createAODescriptorSets();

void cleanUp();

VkImage createTextureImagefor2DGrid(const std::vector<std::vector<int>>& integerGrid, VkQueue vkQueue);
stbi_uc* flattenGridPixels(const std::vector<std::vector<int>>& integerGrid, int width, int height);
std::vector<int> flattenGrid(const std::vector<std::vector<int>>& grid);