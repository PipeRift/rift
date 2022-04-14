// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Utils/TypeUtils.h"

#include "AST/Components/CDeclClass.h"
#include "AST/Components/CDeclFunction.h"
#include "AST/Components/CDeclFunctionLibrary.h"
#include "AST/Components/CDeclStruct.h"
#include "AST/Components/CDeclVariable.h"
#include "AST/Components/CFileRef.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CStmtOutputs.h"
#include "AST/Serialization.h"
#include "AST/Statics/STypes.h"
#include "AST/Utils/Hierarchy.h"

#include <AST/Utils/Paths.h>
#include <Files/Files.h>
#include <Misc/Checks.h>
#include <Profiler.h>
#include <Serialization/Formats/JsonFormat.h>


namespace Rift::Types
{
	void InitTypeFromCategory(AST::Tree& ast, AST::Id id, Type category)
	{
		if (auto* fileRef = ast.TryGet<CFileRef>(id))
		{
			String fileName = Paths::GetFilename(fileRef->path);
			fileName        = Strings::RemoveFromEnd(fileName, Paths::typeExtension);
			ast.Add<CType>(id, {Name{fileName}});
		}

		switch (category)
		{
			case Type::Class: ast.Add<CDeclClass>(id); break;
			case Type::Struct: ast.Add<CDeclStruct>(id); break;
			case Type::FunctionLibrary: ast.Add<CDeclFunctionLibrary>(id); break;
			default: break;
		}
	}

	Type GetCategory(AST::Tree& ast, AST::Id id)
	{
		if (ast.Has<CDeclStruct>(id))
		{
			return Type::Struct;
		}
		else if (ast.Has<CDeclClass>(id))
		{
			return Type::Class;
		}
		else if (ast.Has<CDeclFunctionLibrary>(id))
		{
			return Type::FunctionLibrary;
		}
		return Type::None;
	}

	AST::Id CreateType(AST::Tree& ast, Type type, Name name, const Path& path)
	{
		AST::Id id = ast.Create();
		if (!name.IsNone())
		{
			ast.Add<CIdentifier>(id, name);
		}
		if (!path.empty())
		{
			ast.Add<CFileRef>(id, path);
		}
		InitTypeFromCategory(ast, id, type);
		return id;
	}

	AST::Id AddVariable(AST::TypeRef type, Name name)
	{
		AST::Tree& ast = type.GetAST();

		AST::Id id = ast.Create();
		ast.Add<CIdentifier>(id, name);
		ast.Add<CDeclVariable, CParent>(id);

		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type, id);
		}
		return id;
	}

	AST::Id AddFunction(AST::TypeRef type, Name name)
	{
		AST::Tree& ast = type.GetAST();

		AST::Id id = ast.Create();
		ast.Add<CIdentifier>(id, name);
		ast.Add<CDeclFunction, CParent>(id);
		ast.Add<CStmtOutputs>(id);

		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type, id);
		}
		return id;
	}

	void RemoveTypes(TAccessRef<TWrite<CChild>, TWrite<CParent>, CFileRef> access,
	    TSpan<AST::Id> typeIds, bool removeFromDisk)
	{
		if (removeFromDisk)
		{
			for (AST::Id id : typeIds)
			{
				if (const auto* file = access.TryGet<const CFileRef>(id))
				{
					Files::Delete(file->path, true, false);
				}
			}
		}
		AST::Hierarchy::RemoveDeep(access, typeIds);
	}

	void Serialize(AST::Tree& ast, AST::Id id, String& data)
	{
		ZoneScoped;

		Serl::JsonFormatWriter writer{};
		AST::WriteContext ct{writer.GetContext(), ast, true};
		ct.BeginObject();
		ct.Next("type", GetCategory(ast, id));
		ct.SerializeEntity(id);
		data = writer.ToString();
	}

	void Deserialize(AST::Tree& ast, AST::Id id, const String& data)
	{
		ZoneScoped;

		Serl::JsonFormatReader reader{data};
		if (!reader.IsValid())
		{
			return;
		}

		AST::ReadContext ct{reader, ast};
		ct.BeginObject();

		Type category = Type::None;
		ct.Next("type", category);
		Types::InitTypeFromCategory(ast, id, category);

		ct.SerializeEntity(id);
	}


	AST::Id FindTypeByPath(AST::Tree& ast, const Path& path)
	{
		if (auto* types = ast.TryGetStatic<STypes>())
		{
			const Name pathName{Paths::ToString(path)};
			if (AST::Id* id = types->typesByPath.Find(pathName))
			{
				return *id;
			}
		}
		return AST::NoId;
	}

	bool IsClass(const AST::Tree& ast, AST::Id typeId)
	{
		return ast.Has<CDeclClass>(typeId);
	}

	bool IsStruct(const AST::Tree& ast, AST::Id typeId)
	{
		return ast.Has<CDeclStruct>(typeId);
	}

	bool IsFunctionLibrary(const AST::Tree& ast, AST::Id typeId)
	{
		return ast.Has<CDeclFunctionLibrary>(typeId);
	}

	bool CanContainVariables(const AST::Tree& ast, AST::Id typeId)
	{
		return ast.HasAny<CDeclClass, CDeclStruct>(typeId);
	}

	bool CanContainFunctions(const AST::Tree& ast, AST::Id typeId)
	{
		return ast.HasAny<CDeclClass, CDeclFunctionLibrary>(typeId);
	}
}    // namespace Rift::Types
