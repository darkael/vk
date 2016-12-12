#include "vulkan.h"

const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

class Device() {
public:
    Device();
    ~Device();
    operator VkDevice();
    operator VkPhysicalDevice();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                                 VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat();

private:
    VkPhysicalDevice physical = VK_NULL_HANDLE;
    VkDevice logical = VK_NULL_HANDLE;
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    void pickPhysicalDevice();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    void createLogicalDevice();
};

Device::Device() {
    pickPhysicalDevice();
    createLogicalDevice();
}

Device::~Device() {
    vkDestroyDevice(logical);
}

operator Device::VkDevice() {
    return logical;
}

operator Device::VkPhysicalDevice() {
    return physical;
}

void Device::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    std::cout << "Found " << deviceCount << " physical devices" << std::endl;
    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {

            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);
            std::cout << "Using device " << properties.deviceName << std::endl;
            physical = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitable GPU!"); 
    }
}

QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device,
                                             &queueFamilyCount,
                                             nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device,
                                             &queueFamilyCount,
                                             queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device,
                                             i,
                                             surface,
                                             &presentSupport);
        if (
            queueFamily.queueCount > 0
            && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT
        ) {
            indices.graphicsFamily = i;
        }
        if (queueFamily.queueCount > 0 && presentSupport) {
                indices.presentFamily = i;
        }
        if (indices.isComplete()) {
            break;
        }
        i++;
    }
    return indices;
}

QueueFamilyIndices Device::findQueueFamilies() {
    return findQueueFamilies(physical);
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    auto count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(count);
    vkEnumerateDeviceExtensionProperties(device,
                                         nullptr,
                                         &count,
                                         availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                             deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();

}

bool Device::isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);


    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = (!swapChainSupport.formats.empty()
                             && !swapChainSupport.presentModes.empty());
    }
    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

void Device::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physical);

    float queuePriority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<int> uniqueQueueFamilies = {
        indices.graphicsFamily,
        indices.presentFamily
    };

    for (int queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceCreateInfo createInfo = {};

    VkPhysicalDeviceFeatures deviceFeatures = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = (uint32_t) queueCreateInfos.size();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = 0;
    createInfo.enabledExtensionCount = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = validationLayers.size();
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    auto res = vkCreateDevice(physicalDevice, &createInfo, nullptr, &logical);
    if (res != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
    }
    vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);
}


SwapChainSupportDetails Device::querySwapChainSupport() {
    SwapChainSupportDetails details; 

    vkGetPhysicalphysicalSurfaceCapabilitiesKHR(physical, surface, &details.capabilities);
    auto count;
    vkGetPhysicalphysicalSurfaceFormatsKHR(physical, surface, &count, nullptr);

    if (count != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalphysicalSurfaceFormatsKHR(physical, surface, &count,
                                               details.formats.data());
    }

    vkGetPhysicalphysicalSurfacePresentModesKHR(physical, surface, &count, nullptr);

    if (count != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalphysicalSurfacePresentModesKHR(physical, surface, &count,
                                                    details.presentModes.data());
    }
    return details;
}


uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (
            (typeFilter & (1 << i))
            && (memProperties.memoryTypes[i].propertyFlags & properties) == properties
        ) {
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
}

void Device::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport();
}

VkFormat Device::findSupportedFormat(const std::vector<VkFormat>& candidates,
                                     VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
            if (
                tiling == VK_IMAGE_TILING_LINEAR
                && (props.linearTilingFeatures & features) == features
            ){
                return format;
            } else if (
                tiling == VK_IMAGE_TILING_OPTIMAL
                && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat Device::findDepthFormat() {
    return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

