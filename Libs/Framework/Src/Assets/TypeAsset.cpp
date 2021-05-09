// Copyright 2015-2021 Piperift - All rights reserved

#include "Assets/TypeAsset.h"
#include "Lang/CChildren.h"
#include "Lang/CParent.h"
#include "Lang/Declarations/CClassDecl.h"
#include "Lang/Declarations/CFunctionLibraryDecl.h"
#include "Lang/Declarations/CStructDecl.h"
#include "Lang/Identifiers/CIdentifier.h"
#include "Lang/Misc/CTypeAssetRef.h"

#include <Lang/AST/ASTArchive.h>
#include <Reflection/Static/EnumType.h>


namespace Rift
{
	bool TypeAsset::Serialize(Archive& ar)
	{
		Super::Serialize(ar);

		if (ar.IsLoading())
		{
			String typeStr;
			ar("type", typeStr);
			type = Refl::GetEnumValue<Type>(typeStr).value_or(Type::None);
		}
		else
		{
			String typeValue = String(Refl::GetEnumName(type));
			ar("type", typeValue);
		}

		// ASTArchive astArchive{ar};
		// astArchive("declaration", declaration);

		return true;
	}

	void TypeAsset::InitializeDeclaration(AST::AbstractSyntaxTree& ast)
	{
		declaration = ast.Create();
		// TODO: Use file name
		ast.AddComponent<CIdentifier>(declaration, GetMetaPath());
		ast.AddComponent<CChildren>(declaration);
		auto& assetRef = ast.AddComponent<CTypeAssetRef>(declaration);
		assetRef.asset = {GetInfo()};

		switch (type)
		{
			case Type::Class:
				ast.AddComponent<CClassDecl>(declaration);
				break;
			case Type::Struct:
				ast.AddComponent<CStructDecl>(declaration);
				break;
			case Type::FunctionLibrary:
				ast.AddComponent<CFunctionLibraryDecl>(declaration);
				break;
		}
	}
}    // namespace Rift
