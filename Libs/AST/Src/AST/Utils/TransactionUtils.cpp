// Copyright 2015-2023 Piperift - All rights reserved

#include "AST/Utils/TransactionUtils.h"

#include "AST/Components/Declarations.h"

#include <PipeECS.h>


namespace rift::AST::Transactions
{
	// Transaction being recorded
	static Transaction gActiveTransaction = {};

	ScopedTransaction::ScopedTransaction(const TransactionAccess& access, TView<const Id> entityIds)
	{
		active = PreChange(access, entityIds);
	}
	ScopedTransaction::ScopedTransaction(ScopedTransaction&& other) noexcept
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

	bool PreChange(const TransactionAccess& access, TView<const Id> entityIds)
	{
		if (!EnsureMsg(!gActiveTransaction.active,
		        "Tried to record a transaction while another is already being recorded"))
		{
			return false;
		}

		gActiveTransaction = Transaction{true};

		// Mark files dirty
		TArray<Id> parentIds;
		p::GetAllIdParents(access, entityIds, parentIds);

		parentIds.Append(entityIds);
		access.AddN<CChanged>(parentIds);

		// Transaction ids can also be files. FindParents doesn't consider them, so we merge it
		ExcludeIdsWithout<CFileRef>(access, parentIds);
		if (!parentIds.IsEmpty())
		{
			access.AddN<CFileDirty>(parentIds);
		}

		// TODO: Capture AST state
		return true;
	}

	void PostChange()
	{
		if (EnsureMsg(gActiveTransaction.active,
		        "Cant finish a transaction while none is being recorded"))
		{
			gActiveTransaction = {};
		}
	}
}    // namespace rift::AST::Transactions
