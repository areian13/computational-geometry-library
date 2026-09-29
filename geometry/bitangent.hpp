#pragma once

#include <optional>
#include <vector>

#include "segment.hpp"

enum class BitangentType { LL, LR, RL, RR };

struct Bitangent {
    Segment segment;
    BitangentType type;

    Bitangent(const Segment& segment, BitangentType type) : segment(segment), type(type) {}
    Bitangent(const Point& a, const Point& b, BitangentType type) : segment(a, b), type(type) {}
};

inline std::optional<Bitangent> bitangent(
    const std::vector<Bitangent>& bits,
    BitangentType type
) {
    for (const auto& bit : bits)
        if (bit.type == type) return bit;
    return std::nullopt;
}
template <typename T>
std::optional<Bitangent> bitangent(
    const T& a, const T& b, 
    BitangentType type
) {
    return bitangent(bitangents(a, b), type);
}

inline std::optional<Bitangent> bit_LL(const std::vector<Bitangent>& bits) {
    return bitangent(bits, BitangentType::LL);
}
inline std::optional<Bitangent> bit_LR(const std::vector<Bitangent>& bits) {
    return bitangent(bits, BitangentType::LR);
}
inline std::optional<Bitangent> bit_RL(const std::vector<Bitangent>& bits) {
    return bitangent(bits, BitangentType::RL);
}
inline std::optional<Bitangent> bit_RR(const std::vector<Bitangent>& bits) {
    return bitangent(bits, BitangentType::RR);
}

template <typename T>
std::optional<Bitangent> bit_LL(const T& a, const T& b) {
    return bitangent(a, b, BitangentType::LL);
}
template <typename T>
std::optional<Bitangent> bit_LR(const T& a, const T& b) {
    return bitangent(a, b, BitangentType::LR);
}
template <typename T>
std::optional<Bitangent> bit_RL(const T& a, const T& b) {
    return bitangent(a, b, BitangentType::RL);
}
template <typename T>
std::optional<Bitangent> bit_RR(const T& a, const T& b) {
    return bitangent(a, b, BitangentType::RR);
}