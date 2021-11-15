

#include "AST/Types.h"

#include <Containers/Map.h>
#include <Containers/Set.h>
#include <Memory/UniquePtr.h>
#include <TypeTraits.h>


namespace Rift::AST
{
	struct BasePool
	{
		virtual ~BasePool() {}
	};


	template<typename T>
	struct TPool : public BasePool
	{
		void Add(Id id)
		{
			data.Insert(id);
		}

		bool Remove(Id id)
		{
			return data.Remove(id) > 0;
		}

		T* TryGet(Id id)
		{
			auto it = data.FindIt(id);
			return it != data.end() ? it->second : nullptr;
		}

		const T* TryGet(Id id) const
		{
			auto it = data.FindIt(id);
			return it != data.end() ? it->second : nullptr;
		}

		T& Get(Id id)
		{
			auto it = data.FindIt(id);
			Check(it != data.end() && "Component not found on id. Can't dereference its value");
			return it->second;
		}

		const T& Get(Id id) const
		{
			auto it = data.FindIt(id);
			Check(it != data.end() && "Component not found on id. Can't dereference its value");
			return it->second;
		}

		bool Has(Id id) const
		{
			return data.Contains(id);
		}


	private:
		TMap<Id, T> data;
	};


	/** TPool specification for empty component types.
	 * Empty components don't store component data, only existance
	 */
	template<typename T>
		requires(IsEmpty<T>())
	struct TPool<T> : public BasePool
	{
		void Add(Id id)
		{
			data.Insert(id);
		}

		bool Remove(Id id)
		{
			return data.Remove(id) > 0;
		}

		T* TryGet(Id id)
		{
			// There is no instance in empty TryGet in empty types returns default constructed
			return nullptr;
		}

		const T* TryGet(Id id) const
		{
			// There is no instance in empty TryGet in empty types returns default constructed
			return nullptr;
		}

		T& Get(Id id)
		{
			static_assert(
			    false, "Get() not allowed on empty components. No data is stored on them.");
			return *TryGet(id);
		}

		const T& Get(Id id) const
		{
			static_assert(
			    false, "Get() not allowed on empty components. No data is stored on them.");
			return *TryGet(id);
		}

		bool Has(Id id) const
		{
			return data.Contains(id);
		}


	private:
		TSet<Id> data;
	};


	struct PoolInstance
	{
		Refl::TypeId componentId;
		TUniquePtr<BasePool> instance;

		bool operator<(const PoolInstance& other) const
		{
			return componentId.GetId() < other.componentId.GetId();
		}
	};
}    // namespace Rift::AST
