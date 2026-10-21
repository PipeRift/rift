// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "AST/Systems/NameSystem.h"

#include "AST/Components/CGeneratedName.h"
#include "AST/Components/CNamespace.h"
#include "AST/Components/Declarations.h"
#include "AST/Components/Expressions.h"
#include "AST/Components/Statements.h"
#include "AST/Utils/Namespaces.h"

#include <Pipe/Core/Set.h>
#include <PipeECS.h>


namespace rift::ast::NameSystem
{
	namespace
	{
		/** Reserve a name in the scope, adding a numeric suffix when it is already taken. */
		void AssignName(Tree& ast, Id id, p::TSet<p::Tag>& used, bool reserveEnd = false)
		{
			const p::Tag base = GetName(ast, id);
			if (base.IsNone())
			{
				return;
			}

			const p::StringView baseName = base.AsString();
			p::String name;
			p::String endName;
			for (p::i32 suffix = 0;; ++suffix)
			{
				name.clear();
				if (suffix == 0)
				{
					name.append(baseName);
				}
				else
				{
					p::FormatTo(name, "{}_{}", baseName, suffix);
				}

				const p::Tag tag{name};
				if (used.Contains(tag))
				{
					continue;
				}

				if (reserveEnd)
				{
					endName.clear();
					p::FormatTo(endName, "{}_end", name);
					if (used.Contains(p::Tag{endName}))
					{
						continue;
					}
					used.Insert(p::Tag{endName});
				}

				used.Insert(tag);
				ast.GetOrAdd<CGeneratedName>(id).name = tag;
				return;
			}
		}

		void AssignVariables(Tree& ast, Id ownerId, p::TSet<p::Tag>& used)
		{
			p::TArray<Id> children;
			p::GetIdChildren(ast, ownerId, children);
			for (Id childId : children)
			{
				if (ast.Has<CDeclVariable>(childId))
				{
					AssignName(ast, childId, used);
				}
			}
		}

		void AddVariableNames(Tree& ast, Id ownerId, p::TSet<p::Tag>& used)
		{
			p::TArray<Id> children;
			p::GetIdChildren(ast, ownerId, children);
			for (Id childId : children)
			{
				if (ast.Has<CDeclVariable>(childId))
				{
					used.Insert(GetGeneratedName(ast, childId));
				}
			}
		}

		/** Collects loops in statement execution order so the first loop in a function
		 * gets the unsuffixed name. */
		void CollectForIds(p::TIdScopeRef<CStmtFor, CStmtOutput, CStmtOutputs> scope,
		    Id firstStmtId, p::TArray<Id>& forIds, p::TSet<Id>& visited)
		{
			Id current = firstStmtId;
			while (!IsNone(current) && !visited.Contains(current))
			{
				visited.Insert(current);

				if (const auto* outputs = scope.TryGet<const CStmtOutputs>(current))
				{
					if (scope.Has<CStmtFor>(current))
					{
						forIds.Add(current);
					}
					for (Id nextId : outputs->linkInputNodes)
					{
						CollectForIds(scope, nextId, forIds, visited);
					}
					return;
				}

				const auto* output = scope.TryGet<const CStmtOutput>(current);
				if (!output)
				{
					return;
				}
				current = output->linkInputNode;
			}
		}

		void AssignFunctionNames(
		    Tree& ast, Id functionId, p::TArray<Id>& forIds, p::TSet<p::Tag>& globalNames)
		{
			// Functions inherit the global scope and their type's member names
			p::TSet<p::Tag> functionNames = globalNames;
			const Id ownerId              = p::GetIdParent(ast, functionId);
			if (!IsNone(ownerId))
			{
				AddVariableNames(ast, ownerId, functionNames);
			}

			// Parameters are part of the function scope. Inputs are stored in CExprOutputs
			// and outputs in CExprInputs.
			if (const auto* inputs = ast.TryGet<const CExprOutputs>(functionId))
			{
				for (Id pinId : inputs->pinIds)
				{
					AssignName(ast, pinId, functionNames);
				}
			}
			if (const auto* outputs = ast.TryGet<const CExprInputs>(functionId))
			{
				for (Id pinId : outputs->pinIds)
				{
					AssignName(ast, pinId, functionNames);
				}
			}

			// Locals declared directly in the function share its scope
			AssignVariables(ast, functionId, functionNames);

			// Loop indices and their locals live in the function body
			p::TSet<p::Tag> bodyNames = functionNames;
			for (Id forId : forIds)
			{
				if (const auto* outputs = ast.TryGet<const CExprOutputs>(forId))
				{
					for (p::i32 i = 0; i < outputs->pinIds.Size(); ++i)
					{
						// The first expression output is the loop index. Its end bound is
						// generated as a companion name.
						AssignName(ast, outputs->pinIds[i], bodyNames, i == 0);
					}
				}
				AssignVariables(ast, forId, bodyNames);
			}
		}
	}    // namespace

	void AssignGeneratedNames(Tree& ast)
	{
		// Names are derived from the current graph, never serialized
		for (Id id : p::FindAllIdsWith<CGeneratedName>(ast))
		{
			ast.Remove<CGeneratedName>(id);
		}

		// Globals come first so parameters and locals can avoid them
		p::TSet<p::Tag> globalNames;
		for (Id typeId : p::FindAllIdsWith<CDeclStatic>(ast))
		{
			AssignVariables(ast, typeId, globalNames);
		}

		// Struct and class members have their own scope
		p::TArray<Id> memberTypeIds;
		memberTypeIds.Append(p::FindAllIdsWith<CDeclStruct>(ast));
		memberTypeIds.Append(p::FindAllIdsWith<CDeclClass>(ast));
		for (Id typeId : memberTypeIds)
		{
			p::TSet<p::Tag> memberNames;
			AssignVariables(ast, typeId, memberNames);
		}

		for (Id functionId : p::FindAllIdsWith<CDeclFunction>(ast))
		{
			p::TArray<Id> forIds;
			p::TSet<Id> visited;
			if (const auto* output = ast.TryGet<const CStmtOutput>(functionId))
			{
				CollectForIds(ast, output->linkInputNode, forIds, visited);
			}
			AssignFunctionNames(ast, functionId, forIds, globalNames);
		}
	}
}    // namespace rift::ast::NameSystem
