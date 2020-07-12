// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/matrix.hpp>


namespace VCLang
{
	template <glm::length_t X, glm::length_t Y, typename T>
	class Matrix : public glm::mat<X, Y, T, glm::highp>
	{
		using glm::mat<X, Y, T, glm::highp>::mat;

	public:
		// Conversion from glm matrix
		template <glm::qualifier Q>
		Matrix(const glm::mat<X, Y, T, Q>& other)
			: Matrix(*static_cast<const Matrix<X, Y, T>*>(&other))
		{}

		Matrix Inverse() const
		{
			return glm::inverse(*this);
		}
		Matrix Transpose() const
		{
			return glm::transpose(*this);
		}
		Matrix InverseTranspose() const
		{
			return glm::inverseTranspose(*this);
		}

		T* Data()
		{
			return &(*this)[0].x;
		}
		const T* Data() const
		{
			return &(*this)[0].x;
		}

		static constexpr Matrix Identity()
		{
			return Matrix(1);
		}
	};

	using Matrix4f = Matrix<4, 4, float>;
}	 // namespace VCLang
