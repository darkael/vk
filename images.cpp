#include <string>
#include <vector>
#include <GLFW/glfw3.h>
#include "vk.h"

Image::Image(uint32_t width, uint32_t height, std::shared_ptr<VkDevice> deviceptr,
             VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
             VkMemoryPropertyFlags properties)
: deviceptr(deviceptr), device(*deviceptr.get()), format(format)
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits,
                                                      properties);

    auto res = vkAllocateMemory(device, &allocInfo, nullptr, &memory);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }
    vkBindImageMemory(device, image, memory, 0);
}

void Image::loadPixels(int width, int height, void* pixels)
{
    void* data;
    VkDeviceSize imageSize = width * height * 4;

    vkMapMemory(device, stagingImageMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, (size_t) imageSize);
    vkUnmapMemory(device, stagingImageMemory);
}

operator Image::VkImage() {
    return image;
}

void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {


Image::~Image() {
    vkFreeMemory(*deviceptr.get(), memory);
    vkDestroyImage(image);
}

ImageView::ImageView(Image image, VkFormat format, VkImageAspectFlags aspectFlags,
      std::shared_ptr<Device> deviceptr)
: deviceptr(deviceptr), device(*deviceptr.get())
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image.cobj;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView);
        throw std::runtime_error("failed to create texture image view!");
    }
}

ImageView::~ImageView() {
    VkDestroyImageView(device, cobj, nullptr);
}
