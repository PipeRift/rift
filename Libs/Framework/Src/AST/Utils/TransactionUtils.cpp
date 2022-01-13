// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Utils/TransactionUtils.h"

#include "AST/Components/CType.h"
#include "AST/Components/Tags/CChanged.h"
#include "AST/Components/Tags/CTypeDirty.h"
#include "AST/Utils/Hierarchy.h"


namespace Rift::AST::Transactions
{
	void Change(AST::Tree& ast, Id entityId)
	{
		// TODO: Queue a transaction and apply out of scope or manually
		ast.Add<CChanged>(entityId);

		auto types = ast.Filter<CType>();
		Id typeId  = Hierarchy::FindParent(ast, entityId, [&types](Id parentId) {
            return types.Has(parentId);
		 });
		if (!IsNone(typeId))
		{
			ast.Add<CTypeDirty>(typeId);
		}
	}
}    // namespace Rift::AST::Transactions
