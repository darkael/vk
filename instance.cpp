#include "vulkan.h"

Instance::Instance(const std::string& name, GLFWwindow *window) {
    createInstance(name);
    setupDebugCallback()
    createSurface(window);
}

Instance::~Instance() {
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    vkDestroyInstance(_instance);
}

void Instance::createInstance(const std::string& name) {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = name.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;


    checkExtensions();
    if (enableValidationLayers) {
        checkValidationLayers();
    }

    auto extensions = getRequiredExtensions();
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = validationLayers.size();
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateInstance(&createInfo, nullptr, instance.replace()) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
    }

}

std::vector<const char*> Instance::getRequiredExtensions() {
    std::vector<const char*> extensions;

    unsigned int glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    for (unsigned int i = 0; i < glfwExtensionCount; i++) {
        extensions.push_back(glfwExtensions[i]);
    }

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    return extensions;
}

void Instance::checkExtensions() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::cout << extensionCount << "available extensions:" << std::endl;

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    for (const auto& extension : extensions) {
        std::cout << "\t" << extension.extensionName << std::endl;
    }
}

void Instance::checkValidationLayers() {
    uint32_t count;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::cout << count << " available layers:" << std::endl;

    std::vector<VkLayerProperties> layers(count);
    vkEnumerateInstanceLayerProperties(&count, layers.data());
    for (const auto& layer : layers) {
        std::cout << "\t" << layer.layerName << std::endl;
    }

    for (const char *layerName : validationLayers) {
        bool found = false;
        for (const auto& layer : layers) {
            if (strcmp(layerName, layer.layerName) == 0) {
                found = true;
                break;
            } 
        }
        if (!found) {
            std::cout << "Missing validation layer " << layerName << std::endl;
            throw std::runtime_error("validation layers requested, but not available!");
        }
    }
}

VkResult Instance::CreateDebugReportCallbackEXT(
        const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugReportCallbackEXT* pCallback)
{
    auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT");
    if (func != nullptr) {
        return func(_instance, pCreateInfo, pAllocator, pCallback);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void Instance::DestroyDebugReportCallbackEXT(
        VkDebugReportCallbackEXT callback,
        const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(_instance, "vkDestroyDebugReportCallbackEXT");
    if (func != nullptr) {
        func(_instance, callback, pAllocator);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL Instance::debugCallback(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t obj,
        size_t location,
        int32_t code,
        const char* layerPrefix,
        const char* msg,
        void* userData) {

    std::cerr << "validation layer: " << msg << std::endl;

    return VK_FALSE;
}

void Instance::setupDebugCallback() {
    if (enableValidationLayers) {
        VkDebugReportCallbackCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        createInfo.pfnCallback = debugCallback;

        auto res = CreateDebugReportCallbackEXT(&createInfo, nullptr, _reportCallback);
        if (res != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug callback!");
        }

    }
}

void Instance::createSurface(GLFWwindow *window) {
    auto res = glfwCreateWindowSurface(instance, window, nullptr, &_surface);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}
