// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <AST/Tree.h>
#include <Assets/AssetData.h>
#include <CoreObject.h>
#include <Reflection/TypeFlags.h>


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

		Type type           = Type::None;
		AST::Id declaration = AST::NoId;


		// TODO: Eventually remove this or provide the type with access to the project
		void InitializeDeclaration(AST::Tree& ast);

	protected:
		void Serialize(Serl::CommonContext& ct) override;
	};
}    // namespace Rift
