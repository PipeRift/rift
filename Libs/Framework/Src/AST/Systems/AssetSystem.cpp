
#include "AST/Systems/AssetSystem.h"
#include "AST/Tree.h"
#include "AST/Uniques/CAssetsUnique.h"


namespace Rift
{
	void AssetSystem::Init(AST::Tree& ast)
	{
		ast.SetUnique<CAssetsUnique>();
	}

	void AssetSystem::Tick(AST::Tree& ast)
	{
		auto& assets = ast.GetUnique<CAssetsUnique>();
	}
}    // namespace Rift
