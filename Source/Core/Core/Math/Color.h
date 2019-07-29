// Copyright 1998-2018 Epic Games, Inother. All Rights Reserved.

#pragma once

#include "CoreEngine.h"
#include "Math.h"
#include "Vector.h"
#include "Core/Containers/Array.h"
#include "Core/Strings/String.h"

#include "Core/Reflection/ClassTraits.h"


/**
 * Enum for the different kinds of gamma spaces we expect to need to convert from/to.
 */
enum class EGammaSpace
{
	/** No gamma correction is applied to this space, the incoming colors are assumed to already be in linear space. */
	Linear,
	/** A simplified sRGB gamma correction is applied, pow(1/2.2). */
	Pow22,
	/** Use the standard sRGB conversion. */
	sRGB,
};

struct Color;


/**
 * A linear, 32-bit/component floating point RGBA color.
 */
struct LinearColor
{
	float r, g, b, a;

	/** Static lookup table used for FColor -> FLinearColor conversion. Pow(2.2) */
	static float Pow22OneOver255Table[256];

	/** Static lookup table used for FColor -> FLinearColor conversion. sRGB */
	static float sRGBToLinearTable[256];

	FORCEINLINE explicit LinearColor() : r(0.f), g(0.f), b(0.f), a(0.f) {}
	constexpr FORCEINLINE LinearColor(float r, float g, float b, float a = 1.0f): r(r), g(g), b(b), a(a) {}

	/**
	 * Converts an FColor which is assumed to be in sRGB space, into linear color space.
	 * @param Color The sRGB color that needs to be converted into linear space.
	 */
	LinearColor(const Color& Color);

	LinearColor(const v3& vector);

	explicit LinearColor(const v4& vector);


	bool Serialize(class Archive& ar, const char* name);

	// Conversions.
	Color ToRGBE() const;

	/**
	 * Converts an FColor coming from an observed sRGB output, into a linear color.
	 * @param Color The sRGB color that needs to be converted into linear space.
	 */
	static LinearColor FromSRGBColor(const Color& Color);

	/**
	 * Converts an FColor coming from an observed Pow(1/2.2) output, into a linear color.
	 * @param Color The Pow(1/2.2) color that needs to be converted into linear space.
	 */
	static LinearColor FromPow22Color(const Color& Color);

	// Operators.

	FORCEINLINE float& Component(i32 index) { return (&r)[index]; }
	FORCEINLINE const float& Component(i32 index) const { return (&r)[index]; }

	FORCEINLINE LinearColor operator+(const LinearColor& other) const
	{
		return LinearColor(
			r + other.r,
			g + other.g,
			b + other.b,
			a + other.a
		);
	}
	FORCEINLINE LinearColor& operator+=(const LinearColor& other)
	{
		r += other.r;
		g += other.g;
		b += other.b;
		a += other.a;
		return *this;
	}

	FORCEINLINE LinearColor operator-(const LinearColor& other) const
	{
		return LinearColor(
			r - other.r,
			g - other.g,
			b - other.b,
			a - other.a
		);
	}
	FORCEINLINE LinearColor& operator-=(const LinearColor& other)
	{
		r -= other.r;
		g -= other.g;
		b -= other.b;
		a -= other.a;
		return *this;
	}

	FORCEINLINE LinearColor operator*(const LinearColor& other) const
	{
		return LinearColor(
			r * other.r,
			g * other.g,
			b * other.b,
			a * other.a
		);
	}
	FORCEINLINE LinearColor& operator*=(const LinearColor& other)
	{
		r *= other.r;
		g *= other.g;
		b *= other.b;
		a *= other.a;
		return *this;
	}

	FORCEINLINE LinearColor operator*(float scalar) const
	{
		return LinearColor(
			r * scalar,
			g * scalar,
			b * scalar,
			a * scalar
		);
	}

	FORCEINLINE LinearColor& operator*=(float scalar)
	{
		r *= scalar;
		g *= scalar;
		b *= scalar;
		a *= scalar;
		return *this;
	}

	FORCEINLINE LinearColor operator/(const LinearColor& other) const
	{
		return LinearColor(
			r / other.r,
			g / other.g,
			b / other.b,
			a / other.a
		);
	}
	FORCEINLINE LinearColor& operator/=(const LinearColor& other)
	{
		r /= other.r;
		g /= other.g;
		b /= other.b;
		a /= other.a;
		return *this;
	}

	FORCEINLINE LinearColor operator/(float scalar) const
	{
		const float	invScalar = 1.0f / scalar;
		return LinearColor(
			r * invScalar,
			g * invScalar,
			b * invScalar,
			a * invScalar
		);
	}
	FORCEINLINE LinearColor& operator/=(float scalar)
	{
		const float	invScalar = 1.0f / scalar;
		r *= invScalar;
		g *= invScalar;
		b *= invScalar;
		a *= invScalar;
		return *this;
	}

