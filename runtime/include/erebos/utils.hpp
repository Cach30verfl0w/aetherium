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

#pragma once
#define RPS_VK_DYNAMIC_LOADING 1
#include <rps/rps.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <volk.h>
#include <atomic>

#define EREBOS_DEFAULT_MOVE(t)       \
    t(t&& other) noexcept = default; \
    auto operator=(t&& other) noexcept -> t& = default;

#define EREBOS_DEFAULT_COPY(t)      \
    t(t& other) noexcept = default; \
    auto operator=(t& other) noexcept -> t& = default;

#define EREBOS_DELETE_COPY(t)      \
    t(t& other) noexcept = delete; \
    auto operator=(t& other) noexcept -> t& = delete;

#define EREBOS_DEFAULT_MOVE_COPY(t) \
    EREBOS_DEFAULT_MOVE(t)          \
    EREBOS_DEFAULT_COPY(t)

#define EREBOS_BITFLAGS(t, n, ...) enum n : t { NONE = 0, __VA_ARGS__ };

namespace erebos {
    using i32 = std::int32_t;

    using u8 = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using usize = std::size_t;

    using atomic_bool = std::atomic_bool;

    template<typename T, T... FLAGS>
    [[nodiscard]] constexpr auto are_flags_set(T value) noexcept -> bool {
        constexpr auto flags = (FLAGS | ...);
        return (value & flags) == flags;
    }

    template<auto... FLAGS>
    [[nodiscard]] constexpr auto is_flag_set(auto value) noexcept -> bool {
        return (value & (FLAGS | ...)) != 0;
    }

    /**
     * This functions converts the specified vulkan error code into a human-readable string-representation of the
     * error.
     *
     * @param error_code Vulkan error code
     * @return           The error message
     * @author           Cedric Hammes
     * @since            14/03/2024
     */
    [[nodiscard]] inline auto vk_strerror(const VkResult error_code) noexcept -> std::string {
        switch(error_code) {
            case VK_SUCCESS:
                return "Success";
            case VK_NOT_READY:
                return "Not ready";
            case VK_TIMEOUT:
                return "Timed out";
            case VK_INCOMPLETE:
                return "Incomplete";
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                return "Out of device memory";
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                return "Out of host memory";
            case VK_ERROR_MEMORY_MAP_FAILED:
                return "Memory map failed";
            case VK_ERROR_LAYER_NOT_PRESENT:
                return "Layer not present";
            case VK_ERROR_EXTENSION_NOT_PRESENT:
                return "Extension not present";
            case VK_ERROR_FEATURE_NOT_PRESENT:
                return "Feature not present";
            case VK_ERROR_INCOMPATIBLE_DRIVER:
                return "Incompatible driver";
            case VK_ERROR_TOO_MANY_OBJECTS:
                return "Too many objects";
            case VK_ERROR_FORMAT_NOT_SUPPORTED:
                return "Format not supported";
            case VK_ERROR_FRAGMENTED_POOL:
                return "Fragmented pool";
            case VK_ERROR_SURFACE_LOST_KHR:
                return "Surface lost";
            case VK_PIPELINE_COMPILE_REQUIRED:
                return "Pipeline compile required";
            case VK_ERROR_INVALID_SHADER_NV:
                return "Invalid shader";
            case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
                return "Invalid device address";
            default:
                return fmt::format("Unknown ({})", static_cast<int8_t>(error_code));
        }
    }
}// namespace erebos
