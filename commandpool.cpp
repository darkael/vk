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

CommandPool::CommandPool(std::shared_ptr<Device> deviceptr)
: deviceptr(deviceptr), device(*deviceptr.get())
{

}

void createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = device.findQueueFamilies();

    VkCommandPoolCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    auto res = vkCreateCommandPool(device, &info, nullptr, &pool);
    if (res != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
    }

}

