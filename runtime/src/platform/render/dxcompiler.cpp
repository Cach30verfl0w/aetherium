//   Copyright 2024 Cach30verfl0w
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

/**
 * @author Cedric Hammes
 * @since  18/03/2024
 */

#include "libaetherium/render/dxcompiler.hpp"
#include <iostream>

namespace libaetherium::render {
    DXCompiler::DXCompiler(const std::filesystem::path& path) ://NOLINT
            _library_loader {platform::LibraryLoader {path.string()}},
            _dxc_compiler {},
            _dxc_utils {} {
        // clang-format off
        _DxcCreateInstance =_library_loader.get_function<HRESULT, REFCLSID, REFIID, LPVOID*>("DxcCreateInstance")
                .get_or_throw();
        // clang-format on

        _DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_dxc_utils));
        if (!_dxc_utils) {
            throw std::runtime_error {"Unable to initialize DX Compiler: Failed to initialize DXC Utils"};
        }

        _DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_dxc_compiler));
        if (!_dxc_compiler) {
            throw std::runtime_error {"Unable to initialize DX Compiler: Failed to initialize DX Compiler itself"};
        }
        SPDLOG_INFO("Successfully initialized DX Compiler");
    }

}// namespace libaetherium::render