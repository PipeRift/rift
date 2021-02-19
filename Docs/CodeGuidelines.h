// Copyright 2015-2021 Piperift - All rights reserved
// Coding standards for Rift and Rift-Core

// [header.format] Use .h for header files and .cpp for source files

// [header.guard] include guard
#pragma once

// [basic.language] Use US English for all names and comments
//  BAD  -> Color SpecialisedColour;
//  GOOD -> Color SpecializedColor;

// [header.incl.order]
//  Include order of files should be:
//  1) Associated header (only in .cpp files)
//  2) Same library includes
//  3) Core includes
//  4) External includes

// [header.forward-decl] List as many forward declarations as possible
// If only used once, they can also be written in that line of code.
struct SomeStruct;


// [header.namespace.name] Namespaces use UpperCamelCase
namespace CodingStandards
{};
// [header.type-spacing] Leave 2 new lines between classes, structs, namespaces, forward
// declarations, includes, etc


// [header.struct.name] Structs use UpperCamelCase
struct OneStruct
{};


// [header.class.name] Classes use UpperCamelCase
class OneClass
{
	// [header.class.order.variables] Variables go before functions.
	// They are ordered by public, protected and then private.
public:
	bool publicVar = true;

private:
	i32 anotherVar = true;


	// [header.section-spacing] At least one new line must be provided between access modifiers.
	// Two lines must be provided between variables and functions
public:
	// [header.class.order.functions]
	// Functions go after, following access order like variables.
	void Something() {}
};


// [header.class.name] templates have a T preffix
template <typename T>
class TSomeTemplate
{};


void Something()
{
	// [header.types.integers] Use rift integer types
	u8 a  = 0;
	i16 b = 0;
	u16 c = 0;
	i32 d = 0;
	u32 e = 0;
	i64 f = 0;
	u64 g = 0;

	// [header.types.booleans] Booleans do NOT have a b preffix
	bool alive = true;
}