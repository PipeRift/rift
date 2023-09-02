// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <Pipe/Core/Tag.h>
#include <Pipe/Reflect/Struct.h>


namespace rift::AST
{
	struct CNamespace : public p::Struct
	{
		STRUCT(CNamespace, p::Struct)

		PROP(name);
		p::Tag name;


		CNamespace() = default;
		CNamespace(p::Tag name) : name(name) {}

		bool operator==(const CNamespace& other) const
		{
			return name == other.name;
		}
		bool operator==(p::Tag other) const
		{
			return name == other;
		}
	};

	inline void Read(p::Reader& r, CNamespace& val)
	{
		r.Serialize(val.name);
	}
	inline void Write(p::Writer& w, const CNamespace& val)
	{
		w.Serialize(val.name);
	}


	struct Namespace : public p::Struct
	{
		STRUCT(Namespace, p::Struct)

		static constexpr p::i32 scopeCount = 8;
		p::Tag scopes[scopeCount];    // TODO: Implement Inline arrays


		Namespace() = default;
		template<p::i32 M>
		Namespace(p::Tag scopes[M])
		    requires(M <= scopeCount)
		    : scopes{scopes}
		{}
		Namespace(p::StringView value);
		// Prevent initializer list from stealing string constructor
		Namespace(const p::String& value) : Namespace(p::StringView{value}) {}
		Namespace(std::initializer_list<p::Tag> values)
		{
			const p::i32 size = p::math::Min(p::i32(values.size()), scopeCount);
			for (p::i32 i = 0; i < size; ++i)
			{
				scopes[i] = *(values.begin() + i);
			}
		}

		bool Equals(const Namespace& other) const;
		bool IsEmpty() const;
		p::i32 Size() const;
		bool Contains(const Namespace& other) const;
		p::String ToString(bool isLocal = false) const;
		p::Tag& First()
		{
			return scopes[0];
		}
		p::Tag First() const
		{
			return scopes[0];
		}
		p::Tag& Last()
		{
			const p::i32 size      = Size();
			const p::i32 lastIndex = size > 0 ? (size - 1) : 0;    // Is Size is 0, last is first
			return scopes[lastIndex];
		}
		const p::Tag& Last() const
		{
			const p::i32 size      = Size();
			const p::i32 lastIndex = size > 0 ? (size - 1) : 0;    // Is Size is 0, last is first
			return scopes[lastIndex];
		}
		bool operator==(const Namespace& other) const
		{
			return Equals(other);
		}
		p::Tag operator[](p::i32 index) const
		{
			Check(index >= 0 && index < scopeCount);
			return scopes[index];
		}
		operator bool() const
		{
			return !IsEmpty();
		}

		p::Tag* begin()
		{
			return scopes;
		}
		const p::Tag* begin() const
		{
			return scopes;
		}
		p::Tag* end()
		{
			return scopes + Size();
		}
		const p::Tag* end() const
		{
			return scopes + Size();
		}

		void Read(p::Reader& ct);
		void Write(p::Writer& ct) const;
	};
}    // namespace rift::AST
