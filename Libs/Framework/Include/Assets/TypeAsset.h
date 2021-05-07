// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Assets/AssetData.h>
#include <CoreObject.h>
#include <Lang/AST.h>
#include <Reflection/ClassTraits.h>


namespace Rift
{
	class TypeAsset : public AssetData
	{
		CLASS(TypeAsset, AssetData)

	public:
		enum class Type : u8
		{
			None,
			Class,
			Struct,
			FunctionLibrary
		};

		Type type = Type::None;
		AST::Id declaration;


		// FIX: Not being called
		void Serialize(Archive& ar, StringView name);

		// TODO: Eventually remove this or provide the type with access to the project
		void InitializeDeclaration(AST::AbstractSyntaxTree& ast);
	};

	DEFINE_CLASS_TRAITS(TypeAsset, HasCustomSerialize = true);
}    // namespace Rift
