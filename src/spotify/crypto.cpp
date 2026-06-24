#include <bit>
#include <bitset>

#include <cctype>
#include <span>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>

#include <string>
#include <stdexcept>
#include <sstream>

#include <vector>
#include "Spotify/crypto.hpp"
#include <openssl/rand.h>

namespace crypto {
    using BYTE = unsigned char;


    namespace {
        constexpr size_t BLOCK_SIZE{ 64 };
        constexpr size_t WORD_COUNT{ 64 };
        constexpr size_t LENGTH_OFF_SET{ 56 };
        constexpr size_t BYTE_TO_BITS{ 8 };

        constexpr std::array<uint32_t, WORD_COUNT> k {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
            0x3956c25b, 0x59f111f1, 0x923f82a4, 
            0xab1c5ed5, 0xd807aa98, 0x12835b01,
            0x243185be, 0x550c7dc3, 0x72be5d74, 
            0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 
            0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 
            0x5cb0a9dc, 0x76f988da, 0x983e5152, 
            0xa831c66d, 0xb00327c8, 0xbf597fc7, 
            0xc6e00bf3, 0xd5a79147, 0x06ca6351, 
            0x14292967, 0x27b70a85, 0x2e1b2138, 
            0x4d2c6dfc, 0x53380d13, 0x650a7354, 
            0x766a0abb, 0x81c2c92e, 0x92722c85, 
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 
            0xc76c51a3, 0xd192e819, 0xd6990624, 
            0xf40e3585, 0x106aa070, 0x19a4c116, 
            0x1e376c08, 0x2748774c, 0x34b0bcb5, 
            0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 
            0x682e6ff3, 0x748f82ee, 0x78a5636f, 
            0x84c87814, 0x8cc70208, 0x90befffa, 
            0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };

        [[maybe_unused]] void printBinary(std::vector<BYTE> data) {
            bool first = true;
            for (BYTE b : data) {
                if (!first) std::cout << " ";

                first = false;
                std::cout << std::bitset<BYTE_TO_BITS>(b);
            }
            std::cout << '\n';
        }

        [[maybe_unused]] void printBinary(std::array<uint32_t, BLOCK_SIZE> w) {
            bool first = true;
            for (std::size_t i{}; i < w.size(); ++i) {
                if (!first) std::cout << " ";

                first = false;
                std::cout << std::bitset<32>(w[i]);
            }
            std::cout << '\n';
        }
    }

