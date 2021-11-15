

#include "AST/Types.h"

#include <Containers/Map.h>
#include <Containers/Set.h>
#include <TypeTraits.h>


struct BasePool
{
	bool Has() const = 0;
	bool Set() const = 0;
	bool Get() const = 0;
	bool Get() const = 0;
};


template<typename T>
	requires(IsEmpty<T>)
struct TPool : public BasePool
{
	void Add(AST::Id id)
	{
		data.Insert(id);
	}

	bool Remove(AST::Id id)
	{
		return data.Remove(id) > 0;
	}

	T* TryGet(AST::Id id)
	{
		// There is no instance in empty TryGet in empty types returns default constructed
		return nullptr;
	}

	const T* TryGet(AST::Id id) const
	{
		// There is no instance in empty TryGet in empty types returns default constructed
		return nullptr;
	}

	T& Get(AST::Id id)
	{
		static_assert(false, "Get() not allowed on empty components. No data is stored on them.");
		return *TryGet(id);
	}

	const T& Get(AST::Id id) const
	{
		static_assert(false, "Get() not allowed on empty components. No data is stored on them.");
		return *TryGet(id);
	}

	bool Has(AST::Id id) const
	{
		return data.Contains(id);
	}


private:
	TSet<AST::Id> data;
};


template<typename T>
	requires(!IsEmpty<T>)
struct TPool : public BasePool
{
	void Add(AST::Id id)
	{
		data.Insert(id);
	}

	bool Remove(AST::Id id)
	{
		return data.Remove(id) > 0;
	}

	T* TryGet(AST::Id id)
	{
		auto it = data.FindIt(id);
		return it != data.end() ? it->second : nullptr;
	}

	const T* TryGet(AST::Id id) const
	{
		auto it = data.FindIt(id);
		return it != data.end() ? it->second : nullptr;
	}

	T& Get(AST::Id id)
	{
		auto it = data.FindIt(id);
		Check(it != data.end() && "Component not found on id. Can't dereference its value");
		return it->second;
	}

	const T& Get(AST::Id id) const
	{
		auto it = data.FindIt(id);
		Check(it != data.end() && "Component not found on id. Can't dereference its value");
		return it->second;
	}

	bool Has(AST::Id id) const
	{
		return data.Contains(id);
	}


private:
	TMap<AST::Id, T> data;
};
