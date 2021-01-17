// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "CoreEngine.h"
#include "Math.h"
#include "Reflection/ReflectionTypeTraits.h"
#include "Strings/Name.h"
#include "glm/fwd.hpp"

#include <glm/ext.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>


namespace Rift
{
	template <glm::length_t L, typename T>
	class Vec : public glm::vec<L, T, glm::highp>
	{
		using glm::vec<L, T, glm::highp>::vec;

	public:
		template <glm::qualifier Q>
		Vec(const glm::vec<L, T, Q>& other) : Vec(*static_cast<const Vec<L, T>*>(&other))
		{}

		T Length() const
		{
			return glm::length<L, T>(*this);
		}
		T LengthSqrt() const
		{
			return glm::length2<L, T>(*this);
		}

		T Distance(const Vec<L, T>& other) const
		{
			return glm::distance<L, T>(*this, other);
		}
		T DistanceSqrt(const Vec<L, T>& other) const
		{
			return glm::distance2<L, T>(*this, other);
		}

		Vec Normalize() const
		{
			return glm::normalize<L, T>(*this);
		}

		T* Data()
		{
			return glm::value_ptr(*this);
		}
		const T* Data() const
		{
			return glm::value_ptr(*this);
		}

		static constexpr Vec<L, T> Zero()
		{
			return glm::zero<Vec<L, T>>();
		}
		static constexpr Vec<L, T> One()
		{
			return glm::one<Vec<L, T>>();
		}
	};

	class v2 : public Vec<2, float>
	{
		using Vec<2, float>::Vec;

	public:
		v2(const Vec<2, float>& other) : v2(*static_cast<const v2*>(&other)) {}

		class v3 xz() const;
		class v3 xy() const;
	};
	DECLARE_REFLECTION_TYPE(v2);


	using v2_u32 = Vec<2, u32>;
	DECLARE_REFLECTION_TYPE(v2_u32);


	class v3 : public Vec<3, float>
	{
		using Vec<3, float>::Vec;

	public:
		v3(const Vec<3, float>& other) : v3(*static_cast<const v3*>(&other)) {}

		constexpr v3 operator+(const v3& other)
		{
			return {x + other.x, y + other.y, z + other.z};
		}
		constexpr void operator+=(const v3& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
		}
		constexpr v3 operator-(const v3& other)
		{
			return {x - other.x, y - other.y, z - other.z};
		}
		constexpr void operator-=(const v3& other)
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
		}


		constexpr v3 operator*(float other)
		{
			return {x * other, y * other, z * other};
		}

		constexpr v3 operator/(float other)
		{
			return {x / other, y / other, z / other};
		}

		v2 xy() const
		{
			return v2{x, y};
		}
		v2 xz() const
		{
			return v2{x, z};
		}

		static const v3 Forward;
		static const v3 Right;
		static const v3 Up;
	};
	DECLARE_REFLECTION_TYPE(v3);


	/** Non reflected vectors */

	using v4 = Vec<4, float>;

	using v2_u8 = Vec<2, u8>;
	using v3_u8 = Vec<3, u8>;
	using v4_u8 = Vec<4, u8>;

	using v2_i32 = Vec<2, i32>;
	using v3_i32 = Vec<3, i32>;
	using v4_i32 = Vec<4, i32>;

	using v3_u32 = Vec<3, u32>;
	using v4_u32 = Vec<4, u32>;


	template <typename Type, u32 Dimensions>
	struct Box
	{
		using VectorType = glm::mat<Dimensions, 1, Type>;

		VectorType min;
		VectorType max;


		Box() = default;
		constexpr Box(VectorType min, VectorType max) : min{min}, max{max} {}

		inline void ExtendPoint(const VectorType& point)
		{
			for (u32 i = 0; i < Dimensions; ++i)
			{
				if (point[i] < min[i])
				{
					min[i] = point[i];
				}
				if (point[i] > max[i])
				{
					max[i] = point[i];
				}
			}
		}

		// Limit this bounds by another
		inline void Cut(const Box& other)
		{
			for (u32 i = 0; i < Dimensions; ++i)
			{
				min[i] = Math::Clamp(min[i], other.min[i], other.max[i]);
				max[i] = Math::Clamp(max[i], other.min[i], other.max[i]);
			}
		}

		// Extend this bounds by another
		inline void Extend(const Box& other)
		{
			for (u32 i = 0; i < Dimensions; ++i)
			{
				if (other.min[i] < min[i])
				{
					min[i] = other.min[i];
				}
				if (other.max[i] > max[i])
				{
					max[i] = other.max[i];
				}
			}
		}

		// Limit this bounds by another
		bool Contains(const Box& other)
		{
			for (u32 i = 0; i < Dimensions; ++i)
			{
				if (other.min[i] < min[i] || other.max[i] > max[i])
				{
					return false;
				}
			}
			return true;
		}
	};

	using box2 = Box<float, 2>;
	using box3 = Box<float, 3>;
	using box2_i32 = Box<i32, 2>;
	using box3_i32 = Box<i32, 3>;
}	 // namespace Rift
