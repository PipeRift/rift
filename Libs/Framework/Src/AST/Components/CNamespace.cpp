// Copyright 2015-2023 Piperift - All rights reserved

#include "AST/Components/CNamespace.h"


namespace rift::AST
{
	Namespace::Namespace(p::StringView value)
	{
		p::i32 size          = 0;
		const p::TChar* last = value.data() + value.size();
		const p::TChar* curr = value.data();

		if (curr != last && *curr == '@')
			++curr;

		const p::TChar* scopeStart = curr;
		while (curr != last && size < scopeCount)
		{
			if (*curr == '.')
			{
				scopes[size] = p::Tag{
				    p::StringView{scopeStart, curr}
                };
				scopeStart = curr + 1;
				++size;
			}
			++curr;
		}

		if (scopeStart < curr)    // Add last
		{
			scopes[size] = p::Tag{
			    p::StringView{scopeStart, curr}
            };
		}
	}

	bool Namespace::Equals(const Namespace& other) const
	{
		for (p::i32 i = 0; i < scopeCount; ++i)
		{
			const p::Tag scope      = scopes[i];
			const p::Tag otherScope = other.scopes[i];
			if (scope != otherScope)
			{
				return false;
			}
			else if (scope.IsNone() && otherScope.IsNone())
			{
				return true;
			}
		}
		return true;
	}

	bool Namespace::IsEmpty() const
	{
		return First().IsNone();
	}

	p::i32 Namespace::Size() const
	{
		p::i32 size = 0;
		while (size < scopeCount && !scopes[size].IsNone())
		{
			++size;
		}
		return size;
	}

	p::String Namespace::ToString(bool isLocal) const
	{
		p::String ns;
		if (!isLocal)
		{
			ns.append("@");
			const p::Tag firstScope = scopes[0];
			if (!firstScope.IsNone())
			{
				ns.append(firstScope.AsString());
				ns.append(".");
			}
		}
		for (p::i32 i = 1; i < scopeCount; ++i)
		{
			const p::Tag scope = scopes[i];
			if (scope.IsNone())
			{
				break;
			}
			ns.append(scope.AsString());
			ns.append(".");
		}

		if (!ns.empty())    // Remove last dot
		{
			ns.pop_back();
		}
		return p::Move(ns);
	}

	void Namespace::Read(p::Reader& ct)
	{
		p::u32 size = 0;
		ct.BeginArray(size);
		size = p::math::Min(size, p::u32(Namespace::scopeCount));
		for (p::u32 i = 0; i < size; ++i)
		{
			ct.Next(scopes[i]);
		}
	}

	void Namespace::Write(p::Writer& ct) const
	{
		p::u32 size = Size();
		ct.BeginArray(size);
		for (p::u32 i = 0; i < size; ++i)
		{
			ct.Next(scopes[i]);
		}
	}
}    // namespace rift::AST
