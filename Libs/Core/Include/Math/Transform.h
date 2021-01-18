// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "CoreEngine.h"
#include "Math.h"
#include "Matrix.h"
#include "Quaternion.h"
#include "Reflection/ClassTraits.h"
#include "Reflection/ReflectionTypeTraits.h"
#include "Rotator.h"
#include "Vector.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/transform.hpp>


namespace Rift
{
	struct Transform
	{
		v3 location;
		Quat rotation;
		v3 scale;


		Transform() : location(v3::Zero()), rotation(Quat::Identity()), scale(v3::One()){};
		Transform(v3 location) : location(location), rotation(Quat::Identity()), scale(v3::One()){};


		Rotator GetRotation()
		{
			return rotation.ToRotator();
		}
		void SetRotation(const Rotator& angles)
		{
			rotation = Quat::FromRotator(angles);
		}

		bool Serialize(class Archive& ar, const char* name);


		Matrix4f ToMatrix() const
		{
			// Scale -> Rotate -> Translate
			return glm::translate(location) * rotation.ToMatrix() * glm::scale(scale);
		}

		Matrix4f ToMatrixNoScale() const
		{
			// Rotate -> Translate
			return glm::translate(location) * glm::mat4_cast(rotation);
		}

		Matrix4f ToInverseMatrix() const
		{
			return ToMatrix().Inverse();
		}

		Transform Inverse()
		{
			Transform t{};
			t.SetFromMatrix(ToInverseMatrix());
			return t;
		}

		v3 TransformLocation(const v3& p) const
		{
			return rotation.Rotate(p * scale) + location;
		}
		v3 TransformVector(const v3& vector) const
		{
			return rotation.Rotate(vector * scale);
		}
		Quat TransformRotation(const Quat& q) const
		{
			return rotation * q;
		}
		v3 InverseTransformLocation(const v3& p) const
		{
			return rotation.Unrotate(p - location) * Math::GetSafeScaleReciprocal(scale);
		}
		v3 InverseTransformVector(const v3& vector) const
		{
			return rotation.Unrotate(vector) * Math::GetSafeScaleReciprocal(scale);
		}
		Quat InverseTransformRotation(const Quat& q) const
		{
			return rotation.Inverse() * q;
		}

		v3 GetForward() const
		{
			return rotation.GetForward();
		}


		void SetFromMatrix(const Matrix4f& m)
		{
			v3 skew;
			v4 perpective;
			glm::decompose(m, scale, rotation, location, skew, perpective);
		}

		Transform operator*(const Transform& other) const
		{
			Transform t;
			t.SetFromMatrix(ToMatrix() * other.ToMatrix());
			return t;
		}

#if WITH_EDITOR
		static class Class* GetDetailsWidgetClass();
#endif
	};

	DEFINE_CLASS_TRAITS(Transform, HasCustomSerialize = true, HasDetailsWidget = true);

	DECLARE_REFLECTION_TYPE(Transform);
}	 // namespace Rift