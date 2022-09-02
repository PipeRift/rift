// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "AST/Components/CFileRef.h"
#include "AST/Components/Tags/CChanged.h"
#include "AST/Components/Tags/CDirty.h"

#include <Pipe/ECS/Access.h>
#include <Pipe/ECS/Components/CChild.h>


namespace rift::AST
{
	using TransactionAccess = TAccessRef<TWrite<CChanged>, TWrite<CFileDirty>, CChild, CFileRef>;

	namespace Transactions
	{
		struct Transaction
		{
			bool active = false;
		};

		struct ScopedTransaction
		{
			bool active = false;

			ScopedTransaction() {}
			ScopedTransaction(const TransactionAccess& access, TSpan<const Id> entityIds);
			ScopedTransaction(ScopedTransaction&& other) noexcept;
			~ScopedTransaction();
		};


		bool PreChange(const TransactionAccess& access, TSpan<const Id> entityIds);
		void PostChange();
	}    // namespace Transactions
}    // namespace rift::AST

#define ScopedChange(access, entityIds) \
	rift::AST::Transactions::ScopedTransaction _transaction{access, entityIds};
