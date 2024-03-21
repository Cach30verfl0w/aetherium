//  Copyright 2024 Cach30verfl0w
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

/**
 * @author Cedric Hammes
 * @since  14/03/2024
 */

#include "erebos/vulkan/device.hpp"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace erebos::vulkan {
    namespace {
        auto get_device_local_heap(VkPhysicalDevice device_handle) -> uint32_t {
            VkPhysicalDeviceMemoryProperties memory_properties {};
            vkGetPhysicalDeviceMemoryProperties(device_handle, &memory_properties);

            uint32_t local_heap_size = 0;
            for(uint32_t i = 0; i < memory_properties.memoryHeapCount; i++) {
                const auto heap = memory_properties.memoryHeaps[i];
                if((heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                    continue;

                local_heap_size += heap.size;
            }
            return local_heap_size;
        }

        constexpr auto compare_by_local_heap = [](const auto& left, const auto& right) -> bool {
            return get_device_local_heap(left) < get_device_local_heap(right);
        };
    }// namespace

    /**
     * This constructor creates a virtual device handle by the specified physical device
     * handle.
     *
     * @param physical_device_handle The handle to the physical device
     * @author                       Cedric Hammes
     * @since                        14/03/2024
     */
    Device::Device(const VulkanContext& context, VkPhysicalDevice physical_device_handle) ://NOLINT
            _phy_device {physical_device_handle},
            _device {},
            _queue {} {
        constexpr auto queue_property = 1.0f;
        constexpr std::array<const char*, 1> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        // Get properties
        VkPhysicalDeviceProperties device_properties {};
        ::vkGetPhysicalDeviceProperties(_phy_device, &device_properties);

        // Enable dynamic rendering for device
        VkPhysicalDeviceVulkan13Features vulkan13_features {};
        vulkan13_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        vulkan13_features.dynamicRendering = VK_TRUE;

        VkPhysicalDeviceFeatures2 features {};
        features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features.pNext = &vulkan13_features;

        // Create device queue
        VkDeviceQueueCreateInfo device_queue_create_info {};
        device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        device_queue_create_info.queueCount = 1;
        device_queue_create_info.queueFamilyIndex = 0;
        device_queue_create_info.pQueuePriorities = &queue_property;

        // Create device itself
        SPDLOG_INFO("Creating device '{}' (Driver Version: {}.{}.{})", device_properties.deviceName,
                    VK_API_VERSION_MAJOR(device_properties.driverVersion),
                    VK_API_VERSION_MINOR(device_properties.driverVersion),
                    VK_API_VERSION_PATCH(device_properties.driverVersion));

        VkDeviceCreateInfo device_create_info {};
        device_create_info.pNext = &features;
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.pQueueCreateInfos = &device_queue_create_info;
        device_create_info.queueCreateInfoCount = 1;
        device_create_info.enabledLayerCount = 0;
        device_create_info.enabledExtensionCount = device_extensions.size();
        device_create_info.ppEnabledExtensionNames = device_extensions.data();
        if(const auto err = ::vkCreateDevice(_phy_device, &device_create_info, nullptr, &_device); err != VK_SUCCESS) {
            throw std::runtime_error {fmt::format("Unable to create device: {}", vk_strerror(err))};
        }
        ::volkLoadDevice(_device);
        ::vkGetDeviceQueue(_device, 0, 0, &_queue);

        // Create memory allocator
        uint32_t instance_version {};
        if(const auto err = ::vkEnumerateInstanceVersion(&instance_version); err != VK_SUCCESS) {
            throw std::runtime_error {fmt::format("Unable to get instance version: {}", vk_strerror(err))};
        }

        VmaVulkanFunctions vma_vulkan_functions {};
        vma_vulkan_functions.vkAllocateMemory = vkAllocateMemory;
        vma_vulkan_functions.vkBindBufferMemory = vkBindBufferMemory;
        vma_vulkan_functions.vkBindImageMemory = vkBindImageMemory;
        vma_vulkan_functions.vkCreateBuffer = vkCreateBuffer;
        vma_vulkan_functions.vkCreateImage = vkCreateImage;
        vma_vulkan_functions.vkDestroyBuffer = vkDestroyBuffer;
        vma_vulkan_functions.vkDestroyImage = vkDestroyImage;
        vma_vulkan_functions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
        vma_vulkan_functions.vkFreeMemory = vkFreeMemory;
        vma_vulkan_functions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
        vma_vulkan_functions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
        vma_vulkan_functions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
        vma_vulkan_functions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
        vma_vulkan_functions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
        vma_vulkan_functions.vkMapMemory = vkMapMemory;
        vma_vulkan_functions.vkUnmapMemory = vkUnmapMemory;
        vma_vulkan_functions.vkCmdCopyBuffer = vkCmdCopyBuffer;
        vma_vulkan_functions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2;
        vma_vulkan_functions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2;
        vma_vulkan_functions.vkBindBufferMemory2KHR = vkBindBufferMemory2;
        vma_vulkan_functions.vkBindImageMemory2KHR = vkBindImageMemory2;
        vma_vulkan_functions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;

        VmaAllocatorCreateInfo allocator_create_info {};
        allocator_create_info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
        allocator_create_info.vulkanApiVersion = instance_version;
        allocator_create_info.physicalDevice = _phy_device;
        allocator_create_info.device = _device;
        allocator_create_info.instance = *context;
        allocator_create_info.pVulkanFunctions = &vma_vulkan_functions;
        if(const auto err = ::vmaCreateAllocator(&allocator_create_info, &_allocator); err != VK_SUCCESS) {
            throw std::runtime_error {fmt::format("Unable to create memory allocator: {}", vk_strerror(err))};
        }
    }

    Device::Device(Device&& other) noexcept :
            _phy_device {other._phy_device},
            _device {other._device},
            _queue {other._queue},
            _allocator {other._allocator} {
        other._phy_device = nullptr;
        other._device = nullptr;
        other._queue = nullptr;
        other._allocator = nullptr;
    }

    /**
     * This destructor destroys the device, when the device was not destroyed
     * already.
     *
     * @author Cedric Hammes
     * @since  14/03/2024
     */
    Device::~Device() noexcept {
        if(_allocator == nullptr) {
            ::vmaDestroyAllocator(_allocator);
            _allocator = nullptr;
        }

        if(_device != nullptr) {
            ::vkDestroyDevice(_device, nullptr);
            _device = nullptr;
        }
    }

    auto Device::operator=(Device&& other) noexcept -> Device& {
        _phy_device = other._phy_device;
        _device = other._device;
        _queue = other._queue;
        _allocator = other._allocator;
        other._phy_device = nullptr;
        other._device = nullptr;
        other._queue = nullptr;
        other._allocator = nullptr;
        return *this;
    }

    /**
     * This function enumerates all devices and returns that device with the biggest local
     * heap.
     *
     * @param context The context of the Vulkan application
     * @return        The found device or an error
     * @author        Cedric Hammes
     * @since         14/03/2024
     */
    [[nodiscard]] auto find_device(const VulkanContext& context) noexcept -> kstd::Result<Device> {
        uint32_t device_count = 0;
        if(const auto err = vkEnumeratePhysicalDevices(*context, &device_count, nullptr); err != VK_SUCCESS) {
            return kstd::Error {fmt::format("Unable to get physical device count: {}", vk_strerror(err))};
        }
        SPDLOG_INFO("Found {} devices in total", device_count);

        std::vector<VkPhysicalDevice> devices {device_count};
        if(const auto err = vkEnumeratePhysicalDevices(*context, &device_count, devices.data()); err != VK_SUCCESS) {
            return kstd::Error {fmt::format("Unable to get physical devices: {}", vk_strerror(err))};
        }

        return {{context, devices.at(0)}};
    }
}// namespace erebos::vulkan