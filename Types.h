#pragma once

#include <cstdint>

using u64 = std::uint64_t;
using u32 = std::uint32_t;
using u16 = std::uint16_t;
using u8 = std::uint8_t;
using s64 = std::int64_t;
using s32 = std::int32_t;
using s16 = std::int16_t;
using s8 = std::int8_t;

using uptr = std::uintptr_t;
using sptr = std::intptr_t;

using uint = unsigned int;
using byte = unsigned char;

using f32 = float;
using f64 = double;

#include <string>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
typedef std::vector<byte> ByteVector;
using std::vector;
using std::string;
using std::array;
using std::map;
using std::pair;
using std::unordered_map;

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