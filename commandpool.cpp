CommandPool::CommandPool(std::shared_ptr<Device> deviceptr)
: deviceptr(deviceptr), device(*deviceptr.get())
{
    QueueFamilyIndices queueFamilyIndices = device.findQueueFamilies();

    VkCommandPoolCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    auto res = vkCreateCommandPool(device, &info, nullptr, &pool);
    if (res != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
    }
}
