#pragma once


#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cassert>
#include <cmath>
#include <intrin.h>
#include <iglib.h>

constexpr float Pi = 3.141592653589793f;

using ig::Vector2f;
using ig::Vector2i;

using ig::Recti;
using ig::Rectf;

using ig::Colorb;
using ig::Colorf;

using ig::byte;

#define FORCEINLINE __forceinline
#define CINLINE constexpr FORCEINLINE

constexpr float Epsilon = 1e-5f;

namespace std
{
	template <>
	struct hash<Vector2i>
	{
		inline size_t operator()(const Vector2i _Vec) const
		{
			if (sizeof(size_t) == 8)
				return (size_t(_Vec.x) << 32) + _Vec.y;
			else
				return size_t(_Vec.x ^ _Vec.y) + _Vec.y;
		}

	};
}

template <typename _T>
FORCEINLINE constexpr _T clamp(_T v, _T min, _T max)
{
	if (v > max)
		return max;
	return v < min ? min : v;
}
