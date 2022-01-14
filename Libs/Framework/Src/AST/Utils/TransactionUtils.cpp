// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Utils/TransactionUtils.h"

#include "AST/Components/CType.h"
#include "AST/Components/Tags/CChanged.h"
#include "AST/Components/Tags/CTypeDirty.h"
#include "AST/Utils/Hierarchy.h"


namespace Rift::AST::Transactions
{
	// Transaction being recorded
	static Transaction activeTransaction = {};

	ScopedTransaction::ScopedTransaction(Tree& ast, TSpan<Id> entityIds)
	{
		active = PreChange(ast, entityIds);
	}
	ScopedTransaction::ScopedTransaction(ScopedTransaction&& other)
	{
		active       = other.active;
		other.active = false;
	}
	ScopedTransaction::~ScopedTransaction()
	{
		if (active)
		{
			PostChange();
		}
	}

	bool PreChange(Tree& ast, TSpan<Id> entityIds)
	{
		if (!EnsureMsg(!activeTransaction.ast,
		        "Tried to record a transaction while another is already being recorded"))
		{
			return false;
		}

		activeTransaction = Transaction{&ast};
		activeTransaction.entityIds.Append(entityIds);
		// TODO: Capture AST state
		return true;
	}

	void PostChange()
	{
		if (!EnsureMsg(
		        activeTransaction.ast, "Cant finish a transaction while none is being recorded"))
		{
			return;
		}

		Tree& ast = *activeTransaction.ast;

		ast.Add<CChanged>(activeTransaction.entityIds);

		// Mark types dirty
		auto types = ast.Filter<CType>();
		TArray<Id> typeIds;
		Hierarchy::FindParents(ast, activeTransaction.entityIds, typeIds, [&types](Id parentId) {
			return types.Has(parentId);
		});

		// Transition ids can also be types. FindParents doesnt consider them
		typeIds.Append(activeTransaction.entityIds);
		types.FilterIds(typeIds);
		if (!typeIds.IsEmpty())
		{
			ast.Add<CTypeDirty>(typeIds);
		}

		activeTransaction = {};
	}
}    // namespace Rift::AST::Transactions
