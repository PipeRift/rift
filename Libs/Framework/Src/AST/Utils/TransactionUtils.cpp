// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Utils/TransactionUtils.h"

#include "AST/Components/CType.h"
#include "AST/Components/Tags/CChanged.h"
#include "AST/Components/Tags/CDirty.h"
#include "AST/Filtering.h"
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

		// Mark files dirty
		TAccess<CFileRef> files{ast};
		TArray<Id> fileIds;
		Hierarchy::FindParents(ast, activeTransaction.entityIds, fileIds, [&files](Id parentId) {
			return files.Has<CFileRef>(parentId);
		});

		// Transaction ids can also be files. FindParents doesn't consider them, so we merge it
		fileIds.Append(activeTransaction.entityIds);
		AST::RemoveIfNot<CFileRef>(files, fileIds);
		if (!fileIds.IsEmpty())
		{
			ast.Add<CFileDirty>(fileIds);
		}

		activeTransaction = {};
	}
}    // namespace Rift::AST::Transactions
