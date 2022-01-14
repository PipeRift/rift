// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"


namespace Rift::AST::Transactions
{
	struct Transaction
	{
		Tree* ast = nullptr;
		TArray<Id> entityIds;
	};

	struct ScopedTransaction
	{
		bool active = false;

		ScopedTransaction() {}
		ScopedTransaction(Tree& ast, TSpan<Id> entityIds);
		ScopedTransaction(ScopedTransaction&& other);
		~ScopedTransaction();
	};


	bool PreChange(Tree& ast, TSpan<Id> entityIds);
	void PostChange();
}    // namespace Rift::AST::Transactions

#define ScopedChange(ast, entityIds) \
	Rift::AST::Transactions::ScopedTransaction __transaction{ast, entityIds};
