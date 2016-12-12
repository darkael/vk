class DescriptorSet {
public:
    DescriptorSet();
    ~DescriptorSet();
    operator VkDescriptorSet() { return set };
    operator VkDescriptorSetLayout() { return layout };
    operator VkDescriptorPool() { return pool };

private:
    shared_ptr<Device> deviceptr;
    Device device;
    VkDescriptorSetLayout layout;
    VkDescriptorPool pool;
    VkDescriptorSet set;

    void createLayout();
    void createPool();
    void createSet();
};

DescriptorSet::DescriptorSet(shared_ptr<Device> deviceptr)
: deviceptr(deviceptr), device(*deviceptr.get())
{
    createLayout();
    createPool();
    createSet();
}

DescriptorSet::~DescriptorSet() {
    vkDestroyDescriptorPool(device, pool)
    vkDestroyDescriptorSetLayout(device, layout)
}

void DescriptorSet::createSet() {
    VkDescriptorSetLayout layouts[] = {layout};
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts;
    if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor set!");
    }

    std::array<VkWriteDescriptorSet, 2> writes = {};

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = descriptorSet;
    writes[0].dstBinding = 0;
    writes[0].dstArrayElement = 0;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[0].descriptorCount = 1;
    writes[0].pBufferInfo = &bufferInfo;

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = textureImageView;
    imageInfo.sampler = textureSampler;

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = descriptorSet;
    writes[1].dstBinding = 1;
    writes[1].dstArrayElement = 0;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].descriptorCount = 1;
    writes[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(device, writes.size(), writes.data(), 0, nullptr);
}

void DescriptorSet::createLayout() {
    VkDescriptorSetLayoutBinding uboBinding = {};
    uboBinding.binding = 0;
    uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBinding.descriptorCount = 1;
    uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding sampleBinding = {};
    sampleBinding.binding = 1;
    sampleBinding.descriptorCount = 1;
    sampleBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampleBinding.pImmutableSamplers = nullptr;
    sampleBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboBinding, sampleBinding};
    VkDescriptorSetLayoutCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = bindings.size();
    info.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &info, nullptr, &layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void DescriptorSet::createPool() {
    std::array<VkDescriptorPoolSize, 2> sizes = {};
    sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    sizes[0].descriptorCount = 1;
    sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sizes[1].descriptorCount = 1;

    VkDescriptorPoolCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.poolSizeCount = sizes.size();
    info.pPoolSizes = sizes.data();
    info.maxSets = 1;

    auto res = vkCreateDescriptorPool(device, &info, nullptr, &pool)
    if (res != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
    }
}
