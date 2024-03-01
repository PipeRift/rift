// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "AST/Components/CFileRef.h"
#include "AST/Components/Tags/CChanged.h"
#include "AST/Components/Tags/CDirty.h"

#include <PipeECS.h>


namespace rift::ast
{
	using TransactionAccess =
	    p::TAccessRef<p::TWrite<CChanged>, p::TWrite<CFileDirty>, CChild, CFileRef>;

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
			ScopedTransaction(const TransactionAccess& access, p::TView<const Id> entityIds);
			ScopedTransaction(ScopedTransaction&& other) noexcept;
			~ScopedTransaction();
		};


		bool PreChange(const TransactionAccess& access, p::TView<const Id> entityIds);
		void PostChange();
	}    // namespace Transactions
}    // namespace rift::ast

#define ScopedChange(access, entityIds) \
	rift::ast::Transactions::ScopedTransaction _transaction{access, entityIds};
