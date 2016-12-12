#ifndef __VK_H_INCLUDED
#define __VK_H_INCLUDED

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

class Device() {
public:
    Device() {
        pickPhysicalDevice();
        createLogicalDevice();
    }

    ~Device() {
        vkDestroyDevice(device);
    }
    VkPhysicalDevice physical = VK_NULL_HANDLE;
    Vk logical = VK_NULL_HANDLE;

private:
    VkQueue _graphicsQueue;
    VkQueue _presentQueue;

    void pickPhysicalDevice();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    void createLogicalDevice();
};

#endif
