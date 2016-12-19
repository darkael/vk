#ifndef __VK_H_INCLUDED
#define __VK_H_INCLUDED

#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

struct QueueFamilyIndices {
    int graphicsFamily = -1;
    int presentFamily = -1;

    bool isComplete() {
        return graphicsFamily >= 0 && presentFamily >= 0;
    }
};

class Instance {
public:
    Instance(const std::string& name);
    ~Instance();
private:
    VkInstance _instance;
    VkDebugReportCallbackEXT _reportCallback;
    VkSurfaceKHR surface;

    void createInstance(const std::string& name);
    std::vector<const char*> getRequiredExtensions();
    void checkExtensions();
    void checkValidationLayers();
    VkResult CreateDebugReportCallbackEXT(
            const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator,
            VkDebugReportCallbackEXT* pCallback) ;
    void DestroyDebugReportCallbackEXT(
            VkDebugReportCallbackEXT callback,
            const VkAllocationCallbacks* pAllocator) ;
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugReportFlagsEXT flags,
            VkDebugReportObjectTypeEXT objType,
            uint64_t obj,
            size_t location,
            int32_t code,
            const char* layerPrefix,
            const char* msg,
            void* userData);
    void setupDebugCallback();
    void createSurface(GLFWwindow *window);
};

struct SwapChainSupport;

class Device {
public:
    Device(VkInstance instance, VkSurfaceKHR surface);
    ~Device();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                                 VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat();
    operator VkDevice() { return logical; }
    operator VkPhysicalDevice() { return physical; }

private:
    VkPhysicalDevice physical = VK_NULL_HANDLE;
    VkDevice logical = VK_NULL_HANDLE;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;
    VkInstance instance;

    void pickPhysicalDevice();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies();
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    void createLogicalDevice();
    void queueSubmit(VkSubmitInfo *submitInfo);
	SwapChainSupport querySwapChainSupport();
};

struct SwapChainSupport{
    SwapChainSupport(std::shared_ptr<Device> deviceptr, VkSurfaceKHR surface);
    VkSurfaceFormatKHR chooseSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& availableFormats);

    std::shared_ptr<Device> deviceptr;
    Device device;
    VkSurfaceCapabilitiesKHR capabilities;

    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
};

class SwapChain {
public:
    SwapChain();
    ~SwapChain();

    VkExtent2D extent;

private:
    std::shared_ptr<Device> deviceptr;
    Device device;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageviews;
    VkFormat imageformat;
    std::vector<VkFramebuffer> framebuffers;
    SwapChainSupport support;
};

class CommandPool {
public:
    CommandPool(std::shared_ptr<Device> deviceptr);
    ~CommandPool();
    operator VkCommandPool() { return pool; }

private:
    std::shared_ptr<Device> deviceptr;
    Device device;
    VkCommandPool pool;
};

class Image {
public:
    Image(uint32_t width, uint32_t height, std::shared_ptr<VkDevice> deviceptr,
          VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
          VkMemoryPropertyFlags properties);
    operator VkImage();
    ~Image();

private:
    VkImage image;
    VkDeviceMemory memory;
    VkFormat format;
    std::shared_ptr<Device> deviceptr;
    Device device;

};

class ImageView {
public:
    ImageView(Image image, VkFormat format, VkImageAspectFlags aspectFlags,
          std::shared_ptr<Device> deviceptr);
    ~ImageView();
private:
    VkImageView imageView;;
    std::shared_ptr<Device> deviceptr;
    Device device;
};

class CommandBuffer {
public:
    CommandBuffer(std::shared_ptr<Device> deviceptr, CommandPool commandPool);
    ~CommandBuffer();
    operator VkCommandBuffer() { return commandBuffer; }
    void submit();

private:
	std::shared_ptr<Device> deviceptr;
	Device device;
	CommandPool commandPool;
    VkCommandBuffer commandBuffer;
    virtual void execute() = 0;
    void beginSingleTimeCommands();
    void endSingleTimeCommands();
};

class ImageTransitionCmdBuffer : public CommandBuffer {
public:
    ImageTransitionCmdBuffer(std::shared_ptr<Device> deviceptr,
                             CommandPool commandPool,
                             Image image,
                             VkImageLayout oldLayout,
                             VkImageLayout newLayout);
    void execute();

private:
    VkImageLayout oldLayout;
    VkImageLayout newLayout;
    Image image;
};

class Texture {
public:
    Texture(std::shared_ptr<Device> deviceptr, CommandPool commandPool);
    ~Texture();

private:
    std::shared_ptr<Device> deviceptr;
    Image stagingImage;
    Image image;
    Device device;

    VkSampler sampler;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    VkVertexInputBindingDescription getBindingDescription();
    std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
    bool operator==(const Vertex& other) const;
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                        (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ 
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
};

class Model {
public:
    Model(Texture texture, const std::string& filename);
    ~Model();
	void createIndexBuffer();
	void createVertexBuffer();

private:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    Texture texture;
};

#endif
