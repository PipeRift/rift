

#include "AST/Types.h"

#include <Containers/BitArray.h>
#include <Containers/Map.h>
#include <Containers/Set.h>
#include <Memory/UniquePtr.h>
#include <TypeTraits.h>


namespace Rift::AST
{
	struct BasePool
	{
		using Iterator      = TSet<Id>::Iterator;
		using ConstIterator = TSet<Id>::ConstIterator;


		BasePool() {}
		virtual ~BasePool() {}


		bool Has(Id id) const
		{
			return entities.Contains(id);
		}

		i32 Size() const
		{
			return entities.Size();
		}

		// Iterator functions
		Iterator begin()
		{
			return entities.begin();
		};
		ConstIterator begin() const
		{
			return entities.begin();
		};
		ConstIterator cbegin() const
		{
			return entities.cbegin();
		};

		Iterator end()
		{
			return entities.end();
		};
		ConstIterator end() const
		{
			return entities.end();
		};
		ConstIterator cend() const
		{
			return entities.cend();
		};

	private:
		TSet<Id> entities;
	};


	template<typename T>
	struct TPool : public BasePool
	{
		void Add(Id id)
		{
			entities.Insert(id);
			data.InsertDefaulted(id);
		}

		void Add(Id id, T&& value)
		{
			entities.Insert(id);
			data.Insert(id, Move(value));
		}

		void Add(Id id, const T& value)
		{
			entities.Insert(id);
			data.Insert(id, value);
		}

		bool Remove(Id id)
		{
			entities.Remove(id);
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
			entities.Insert(id);
		}

		bool Remove(Id id)
		{
			return entities.Remove(id) > 0;
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
