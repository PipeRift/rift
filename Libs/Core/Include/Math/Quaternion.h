// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Math.h"
#include "Matrix.h"
#include "Reflection/ReflectionTypeTraits.h"

#include <glm/ext/quaternion_common.hpp>
#include <glm/gtx/quaternion.hpp>


namespace VCLang
{
	class v3;
	class Rotator;

	class Quat : public glm::qua<float, glm::highp>
	{
		using glm::qua<float, glm::highp>::qua;

	public:
		// Conversion from glm quaternion
		template <glm::qualifier Q>
		Quat(const glm::qua<float, Q>& other) : Quat(*static_cast<const Quat*>(&other))
		{}

		v3 Rotate(const v3& vector) const;
		v3 Unrotate(const v3& vector) const;

		Rotator ToRotator() const;
		Rotator ToRotatorRad() const;

		v3 GetForward() const;
		v3 GetRight() const;
		v3 GetUp() const;

		Quat Inverse() const;

		float* Data()
		{
			return &x;
		}
		const float* Data() const
		{
			return &x;
		}

		Matrix4f ToMatrix() const
		{
			return glm::mat4_cast<float>(*this);
		}

		bool Equals(const Quat& other, float tolerance = Math::SMALL_NUMBER) const;

		static Quat FromRotator(Rotator rotator);

		static Quat FromRotatorRad(Rotator rotator);

		static Quat LookAt(const v3& origin, const v3& dest);

		static constexpr Quat Identity()
		{
			return {1, 0, 0, 0};
		}
	};
	DECLARE_REFLECTION_TYPE(Quat);


	inline Quat Quat::Inverse() const
	{
		const auto result = glm::inverse<float, glm::highp>(*this);
		return *static_cast<const Quat*>(&result);
	}
}	 // namespace VCLang
