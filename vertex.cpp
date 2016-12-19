#include <vk.h>

VkVertexInputBindingDescription Vertex::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}


std::array<VkVertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 3> description = {};
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

    return description;
}

bool Vertex::operator==(const Vertex& other) const {
    return pos == other.pos && color == other.color && texCoord == other.texCoord;
}
