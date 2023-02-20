// Copyright 2015-2023 Piperift - All rights reserved

#include "Rift.h"

#include "Module.h"


namespace rift
{
	static p::TMap<p::ClassType*, p::TOwnPtr<class Module>> modules{};
	static p::TArray<RiftTypeDescriptor> riftTypes;
	static p::TArray<View> views{};


	void EnableModule(p::ClassType* type)
	{
		Check(Module::GetStaticType()->IsParentOf(type));

		if (!modules.Contains(type))
		{
			auto module = MakeOwned<Module>(type);
			module->DoLoad();
			modules.Insert(type, Move(module));
		}
	}

	void DisableModule(p::ClassType* type)
	{
		modules.Remove(type);
	}

	p::TPtr<Module> GetModule(p::ClassType* type)
	{
		if (auto* module = modules.Find(type))
		{
			return *module;
		}
		return {};
	}

	void RegisterView(View view)
	{
		views.Add(Move(view));
	}

	void RegisterRiftType(RiftTypeDescriptor&& descriptor)
	{
		riftTypes.FindOrAddSorted(Move(descriptor));
	}


	p::TSpan<const RiftTypeDescriptor> GetRiftTypes()
	{
		return riftTypes;
	}

	const RiftTypeDescriptor* FindRiftType(p::Tag typeId)
	{
		const i32 index = riftTypes.FindSortedEqual(typeId);
		return index != NO_INDEX ? riftTypes.Data() + index : nullptr;
	}

	const RiftTypeDescriptor* FindRiftType(p::TAccessRef<AST::CDeclType> access, AST::Id typeId)
	{
		if (const auto* type = access.TryGet<const AST::CDeclType>(typeId))
		{
			return FindRiftType(type->typeId);
		}
		return nullptr;
	}

};    // namespace rift
