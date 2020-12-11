#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <vector>
#include <string>
#include <array>

using u64 = std::uint64_t;
using u32 = std::uint32_t;
using u16 = std::uint16_t;
using u8 = std::uint8_t;
using s64 = std::int64_t;
using s32 = std::int32_t;
using s16 = std::int16_t;
using s8 = std::int8_t;

using Bytes = std::vector<u8>;

using std::vector;
using std::string;
using std::array;

template<typename T, size_t ARR_SIZE>
size_t array_countof(T(&)[ARR_SIZE]) { return ARR_SIZE; }

template<typename T, typename Y>
T align_up(const T n, const Y align)
{
    const T alignm1 = align - 1;
    return (n + alignm1) & (~alignm1);
}

template<typename T, typename Y>
T align_down(const T n, const Y align)
{
    const T alignm1 = align - 1;
    return n & (~alignm1);
}

#endif // TYPES_H