	// clamped in 0..1 range
	FORCEINLINE LinearColor GetClamped(float min = 0.0f, float max = 1.0f) const
	{
		LinearColor ret;
		ret.r = Math::Clamp(r, min, max);
		ret.g = Math::Clamp(g, min, max);
		ret.b = Math::Clamp(b, min, max);
		ret.a = Math::Clamp(a, min, max);
		return ret;
	}

	/** Comparison operators */
	FORCEINLINE bool operator==(const LinearColor& other) const
	{
		return r == other.r && g == other.g && b == other.b && a == other.a;
	}
	FORCEINLINE bool operator!=(const LinearColor& Other) const
	{
		return r != Other.r || g != Other.g || b != Other.b || a != Other.a;
	}

	// Error-tolerant comparison.
	FORCEINLINE bool Equals(const LinearColor& other, float Tolerance = Math::SMALL_NUMBER) const
	{
		return Math::Abs(r - other.r) < Tolerance && Math::Abs(g - other.g) < Tolerance && Math::Abs(b - other.b) < Tolerance && Math::Abs(a - other.a) < Tolerance;
	}

	LinearColor CopyWithNewOpacity(float NewOpacity) const
	{
		LinearColor NewCopy = *this;
		NewCopy.a = NewOpacity;
		return NewCopy;
	}

	v3 ToV3() const { return { r, g, b };    }
	v4 ToV4() const { return { r, g, b, a }; }

	/**
	 * Converts byte hue-saturation-brightness to floating point red-green-blue.
	 */
	static LinearColor FGetHSV(u8 H,u8 S,u8 V);

	/**
	* Makes a random but quite nice color.
	*/
	static LinearColor MakeRandomColor();

	/**
	* Converts temperature in Kelvins of a black body radiator to RGB chromaticism.
	*/
	static LinearColor MakeFromColorTemperature( float temp );

	/**
	 * Euclidean distance between two points.
	 */
	static inline float Dist( const LinearColor &one, const LinearColor &other )
	{
		return Math::Sqrt( Math::Square(other.r-one.r) + Math::Square(other.g-one.g) + Math::Square(other.b-one.b) + Math::Square(other.a-one.a) );
	}

	/**
	 * Generates a list of sample points on a Bezier curve defined by 2 points.
	 *
	 * @param	ControlPoints	Array of 4 Linear Colors (vert1, controlpoint1, controlpoint2, vert2).
	 * @param	NumPoints		Number of samples.
	 * @param	OutPoints		Receives the output samples.
	 * @return					Path length.
	 */
	static float EvaluateBezier(const LinearColor* ControlPoints, i32 NumPoints, TArray<LinearColor>& OutPoints);

	/** Converts a linear space RGB color to an HSV color */
	LinearColor LinearRGBToHSV() const;

	/** Converts an HSV color to a linear space RGB color */
	LinearColor HSVToLinearRGB() const;

	/**
	 * Linearly interpolates between two colors by the specified progress amount.  The interpolation is performed in HSV color space
	 * taking the shortest path to the new color's hue.  This can give better results than Math::Lerp(), but is much more expensive.
	 * The incoming colors are in RGB space, and the output color will be RGB.  The alpha value will also be interpolated.
	 *
	 * @param	From		The color and alpha to interpolate from as linear RGBA
	 * @param	To			The color and alpha to interpolate to as linear RGBA
	 * @param	Progress	scalar interpolation amount (usually between 0.0 and 1.0 inclusive)
	 * @return	The interpolated color in linear RGB space along with the interpolated alpha value
	 */
	static LinearColor LerpUsingHSV( const LinearColor& From, const LinearColor& To, const float Progress );

	/** Quantizes the linear color and returns the result as a FColor.  This bypasses the SRGB conversion. */
	Color Quantize() const;

	/** Quantizes the linear color with rounding and returns the result as a FColor.  This bypasses the SRGB conversion. */
	Color QuantizeRound() const;

	/** Quantizes the linear color and returns the result as a FColor with optional sRGB conversion and quality as goal. */
	Color ToColor(const bool bSRGB) const;

	/**
	 * Returns a desaturated color, with 0 meaning no desaturation and 1 == full desaturation
	 *
	 * @param	Desaturation	Desaturation factor in range [0..1]
	 * @return	Desaturated color
	 */
	LinearColor Desaturate( float Desaturation ) const;

