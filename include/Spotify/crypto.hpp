#pragma once

#include <span>
#include <string_view>
#include <cstddef>

namespace crypto {
    using BYTE = unsigned char;

    std::string randomStrGen(std::size_t length = 128);
    std::array<BYTE, 32> sha256(std::string_view input);
    std::string toHex(const std::array<BYTE, 32>& digest);
    std::string base64_encode(std::span<const BYTE> input);
}
