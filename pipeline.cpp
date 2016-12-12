class Pipeline {
public:
    Pipeline(std::shared_ptr<Device> deviceptr,
             const SwapChain& swapChain,
             const RenderPass& renderPass,
             const DescriptorSet& descriptorSet,
             const VertexShader& vertShader,
             const FragmentShader& fragShader);
    ~Pipeline();
    operator VkPipeline() { return set; }
    operator VkPipelineLayout() { return layout; }

private:
    std::shared_ptr<Device> deviceptr;
    Device device;
    VkPipeline pipeline;

    const VkPipelineLayout& layout;
    const VertexShader& vertexShader;
    const FragmentShader& fragmentShader;
};

Pipeline::Pipeline(std::shared_ptr<Device> deviceptr,
                   const SwapChain& swapChain
                   const RenderPass& renderPass,
                   const DescriptorSet& descriptorSet,
                   const VertexShader& vertShader,
                   const FragmentShader& fragShader);
: deviceptr(deviceptr), device(*deviceptr.get()), swapChain(swapChain),
  renderPass(renderPass), vertShader(vertShader), fragShader(fragShader),
{
    //const std::string vertShaderName("shaders/vert.spv");
    //const std::string fragShaderName("shaders/frag.spv");
    auto vertShaderStageInfo = vertShader.stageInfo();
    auto fragShaderStageInfo = fragShader.stageInfo();
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
    
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapChain.extent.width;
    viewport.height = (float) swapChain.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = swapChain.extent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkDescriptorSetLayout setLayouts[] = {descriptorSet.layout};

    VkPipelineLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = setLayouts;

    auto res = vkCreatePipelineLayout(device, &layoutInfo, nullptr, layout&);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;

    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkGraphicsPipelineCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    info.stageCount = 2;
    info.pStages = shaderStages;

    info.pVertexInputState = &vertexInputInfo;
    info.pInputAssemblyState = &inputAssembly;
    info.pViewportState = &viewportState;
    info.pRasterizationState = &rasterizer;
    info.pMultisampleState = &multisampling;
    info.pColorBlendState = &colorBlending;

    info.layout = layout;
    info.renderPass = renderPass;
    info.subpass = 0;
    info.basePipelineHandle = VK_NULL_HANDLE;

    info.pDepthStencilState = &depthStencil;

    auto res = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline);
    if (res != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
    }
}

Pipeline::~Pipeline() {
    vkDestroyPipeline(device, pipeline)
    vkDestroyPipelineLayout(device, layout)
}
