// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreEngine.h"
#include <glm/glm.hpp>
#include "glm/vec3.hpp"
#include "glm/detail/type_vec3.hpp"
#include "glm/geometric.hpp"
#include "glm/matrix.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/type_ptr.inl"

#include "Math.h"
#include "Core/Strings/Name.h"
#include "Core/Reflection/ReflectionTypeTraits.h"


template<glm::length_t L, typename T>
class vec : public glm::vec<L, T> {
	using glm::vec<L, T>::vec;

public:

	FORCEINLINE T Length() const { return glm::length<L, T>(*this); }
	FORCEINLINE T LengthSqrt() const { return glm::length2<L, T>(*this); }

	FORCEINLINE T Distance(const vec<L, T>& other) const { return glm::distance<L, T>(*this, other); }
	FORCEINLINE T DistanceSqrt(const vec<L, T>& other) const { return glm::distance2<L, T>(*this, other); }

	FORCEINLINE vec Normalize() const { return glm::normalize<L, T>(*this); }

	FORCEINLINE T*       Data()       { return &x; }
	FORCEINLINE const T* Data() const { return &x; }

	static constexpr vec<L, T> Zero() { return glm::zero<vec<L, T>>(); }
	static constexpr vec<L, T> One()  { return glm::one<vec<L, T>>(); }
};

class v2 : public vec<2, float> {
	using vec<2, float>::vec;
public:
	class v3 xz() const;
	class v3 xy() const;
};
DECLARE_REFLECTION_TYPE(v2);

using v2_u32 = vec<2, u32>;
DECLARE_REFLECTION_TYPE(v2_u32);

class v3 : public vec<3, float> {
	using vec<3, float>::vec;

public:

	constexpr v3 operator+(const v3& other)
	{
		return { x + other.x, y + other.y, z + other.z };
	}
	constexpr void operator+=(const v3& other)
	{
		x += other.x; y += other.y; z += other.z;
	}
	constexpr v3 operator-(const v3& other)
	{
		return { x - other.x, y - other.y, z - other.z };
	}
	constexpr void operator-=(const v3& other)
	{
		x -= other.x; y -= other.y; z -= other.z;
	}


	constexpr v3 operator*(float other)
	{
		return { x * other, y * other, z * other };
	}

	constexpr v3 operator/(float other)
	{
		return { x / other, y / other, z / other };
	}

	FORCEINLINE v2 xy() const { return v2{ x, y }; }
	FORCEINLINE v2 xz() const { return v2{ x, z }; }

	static const v3 Forward;
	static const v3 Right;
	static const v3 Up;
};
DECLARE_REFLECTION_TYPE(v3);


/** Non reflected vectors */

using v4    = vec<4, float>;

using v2_u8 = vec<2, u8>;
using v3_u8 = vec<3, u8>;
using v4_u8 = vec<4, u8>;

using v2_i32 = vec<2, i32>;
using v3_i32 = vec<3, i32>;
using v4_i32 = vec<4, i32>;

using v3_u32 = vec<3, u32>;
using v4_u32 = vec<4, u32>;


template<glm::length_t X, glm::length_t Y, typename T>
class Matrix : public glm::mat<X, Y, T, glm::highp>
{
	using glm::mat<X, Y, T, glm::highp>::mat;

public:

	Matrix Inverse() const { return glm::inverse(*this); }
	Matrix Transpose() const { return glm::transpose(*this); }
	Matrix InverseTranspose() const { return glm::inverseTranspose(*this); }

	T* Data() { return &operator[](0).x; }
	const T* Data() const { return &operator[](0).x; }

	static constexpr Matrix Identity() { return glm::identity<glm::mat<X, Y, T, glm::highp>>(); }
};

using Matrix4f = Matrix<4, 4, float>;


class Rotator : public v3 {
	using v3::v3;

public:

	float Pitch() const { return y; }
	float Yaw()   const { return z; }
	float Roll()  const { return x; }

	float& Pitch() { return y; }
	float& Yaw()   { return z; }
	float& Roll()  { return x; }

	/**
	 * Clamps an angle to the range of [0, 360).
	 *
	 * @param Angle The angle to clamp.
	 * @return The clamped angle.
	 */
	static float ClampAxis(float Angle);

	/**
	 * Clamps an angle to the range of (-180, 180].
	 *
	 * @param Angle The Angle to clamp.
	 * @return The clamped angle.
	 */
	static float NormalizeAxis(float Angle);
};


class Quat : public glm::qua<float, glm::highp> {
	using glm::qua<float, glm::highp>::qua;

public:

	v3 Rotate(const v3& vector) const;
	v3 Unrotate(const v3& vector) const;

	Rotator ToRotator() const;

	Rotator ToRotatorRad() const {
		return ToRotator() * Math::DEGTORAD;
	}

	v3 GetForward() const { return *this * v3::Forward; }
	v3 GetRight()   const { return *this * v3::Right;   }
	v3 GetUp()      const { return *this * v3::Up;      }

	Quat Inverse() const { glm::inverse(*this); }

	float*       Data()       { return &x; }
	const float* Data() const { return &x; }

	Matrix4f ToMatrix() const { return glm::mat4_cast<float>(*this); }

	bool Equals(const Quat& other, float tolerance = Math::SMALL_NUMBER) const;

	static Quat FromRotator(Rotator rotator);

	static Quat FromRotatorRad(Rotator rotator) {
		return FromRotator(rotator * Math::DEGTORAD);
	}

	static Quat LookAt(const v3& origin, const v3& dest);

	static constexpr Quat Identity() { return glm::quat_identity<value_type, glm::highp>(); }
};
DECLARE_REFLECTION_TYPE(Quat);


template<typename Type, u32 Dimensions>
struct Box {
	using VectorType = glm::mat<Dimensions, 1, Type>;

	VectorType min;
	VectorType max;


	Box() = default;
	constexpr Box(VectorType min, VectorType max) : min{min}, max{max} {}

	inline void ExtendPoint(const VectorType& point) {
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
	inline void Cut(const Box& other) {
		for (u32 i = 0; i < Dimensions; ++i)
		{
			min[i] = Math::Clamp(min[i], other.min[i], other.max[i]);
			max[i] = Math::Clamp(max[i], other.min[i], other.max[i]);
		}
	}

	// Extend this bounds by another
	inline void Extend(const Box& other) {
		for (u32 i = 0; i < Dimensions; ++i)
		{
			if (other.min[i] < min[i]) {
				min[i] = other.min[i];
			}
			if (other.max[i] > max[i]) {
				max[i] = other.max[i];
			}
		}
	}

	// Limit this bounds by another
	FORCEINLINE bool Contains(const Box& other) {
		for (u32 i = 0; i < Dimensions; ++i)
		{
			if (other.min[i] < min[i] || other.max[i] > max[i]) {
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
