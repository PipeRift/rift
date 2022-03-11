// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "AST/Access.h"
#include "AST/Components/CChild.h"
#include "AST/Components/CFileRef.h"
#include "AST/Components/Tags/CChanged.h"
#include "AST/Components/Tags/CDirty.h"


namespace Rift
{
	using TransactionAccess = TAccess<TWrite<CChanged>, TWrite<CFileDirty>, CChild, CFileRef>;

	namespace AST::Transactions
	{
		struct Transaction
		{
			const TransactionAccess* access = nullptr;
			TArray<Id> entityIds;
		};

		struct ScopedTransaction
		{
			bool active = false;

			ScopedTransaction() {}
			ScopedTransaction(const TransactionAccess& access, TSpan<const Id> entityIds);
			ScopedTransaction(ScopedTransaction&& other);
			~ScopedTransaction();
		};


		bool PreChange(const TransactionAccess& access, TSpan<const Id> entityIds);
		void PostChange();
	}    // namespace AST::Transactions
}    // namespace Rift

#define ScopedChange(access, entityIds) \
	Rift::AST::Transactions::ScopedTransaction _transaction{access, entityIds};
