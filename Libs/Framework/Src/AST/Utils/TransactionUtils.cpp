// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Utils/TransactionUtils.h"

#include "AST/Components/CType.h"
#include "AST/Filtering.h"
#include "AST/Utils/Hierarchy.h"


namespace Rift::AST::Transactions
{
	// Transaction being recorded
	static Transaction activeTransaction = {};

	ScopedTransaction::ScopedTransaction(const TransactionAccess& access, TSpan<const Id> entityIds)
	{
		active = PreChange(access, entityIds);
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

	bool PreChange(const TransactionAccess& access, TSpan<const Id> entityIds)
	{
		if (!EnsureMsg(!activeTransaction.access,
		        "Tried to record a transaction while another is already being recorded"))
		{
			return false;
		}

		activeTransaction = Transaction{&access};
		activeTransaction.entityIds.Append(entityIds);
		// TODO: Capture AST state
		return true;
	}

	void PostChange()
	{
		if (!EnsureMsg(
		        activeTransaction.access, "Cant finish a transaction while none is being recorded"))
		{
			return;
		}

		const TransactionAccess& access = *activeTransaction.access;

		access.Add<CChanged>(activeTransaction.entityIds);

		// Mark files dirty
		TArray<Id> fileIds;
		Hierarchy::FindParents(access, activeTransaction.entityIds, fileIds,
		    [&access](Id parentId) {
			return access.Has<CFileRef>(parentId);
		});

		// Transaction ids can also be files. FindParents doesn't consider them, so we merge it
		fileIds.Append(activeTransaction.entityIds);
		AST::RemoveIfNot<CFileRef>(access, fileIds);
		if (!fileIds.IsEmpty())
		{
			access.Add<CFileDirty>(fileIds);
		}

		activeTransaction = {};
	}
}    // namespace Rift::AST::Transactions