    // randomStrGen(length) generates a cryptographical random string of size 
    //  `length` using chracters permitted by RFC 7636 PKCE code verifiers such as:
    //   'A-Z', 'a-z', '-._~'
    // requires: length >= 43 && length <= 128
    // throws: std::runtime_error if secure random byte generation fails.
    // effects: None
    std::string randomStrGen(std::size_t length) {
        assert(length >= 43 && length <= 128);
        constexpr char charSet[]{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~" };

        std::string result;
        result.reserve(length);

        constexpr std::size_t charSetLength{ sizeof(charSet) - 1 };
        std::vector<BYTE> randomBytes(length);
        if (RAND_bytes(randomBytes.data(), static_cast<int>(randomBytes.size())) != 1) {
            throw std::runtime_error("RAND_bytes failed");
        }
        for (unsigned char b : randomBytes) {
            result.push_back(charSet[b % charSetLength]);
        }
        return result;
    }


    // sha256(input) computes the SHA-256 hash encoded array given a string input.
    //  Returns the SHA-256 digest of input as a std::array<BYTE, 32> in big-endian order.
    // requires: None
    // throws: Any exception resulting from internal memory allocation failures.
    // effects: None.
    std::array<BYTE, 32> sha256(std::string_view input) {
        std::array<uint32_t, 8> hashValues{ 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 
            0xa54ff53a, 0x510e527f, 0x9b05688c,
            0x1f83d9ab, 0x5be0cd19 };

        std::vector<BYTE> data(input.begin(), input.end());
        std::size_t orgLength { data.size() };
        std::uint64_t bitLength { static_cast<std::uint64_t>(orgLength) * BYTE_TO_BITS };

        // Append a 0b10000000 to the std::vector
        data.push_back(0b10000000);

        // Pad with zeros until data's length is congreuent to 448 mod (512)
        while ((data.size() % BLOCK_SIZE) != LENGTH_OFF_SET) {
            data.push_back(0b00000000);
        }

        // Append BLOCK_SIZE bits that represents the length of the orignal input
        for (std::size_t i{0}; i < 8; ++i) {  
            data.push_back(static_cast<BYTE>(bitLength >> (LENGTH_OFF_SET - BYTE_TO_BITS * i) & 0xFF));
        }

        for (std::size_t i{}; i < data.size(); i += BLOCK_SIZE) {
            // Copy the 512 bits chunk data into a new array w
            constexpr std::size_t MLENGTH{BLOCK_SIZE};
            uint32_t w[MLENGTH]{};

            for (std::size_t j{}; j < 16; ++j) {
                std::size_t offset = i + j * 4;

                const BYTE* block{data.data() + offset};
                w[j] = 
                    (static_cast<uint32_t>(block[0]) << 24) |
                    (static_cast<uint32_t>(block[1]) << 16) |
                    (static_cast<uint32_t>(block[2]) << BYTE_TO_BITS) |
                    (static_cast<uint32_t>(block[3]));
            }

            for (std::size_t j{16}; j < BLOCK_SIZE; ++j) {
                uint32_t s0{ (std::rotr(w[j - 15], 7)) ^ (std::rotr(w[j - 15], 18)) ^ (w[j - 15] >> 3) };
                uint32_t s1{ (std::rotr(w[j - 2], 17) ^ (std::rotr(w[j - 2], 19)) ^ (w[j - 2] >> 10)) };
                w[j] = w[j - 16] + s0 + w[j - 7] + s1;
            }

            uint32_t a{hashValues[0]};
            uint32_t b{hashValues[1]};
            uint32_t uc{hashValues[2]};
            uint32_t d{hashValues[3]};
            uint32_t e{hashValues[4]};
            uint32_t f{hashValues[5]};
            uint32_t g{hashValues[6]};
            uint32_t h{hashValues[7]};

            for(std::size_t j{}; j < MLENGTH; ++j) {
                uint32_t s1{ (std::rotr(e, 6)) ^ (std::rotr(e, 11)) ^ (std::rotr(e, 25)) };
                uint32_t Ch{ (e & f) ^ (~e & g)};
                uint32_t temp1{ h + s1 + Ch + k[j] + w[j] };

                uint32_t s0{ (std::rotr(a, 2)) ^ (std::rotr(a, 13)) ^ (std::rotr(a, 22)) };
                uint32_t Maj{ (a & b) ^ (b & uc) ^ (uc & a) };
                uint32_t temp2{ s0 + Maj };

                h = g;
                g = f;
                f = e;
                e = d + temp1;
                d = uc;
                uc = b;
                b = a;
                a = temp1 + temp2;
            }

            hashValues[0] += a;
            hashValues[1] += b;
            hashValues[2] += uc;
            hashValues[3] += d;
            hashValues[4] += e;
            hashValues[5] += f;
            hashValues[6] += g;
            hashValues[7] += h;
        }

        std::array<BYTE, 32> digest{};
        for (std::size_t i{}; i < hashValues.size(); ++i) {
            digest[i * 4 + 0] = static_cast<BYTE>((hashValues[i] >> 24) & 0xFF);
            digest[i * 4 + 1] = static_cast<BYTE>((hashValues[i] >> 16) & 0xFF);
            digest[i * 4 + 2] = static_cast<BYTE>((hashValues[i] >> 8) & 0xFF);
            digest[i * 4 + 3] = static_cast<BYTE>(hashValues[i] & 0xFF);
        }
        return digest;
    }

    // toHex(digest) converts a digest message into a hexadecimal string
    std::string toHex(const std::array<BYTE, 32>& digest) {
        std::stringstream oss;
        oss << std::hex << std::setfill('0');

        for (unsigned char uc : digest) {
            oss << std::setw(2) << static_cast<int>(uc);
        }

        return oss.str();
    }

    // base64_encode(input) encodes a BYTE sequnece in Base64URL format.
    //  The encoding uses the URL-safe Base64 alphabet:
    //  `A-Z`, `a-z`, `0-9`, `-`, `_`.
    //  Padding characters '=' are not produced.
    // requires: input.size() >= 0 (any valid std::span<const BYTE>).
    // effects: None
    std::string base64_encode(std::span<const BYTE> input) {
        constexpr std::string_view BASE64_URL{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_" };

        std::string output;
        std::size_t lenEncode{ (input.size() / 3) * 4 };
       
        switch (input.size() % 3) {
            case 1: lenEncode += 2; break;
            case 2: lenEncode += 3; break;
        }

        output.reserve(lenEncode);
        for (std::size_t pos{}; pos < input.size(); pos += 3) {
            output.push_back(BASE64_URL[(input[pos + 0] & 0xFC) >> 2]);

            if (pos + 1 < input.size()) {
                output.push_back(BASE64_URL[((input[pos + 0] & 0x03u) << 4u ) | ((input[pos + 1] & 0xF0u) >> 4u)]);

                if (pos + 2 < input.size()) {
                    output.push_back(BASE64_URL[((input[pos + 1] & 0x0Fu) << 2u) | ((input[pos + 2] & 0xC0u) >> 6u)]);
                    output.push_back(BASE64_URL[((input[pos + 2] & 0x3F))]);
                } else {
                    output.push_back(BASE64_URL[(input[pos + 1] & 0x0Fu) << 2u]);
                }

            } else {
                output.push_back(BASE64_URL[(input[pos + 0] & 0x03u) << 4u]);
            }
        }
        return output;
    }

    std::string urlEncode(std::string_view input) {
        std::ostringstream escaped;

        escaped << std::hex << std::uppercase;
        for (char c : input) {
            BYTE uc = static_cast<BYTE>(c);

            if (std::isalnum(uc) || 
                    uc == '-' || uc == '_' || 
                    uc == '.' || uc == '~') {
                escaped << uc;
            } else {
                escaped << '%'
                     << std::setw(2)
                     << std::setfill('0')
                     << static_cast<int>(uc);
            }
        }

        return escaped.str();
    }
}
