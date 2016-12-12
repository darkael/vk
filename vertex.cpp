struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
    bool operator==(const Vertex& other) const {
};

static VkVertexInputBindingDescription Vertex::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}


static std::array<VkVertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
    description[0].binding = 0;
    description[0].location = 0;
    description[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    description[0].offset = offsetof(Vertex, pos);

    description[1].binding = 0;
    description[1].location = 1;
    description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    description[1].offset = offsetof(Vertex, color);

    description[2].binding = 0;
    description[2].location = 2;
    description[2].format = VK_FORMAT_R32G32_SFLOAT;
    description[2].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
}

bool Vertex::operator==(const Vertex& other) const {
    return pos == other.pos && color == other.color && texCoord == other.texCoord;
}
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                        (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ 
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}
