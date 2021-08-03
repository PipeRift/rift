// Copyright 2015-2021 Piperift - All rights reserved

#include "Assets/TypeAsset.h"

#include "AST/Components/CChild.h"
#include "AST/Components/CClassDecl.h"
#include "AST/Components/CFileRef.h"
#include "AST/Components/CFunctionLibraryDecl.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CParent.h"
#include "AST/Components/CStructDecl.h"
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

		// auto project = RiftContext::GetProject();
		// Check(project);
		if (ct.IsReading())
		{
			// ASTReadContext astCt{ct.GetRead(), RiftContext::AST()};
			// astCt.SerializeRoot(declaration);
		}
		else
		{
			// ASTWriteContext astCt{ct.GetWrite(), RiftContext::AST()};
			// astCt.SerializeRoot(declaration);
		}
	}

	void TypeAsset::InitializeDeclaration(AST::Tree& ast)
	{
		declaration = ast.Create();
		switch (type)
		{
			case Type::Class:
				ast.Add<CClassDecl>(declaration);
				break;
			case Type::Struct:
				ast.Add<CStructDecl>(declaration);
				break;
			case Type::FunctionLibrary:
				ast.Add<CFunctionLibraryDecl>(declaration);
				break;
		}
		// TODO: Use file name
		ast.Add<CIdentifier>(declaration, GetMetaPath());
		ast.Add<CParent>(declaration);

		auto& assetRef = ast.Add<CFileRef>(declaration);
		assetRef.path  = Paths::FromString(GetInfo().GetStrPath());
	}
}    // namespace Rift
