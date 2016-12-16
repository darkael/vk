class Texture {
public:
    Texture(std::shared_ptr<Device> deviceptr);
    ~Texture();

private
    std::shared_ptr<Device> deviceptr;
    Image stagingImage;
    Image image;
    Device device;
};

Texture::Texture(std::shared_ptr<Device> deviceptr, const string& path)
: deviceptr(deviceptr), device(*deviceptr.get()),
  image(texWidth, texHeight, deviceptr
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
{
    int width;
    int height;
    int channels;

    stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &width, &height,
                                &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = width * height * 4;
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VDeleter<VkDeviceMemory> stagingImageMemory{device, vkFreeMemory};
    Image stagingImage(texWidth, texHeight, deviceptr,
                       VK_FORMAT_R8G8B8A8_UNORM,
                       VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingImage.loadPixels(width, height, pixels);

    stbi_image_free(pixels);

    transitionImageLayout(stagingImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyImage(stagingImage, textureImage, texWidth, texHeight);
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

}

