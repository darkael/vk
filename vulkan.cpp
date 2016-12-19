#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <chrono>
#include <set>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <vector>
#include <cstring>
#include <limits>
#include <fstream>
#include <array>
#include <ctime>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include "vdeleter.h"
#include "stb_image.h"
#include "tiny_obj_loader.h"

const int WIDTH = 800;
const int HEIGHT = 600;

const std::string MODEL_PATH = "model.obj";
const std::string TEXTURE_PATH = "model.jpg";

const std::vector<const char*> validationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
};
const bool enableValidationLayers = true;

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class HelloTriangleApplication {
    public:
        void run() {
            last_time = std::chrono::steady_clock::now();
            initWindow();
            initVulkan();
            mainLoop();
        }

    private:
        GLFWwindow *window;
        std::chrono::time_point<std::chrono::steady_clock> last_time;
        double frames{0};
        std::vector<VkCommandBuffer> commandBuffers;
        VDeleter<VkSemaphore> imageAvailableSemaphore{device, vkDestroySemaphore};
        VDeleter<VkSemaphore> renderFinishedSemaphore{device, vkDestroySemaphore};
        VDeleter<VkBuffer> vertexBuffer{device, vkDestroyBuffer};
        VDeleter<VkDeviceMemory> vertexBufferMemory{device, vkFreeMemory};
        VDeleter<VkBuffer> indexBuffer{device, vkDestroyBuffer};
        VDeleter<VkDeviceMemory> indexBufferMemory{device, vkFreeMemory};

        VDeleter<VkBuffer> uniformStagingBuffer{device, vkDestroyBuffer};
        VDeleter<VkDeviceMemory> uniformStagingBufferMemory{device, vkFreeMemory};
        VDeleter<VkBuffer> uniformBuffer{device, vkDestroyBuffer};
        VDeleter<VkDeviceMemory> uniformBufferMemory{device, vkFreeMemory};

        VDeleter<VkImage> stagingImage{device, vkDestroyImage};
        VDeleter<VkDeviceMemory> stagingImageMemory{device, vkFreeMemory};
        VDeleter<VkImage> textureImage{device, vkDestroyImage};
        VDeleter<VkDeviceMemory> textureImageMemory{device, vkFreeMemory};
        VDeleter<VkImageView> textureImageView{device, vkDestroyImageView};

        VDeleter<VkSampler> textureSampler{device, vkDestroySampler};

        VDeleter<VkImage> depthImage{device, vkDestroyImage};
        VDeleter<VkDeviceMemory> depthImageMemory{device, vkFreeMemory};
        VDeleter<VkImageView> depthImageView{device, vkDestroyImageView};

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        void createSurface() {
            if (glfwCreateWindowSurface(instance, window, nullptr, surface.replace()) != VK_SUCCESS) {
                throw std::runtime_error("failed to create window surface!");
            }

        }

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }

            throw std::runtime_error("failed to find suitable memory type!");
        }

        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VDeleter<VkBuffer>& buffer, VDeleter<VkDeviceMemory>& bufferMemory) {
            VkBufferCreateInfo bufferInfo = {};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = size;
            bufferInfo.usage = usage;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateBuffer(device, &bufferInfo, nullptr, buffer.replace()) != VK_SUCCESS) {
                throw std::runtime_error("failed to create vertex buffer!");
            }

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

            VkMemoryAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

            if (vkAllocateMemory(device, &allocInfo, nullptr, bufferMemory.replace()) != VK_SUCCESS) {
                    throw std::runtime_error("failed to allocate vertex buffer memory!");
            }

            vkBindBufferMemory(device, buffer, bufferMemory, 0);
        }

        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();

            VkBufferCopy copyRegion = {};
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

            endSingleTimeCommands(commandBuffer);
        }


        void copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height) {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();

            VkImageSubresourceLayers subResource = {};
            subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subResource.baseArrayLayer = 0;
            subResource.mipLevel = 0;
            subResource.layerCount = 1;

            VkImageCopy region = {};
            region.srcSubresource = subResource;
            region.dstSubresource = subResource;
            region.srcOffset = {0, 0, 0};
            region.dstOffset = {0, 0, 0};
            region.extent.width = width;
            region.extent.height = height;
            region.extent.depth = 1;
            vkCmdCopyImage(
                commandBuffer,
                srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &region
            );


            endSingleTimeCommands(commandBuffer);
        }

        void createCommandBuffers() {
            if (commandBuffers.size() > 0) {
                    vkFreeCommandBuffers(device, commandPool, commandBuffers.size(), commandBuffers.data());
            }
            commandBuffers.resize(swapChainFramebuffers.size());
            VkCommandBufferAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = commandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();



            if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
                    throw std::runtime_error("failed to allocate command buffers!");
            }

            for (size_t i = 0; i < commandBuffers.size(); i++) {
                VkCommandBufferBeginInfo beginInfo = {};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
                beginInfo.pInheritanceInfo = nullptr; // Optional

                vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

                VkRenderPassBeginInfo renderPassInfo = {};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass = renderPass;
                renderPassInfo.framebuffer = swapChainFramebuffers[i];

                renderPassInfo.renderArea.offset = {0, 0};
                renderPassInfo.renderArea.extent = swapChainExtent;

                std::array<VkClearValue, 2> clearValues = {};
                clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
                clearValues[1].depthStencil = {1.0f, 0};

                renderPassInfo.clearValueCount = clearValues.size();
                renderPassInfo.pClearValues = clearValues.data();

                vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
                vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

                VkBuffer vertexBuffers[] = {vertexBuffer};
                VkDeviceSize offsets[] = {0};
                vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

                vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
                vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);


                vkCmdDrawIndexed(commandBuffers[i], indices.size(), 1, 0, 0, 0);
                vkCmdEndRenderPass(commandBuffers[i]);
                if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to record command buffer!");
                }
            }

        }

        void createUniformBuffer() {
            VkDeviceSize bufferSize = sizeof(UniformBufferObject);

            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformStagingBuffer, uniformStagingBufferMemory);
            createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uniformBuffer, uniformBufferMemory);
        }

        bool hasStencilComponent(VkFormat format) {
            return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
        }

        void createDepthResources() {
            VkFormat depthFormat = findDepthFormat();

            createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);

            createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, depthImageView);

            transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

        }

        void initVulkan() {
            //createDepthResources();

            createUniformBuffer();
            createCommandBuffers();
            createSemaphores();
        }

        void recreateSwapChain() {
            vkDeviceWaitIdle(device);

            createSwapChain();
            createImageViews();
            createRenderPass();
            createGraphicsPipeline();
            createDepthResources();
            createFramebuffers();
            createCommandBuffers();
        }

        void drawFrame() {
            uint32_t imageIndex;
            VkResult result = vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

            if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                    recreateSwapChain();
                        return;
            } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
                    throw std::runtime_error("failed to acquire swap chain image!");
            }

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
            VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;

            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

            VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;

            if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                    throw std::runtime_error("failed to submit draw command buffer!");
            }

            VkPresentInfoKHR presentInfo = {};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;
            VkSwapchainKHR swapChains[] = {swapChain};
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapChains;
            presentInfo.pImageIndices = &imageIndex;
            vkQueuePresentKHR(presentQueue, &presentInfo);
            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
                    recreateSwapChain();
            } else if (result != VK_SUCCESS) {
                    throw std::runtime_error("failed to present swap chain image!");
            }
        }

        void createSemaphores() {
            VkSemaphoreCreateInfo semaphoreInfo = {};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, imageAvailableSemaphore.replace()) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, renderFinishedSemaphore.replace()) != VK_SUCCESS)
            {

                throw std::runtime_error("failed to create semaphores!");
            }


        }

        void printFPS(){
            auto now = std::chrono::steady_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - last_time);
            auto count = diff.count();

            frames++;
            if (count > 1) {
                std::cout << frames / count << " FPS" << std::endl;
                last_time = now;
                frames = 0;
            }

        }

        void updateUniformBuffer() {
            static auto startTime = std::chrono::high_resolution_clock::now();

            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;


            UniformBufferObject ubo = {};
            ubo.model = glm::rotate(glm::mat4(), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

            ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

            ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
            ubo.proj[1][1] *= -1;

            void* data;
            vkMapMemory(device, uniformStagingBufferMemory, 0, sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
            vkUnmapMemory(device, uniformStagingBufferMemory);

            copyBuffer(uniformStagingBuffer, uniformBuffer, sizeof(ubo));
        }

        void mainLoop() {
            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
                updateUniformBuffer();
                drawFrame();
                printFPS();
            }
           vkDeviceWaitIdle(device);
        }

        void initWindow() {
            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan test", nullptr, nullptr);


            glfwSetWindowUserPointer(window, this);
            glfwSetWindowSizeCallback(window, HelloTriangleApplication::onWindowResized);
        }

        static void onWindowResized(GLFWwindow* window, int width, int height) {
            if (width == 0 || height == 0) return;

            HelloTriangleApplication* app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
            app->recreateSwapChain();
        }
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

