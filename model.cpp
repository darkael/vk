#include "vk.h"
#include "tiny_obj_loader.h"
#include <unordered_map>

Model::Model(Texture texture, const std::string& filename)
: texture(texture)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    auto res = tinyobj::LoadObj(&attrib, &shapes, &materials,
                                &err, filename.c_str());
    if (!res) {
        throw std::runtime_error(err);
    }

    std::unordered_map<Vertex, int> uniqueVertices = {};

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex = {};
            vertex.pos = {
                attrib.vertices[3 * index.vertex_index],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = {1.0f, 1.0f, 1.0f};

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = vertices.size();
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }

    }

    indexBuffer = Buffer(deviceptr,
                         commandPool,
                         indices.data(),
                         sizeof(indices[0]) * indices.size(),
                         VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    vertexBuffer = Buffer(deviceptr,
                          commandPool,
                          vertices.data(),
                          sizeof(vertices[0]) * vertices.size(),
                          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
}
