class SwapChain {
public:
    SwapChain();
    ~SwapChain();

    VkExtent2D extent;

private:
    std::shared_ptr<Device> deviceptr;
    Device device;
    VkSwapChainKHR swapChain;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageviews;
    VkFormat imageformat;
    std::vector<VkFramebuffer> framebuffers;
};

SwapChain::SwapChain(std::shared_ptr<Device> deviceptr, SwapChainSupport support)
: deviceptr(deviceptr), device(*deviceptr.get())
{
    create(support);
    createImageViews();
    createFramebuffers();
}

SwapChain::~SwapChain() {
    for (const auto& framebuffer: framebuffers) {
        vkDestroyFramebuffer(device, framebuffer);
    }
    for (const auto& imageview: imageviews) {
        vkDestroyImageView(device, imageview);
    }
    vkDestroySwapchainKHR(device, swapChain);
}

struct SwapChainSupport{
    SwapChainSupport(std::shared_ptr<Device> deviceptr, VkSurfaceKHR surface);
    VkSurfaceFormatKHR chooseSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& availableFormats);

    std::shared_ptr<Device> deviceptr;
    Device device;
    VkSurfaceCapabilitiesKHR capabilities;

    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
}

SwapChainSupport::SwapChainSupport(std::shared_ptr<Device> deviceptr, VkSurfaceKHR surface)
: deviceptr(deviceptr), device(*deviceptr.get())
{
    uint32_t count{0};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);
    
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);
    if (count != 0) {
        formats.resize(count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats.data());
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);

    if (count != 0) {
        presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, presentModes.data());
    }
    surfaceFormat = chooseSwapSurfaceFormat(formats);
    presentMode = chooseSwapPresentMode(presentModes);
}

VkSurfaceFormatKHR SwapChainSupport::chooseSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats
) {
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
            return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }
    for (const auto& availableFormat : availableFormats) {
        if (
            availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM
            && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        ) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR SwapChainSupport::choosePresentMode(
    const std::vector<VkPresentModeKHR> availableModes
) {
    for (const auto& availablePresentMode : availableModes) {
        //if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
        if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChainSupport::chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& capabilities
) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = {WIDTH, HEIGHT};

        actualExtent.width = std::max(capabilities.minImageExtent.width, 
                                      std::min(capabilities.maxImageExtent.width,
                                               actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, 
                                       std::min(capabilities.maxImageExtent.height,
                                                actualExtent.height));
        return actualExtent;
    }
}


void SwapChain::create(SwapChainSupport support) {
    uint32_t imageCount = support.capabilities.minImageCount + 1;
    if (
        support.capabilities.maxImageCount > 0
        && imageCount > support.capabilities.maxImageCount
    ) {
        imageCount = support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = support.surfaceFormat.format;
    createInfo.imageColorSpace = support.surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = device.findQueueFamilies();
    uint32_t queueFamilyIndices[] = {
        (uint32_t) indices.graphicsFamily,
        (uint32_t) indices.presentFamily
    };

    if (indices.graphicsFamily != indices.presentFamily) {
        std::cout << "Graphics and presentation queue families different" << std::endl;
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        std::cout << "Graphics and presentation queue families are the same" << std::endl;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    createInfo.preTransform = support.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR oldSwapChain = swapChain;
    createInfo.oldSwapchain = oldSwapChain;

    VkSwapchainKHR newSwapChain;
    auto res = vkCreateSwapchainKHR(device, &createInfo,
                                    nullptr, &newSwapChain);
    if (res != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
    }

    swapChain = newSwapChain;

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    images.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, images.data());

    imageformat = support.surfaceFormat.format;
}

void SwapChain::createImageViews() {
    imageviews.resize(images.size()); 

    for (uint32_t i = 0; i < swapChainImages.size(); i++) {
        imageviews[i] = ImageView(images[i], imageformat,
                                  VK_IMAGE_ASPECT_COLOR_BIT, deviceptr);
    }
}

void createFramebuffers() {
    framebuffers.resize(imageviews.size());
    for (size_t i = 0; i < imageviews.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            imageviews[i],
            depthImageView
        };

        VkFramebufferCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass = renderPass;
        info.attachmentCount = attachments.size();
        info.pAttachments = attachments.data();
        info.width = swapChainExtent.width;
        info.height = swapChainExtent.height;
        info.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, swapChainFramebuffers[i].replace()) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

