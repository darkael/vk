class Shader {
public:
    Shader(shared_ptr<Device> deviceptr, const std::string& filename,
           VkShaderStageFlagBits stageFlags);
    ~Shader();
    operator VkShaderModule() { return module; }

private:
    shared_ptr<Device> deviceptr;
    Device device;
    VkShaderModule module;
    VkShaderStageFlagBits stageFlags;
    VkPipeLineShaderStageCreateInfo stageInfo();
};

class VertexShader: Shader {
public:
    VertexShader(shared_ptr<Device> deviceptr, const std::string& filename)
    : Shader(deviceptr, filename, VK_SHADER_STAGE_VERTEX_BIT) {}
};

class FragmentShader: Shader {
public:
    VertexShader(shared_ptr<Device> deviceptr, const std::string& filename)
    : Shader(deviceptr, filename, VK_SHADER_STAGE_FRAGMENT_BIT) {}
};

Shader::Shader(shared_ptr<Device> deviceptr, const std::string& filename,
               VkShaderStageFlagBits stageFlags)
: deviceptr(deviceptr), device(*deviceptr.get()), stageFlags(stageFlags)
{
    auto code = readFile(filename);
    VkShaderModuleCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = code.size();
    info.pCode = (uint32_t*) code.data();

    auto res = vkCreateShaderModule(device, &info, nullptr, &module);
    if (res != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
    }
}

Shader::~Shader() {
    vkDestroyShaderModule(device, module);
}

static std::vector<char> Shader::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

VkPipeLineShaderStageCreateInfo Shader::stageInfo() {
    VkPipelineShaderStageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.stage = stageInfo;
    info.module = module;
    info.pName = "main";
    return info;
}