	/** Computes the perceptually weighted luminance value of a color. */
	inline float ComputeLuminance() const
	{
		return r * 0.3f + g * 0.59f + b * 0.11f;
	}

	/**
	 * Returns the maximum value in this color structure
	 *
	 * @return The maximum color channel value
	 */
	FORCEINLINE float GetMax() const
	{
		return Math::Max( Math::Max( Math::Max( r, g ), b ), a );
	}

	/** useful to detect if a light contribution needs to be rendered */
	bool IsAlmostBlack() const
	{
		return Math::Square(r) < Math::SMALLER_NUMBER && Math::Square(g) < Math::SMALLER_NUMBER && Math::Square(b) < Math::SMALLER_NUMBER;
	}

	/**
	 * Returns the minimum value in this color structure
	 *
	 * @return The minimum color channel value
	 */
	FORCEINLINE float GetMin() const
	{
		return Math::Min( Math::Min( Math::Min( r, g ), b ), a );
	}

	FORCEINLINE float GetLuminance() const
	{
		return r * 0.3f + g * 0.59f + b * 0.11f;
	}

	String ToString() const {
		String str;
		str.sprintf(TX("(r=%f,g=%f,b=%f,a=%f)"), r, g, b, a);
		return eastl::move(str);
	}

	// Common colors.
	static const LinearColor White;
	static const LinearColor Gray;
	static const LinearColor Black;
	static const LinearColor Transparent;
	static const LinearColor Red;
	static const LinearColor Green;
	static const LinearColor Blue;
	static const LinearColor Yellow;
};

FORCEINLINE LinearColor operator*(float scalar,const LinearColor& Color)
{
	return Color.operator*( scalar );
}

//
//	FColor
//	Stores a color with 8 bits of precision per channel.
//	Note: Linear color values should always be converted to gamma space before stored in an FColor, as 8 bits of precision is not enough to store linear space colors!
//	This can be done with FLinearColor::ToFColor(true)
//

struct Color
{
public:
	#pragma warning(disable: 4201) // Avoid warning about nameless struct
	union {
		struct { u8 r, g, b, a; };
		u32 alignmentDummy;
	};
	#pragma warning(default: 4201)

	u32& DWColor(void) {return *((u32*)this);}
	const u32& DWColor(void) const {return *((u32*)this);}

	// Constructors.
	FORCEINLINE explicit Color()
	{
		// put these into the body for proper ordering with INTEL vs non-INTEL_BYTE_ORDER
		r = g = b = a = 0;
	}

	constexpr FORCEINLINE Color( u8 r, u8 g, u8 b, u8 a = 255 )
		: a(a), r(r), g(g), b(b)
	{}

	FORCEINLINE explicit Color( u32 InColor )
	{
		DWColor() = InColor;
	}

	bool Serialize(class Archive& ar, const char* name);

	// Operators.
	FORCEINLINE bool operator==( const Color &other ) const
	{
		return DWColor() == other.DWColor();
	}

	FORCEINLINE bool operator!=( const Color& other ) const
	{
		return DWColor() != other.DWColor();
	}

	FORCEINLINE void operator+=(const Color& other)
	{
		r = (u8) Math::Min((i32) r + (i32) other.r, 255);
		g = (u8) Math::Min((i32) g + (i32) other.g, 255);
		b = (u8) Math::Min((i32) b + (i32) other.b, 255);
		a = (u8) Math::Min((i32) a + (i32) other.a, 255);
	}

	LinearColor FromRGBE() const;

	/**
	 * Makes a random but quite nice color.
	 */
	static Color MakeRandomColor();

	/**
	 * Makes a color red->green with the passed in scalar (e.g. 0 is red, 1 is green)
	 */
	static Color MakeRedToGreenColorFromScalar(float scalar);

	/**
	* Converts temperature in Kelvins of a black body radiator to RGB chromaticism.
	*/
	static Color MakeFromColorTemperature( float Temp );

	/**
	 *	@return a new FColor based of this color with the new alpha value.
	 *	Usage: const FColor& MyColor = FColorList::Green.WithAlpha(128);
	 */
	Color WithAlpha( u8 alpha ) const
	{
		return Color( r, g, b, alpha );
	}

	/**
	 * Reinterprets the color as a linear color.
	 *
	 * @return The linear color representation.
	 */
	FORCEINLINE LinearColor ReinterpretAsLinear() const
	{
		return LinearColor(r / 255.f, g / 255.f, b / 255.f, a / 255.f);
	}

	/**
	 * Converts this color value to a hexadecimal string.
	 *
	 * The format of the string is RRGGBBAA.
	 *
	 * @return Hexadecimal string.
	 * @see FromHex, ToString
	 */
	FORCEINLINE String ToHex() const
	{
		String str;
		str.sprintf(TX("%02X%02X%02X%02X"), r, g, b, a);
		return eastl::move(str);
	}

