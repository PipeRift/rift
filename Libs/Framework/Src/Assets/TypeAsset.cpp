// Copyright 2015-2021 Piperift - All rights reserved

#include "Assets/TypeAsset.h"

#include "AST/Components/CChildren.h"
#include "AST/Components/CClassDecl.h"
#include "AST/Components/CFunctionLibraryDecl.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CParent.h"
#include "AST/Components/CStructDecl.h"
#include "AST/Components/CTypeAssetRef.h"
#include "AST/Serialization.h"
#include "RiftContext.h"

#include <Reflection/Static/EnumType.h>


namespace Rift
{
	void TypeAsset::Serialize(Serl::CommonContext& ct)
	{
		Super::Serialize(ct);

		ct.PushAddFlags(Serl::WriteFlags_CacheStringValues);
		if (ct.IsReading())
		{
			String typeStr;
			ct.Next("type", typeStr);
			type = Refl::GetEnumValue<Type>(typeStr).value_or(Type::None);
		}
		else
		{
			String typeStr{Refl::GetEnumName(type)};
			ct.Next("type", typeStr);
		}
		ct.PopFlags();

		auto project = RiftContext::GetProject();
		Check(project);
		if (ct.IsReading())
		{
			ASTReadContext astCt{ct.GetRead(), project->GetAST()};
			astCt.SerializeRoot(declaration);
		}
		else
		{
			ASTWriteContext astCt{ct.GetWrite(), project->GetAST()};
			astCt.SerializeRoot(declaration);
		}
	}

	void TypeAsset::InitializeDeclaration(AST::Tree& ast)
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
