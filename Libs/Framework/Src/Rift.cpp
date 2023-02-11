// Copyright 2015-2023 Piperift - All rights reserved

#include "Rift.h"

#include "Module.h"


namespace rift
{
	static p::TMap<p::ClassType*, p::TOwnPtr<class Module>> modules{};
	static p::TArray<FileTypeDescriptor> fileTypes;
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

	void RegisterFileType(FileTypeDescriptor&& descriptor)
	{
		fileTypes.FindOrAddSorted(Move(descriptor));
	}


	p::TSpan<const FileTypeDescriptor> GetFileTypes()
	{
		return fileTypes;
	}

	const FileTypeDescriptor* FindFileType(p::Tag typeId)
	{
		const i32 index = fileTypes.FindSortedEqual(typeId);
		return index != NO_INDEX ? fileTypes.Data() + index : nullptr;
	}

};    // namespace rift