	/**
	 * Converts this color value to a string.
	 *
	 * @return The string representation.
	 * @see ToHex
	 */
	FORCEINLINE String ToString() const
	{
		String str;
		str.sprintf(TX("(R=%i,G=%i,B=%i,A=%i)"), r, g, b, a);
		return str;
	}

	/**
	 * Gets the color in a packed uint32 format packed in the order ARGB.
	 */
	FORCEINLINE u32 ToPackedARGB() const
	{
		return ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | ( b << 0 );
	}

	/**
	 * Gets the color in a packed uint32 format packed in the order ABGR.
	 */
	FORCEINLINE u32 ToPackedABGR() const
	{
		return ( a << 24 ) | ( b << 16 ) | ( g << 8 ) | ( r << 0 );
	}

	/**
	 * Gets the color in a packed uint32 format packed in the order RGBA.
	 */
	FORCEINLINE u32 ToPackedRGBA() const
	{
		return ( r << 24 ) | ( g << 16 ) | ( g << 8 ) | ( a << 0 );
	}

	/**
	 * Gets the color in a packed uint32 format packed in the order BGRA.
	 */
	FORCEINLINE u32 ToPackedBGRA() const
	{
		return ( b << 24 ) | ( g << 16 ) | ( r << 8 ) | ( a << 0 );
	}

#if WITH_EDITOR
	static class Class* GetDetailsWidgetClass();
#endif

	/** Some pre-initialized colors, useful for debug code */
	static const Color White;
	static const Color Black;
	static const Color Transparent;
	static const Color Red;
	static const Color Green;
	static const Color Blue;
	static const Color Yellow;
	static const Color Cyan;
	static const Color Magenta;
	static const Color Orange;
	static const Color Purple;
	static const Color Turquoise;
	static const Color Silver;
	static const Color Emerald;

private:
	/**
	 * Please use .ToFColor(true) on FLinearColor if you wish to convert from FLinearColor to FColor,
	 * with proper sRGB conversion applied.
	 *
	 * Note: Do not implement or make public.  We don't want people needlessly and implicitly converting between
	 * FLinearColor and FColor.  It's not a free conversion.
	 */
	explicit Color(const LinearColor& LinearColor);
};


FORCEINLINE u32 GetTypeHash( const Color& Color )
{
	return Color.DWColor();
}


/*FORCEINLINE uint32 GetTypeHash( const LinearColor& LinearColor )
{
	// Note: this assumes there's no padding in FLinearColor that could contain uncompared data.
	return FCrc::MemCrc_DEPRECATED(&LinearColor, sizeof(LinearColor));
}*/


/** Computes a brightness and a fixed point color from a floating point color. */
extern void ComputeAndFixedColorAndIntensity(const LinearColor& InLinearColor,Color& OutColor,float& OutIntensity);

// These act like a POD
EASTL_DECLARE_IS_POD(Color, true);
EASTL_DECLARE_IS_POD(LinearColor, true);

DEFINE_CLASS_TRAITS(Color,
	HasCustomSerialize = true,
	HasDetailsWidget = true
);
DECLARE_REFLECTION_TYPE(Color);


/**
 * Helper struct for a 16 bit 565 color of a DXT1/3/5 block.
 */
struct FDXTColor565
{
	/** Blue component, 5 bit. */
	u16 B:5;

	/** Green component, 6 bit. */
	u16 G:6;

	/** Red component, 5 bit */
	u16 R:5;
};


/**
 * Helper struct for a 16 bit 565 color of a DXT1/3/5 block.
 */
struct FDXTColor16
{
	union
	{
		/** 565 Color */
		FDXTColor565 Color565;
		/** 16 bit entity representation for easy access. */
		u16 Value;
	};
};


/**
 * Structure encompassing single DXT1 block.
 */
struct FDXT1
{
	/** Color 0/1 */
	union
	{
		FDXTColor16 Color[2];
		u32 Colors;
	};
	/** Indices controlling how to blend colors. */
	u32 Indices;
};


/**
 * Structure encompassing single DXT5 block
 */
struct FDXT5
{
	/** Alpha component of DXT5 */
	u8	Alpha[8];
	/** DXT1 color component. */
	FDXT1	DXT1;
};


// Make DXT helpers act like PODs
EASTL_DECLARE_IS_POD(FDXT1, true);
EASTL_DECLARE_IS_POD(FDXT5, true);
EASTL_DECLARE_IS_POD(FDXTColor16, true);
EASTL_DECLARE_IS_POD(FDXTColor565, true);
