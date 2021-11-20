

#include "AST/Types.h"

#include <Containers/BitArray.h>
#include <Containers/Map.h>
#include <Containers/Set.h>
#include <Memory/UniquePtr.h>
#include <TypeTraits.h>


namespace Rift::AST
{
	struct ComponentList
	{
	}

	struct Archetype
	{
		using Index = IdTraits<Id>::Index;

	private:
		TArray<Index> sortedEntities;
		TArray<Refl::TypeId> componentTypes;
		TArray<void*> componentLists;


	public:
		Archetype(TArray<Refl::TypeId>&& componentTypes) : componentTypes{Move(componentTypes)}
		{
			componentTypes.Sort();
		}


		bool Has(Id id) const
		{
			return GetRow(id) != NO_INDEX;
		}

		template<typename T>
		T* Get(Id id) const
		{
			GetTypeId<T>();
			const i32 id = GetRow(id);
			if (id != NO_INDEX) {}
			return;
		}

		i32 Size() const
		{
			return sortedEntities.Size();
		}

		i32 GetRow(AST::Id id) const
		{
			return sortedEntities.FindSortedEqual(IdTraits<Id>::GetIndex(id));
		}
	};

	struct TestRegistry
	{
		TArray<Archetype*> archetypes;
		TSet<Refl::TypeId, TArray<Archetype*>> componentToArchetypes;


		template<typename... C>
		AST::Id Create(const C&...)
		{
			auto* arch = new Archetype();
			archetypes.Add(arch);
		}

		void* Add(Refl::TypeId componentId)
		{
			auto* arch = new Archetype();
			archetypes.Add(arch);
		}
		AST::Id Remove(Refl::TypeId componentId)
		{
			auto* arch = new Archetype();
			archetypes.Add(arch);
		}

		template<typename C>
		bool Has(AST::Id id) const
		{}
	}
}    // namespace Rift::AST
