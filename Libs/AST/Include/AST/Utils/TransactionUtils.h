// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "AST/Components/CFileRef.h"
#include "AST/Components/Tags/CChanged.h"
#include "AST/Components/Tags/CDirty.h"

#include <PipeECS.h>


namespace rift::ast
{
	using TransactionScope = p::TIdScopeRef<p::Writes<CChanged, CFileDirty>, CChild, CFileRef>;

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
			ScopedTransaction(const TransactionScope& scope, p::TView<const Id> entityIds);
			ScopedTransaction(ScopedTransaction&& other) noexcept;
			~ScopedTransaction();
		};


		bool PreChange(const TransactionScope& scope, p::TView<const Id> entityIds);
		void PostChange();
	}    // namespace Transactions
}    // namespace rift::ast

#define ScopedChange(scope, entityIds) \
	rift::ast::Transactions::ScopedTransaction _transaction{scope, entityIds};
