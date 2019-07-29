// Copyright 2015-2019 Piperift - All rights reserved

#include "Archive.h"

void Archive::Serialize(const char* name, v2& val)
{
	BeginObject(name);
	{
		Serialize(TX("x"), val.x);
		Serialize(TX("y"), val.y);
	}
	EndObject();
}

void Archive::Serialize(const char* name, v2_u32& val)
{
	BeginObject(name);
	{
		Serialize(TX("x"), val.x);
		Serialize(TX("y"), val.y);
	}
	EndObject();
}

void Archive::Serialize(const char* name, v3& val)
{
	BeginObject(name);
	{
		Serialize(TX("x"), val.x);
		Serialize(TX("y"), val.y);
		Serialize(TX("z"), val.z);
	}
	EndObject();
}

void Archive::Serialize(const char* name, Quat& val)
{
	BeginObject(name);
	{
		Serialize(TX("x"), val.x);
		Serialize(TX("y"), val.y);
		Serialize(TX("z"), val.z);
		Serialize(TX("w"), val.w);
	}
	EndObject();
}

void JsonArchive::Serialize(const char* name, bool& val)
{
	if (IsLoading())
	{
		const Json& field = Data()[name];
		if (field.is_boolean())
		{
			val = field.get<bool>();
		}
	}
	else
	{
		Data()[name] = val;
	}
}

void JsonArchive::Serialize(const char* name, u8& val)
{
	if (IsLoading())
	{
		const Json& field = Data()[name];
		if (field.is_number_unsigned())
		{
			val = field.get<u8>();
		}
	}
	else
	{
		Data()[name] = val;
	}
}

void JsonArchive::Serialize(const char* name, i32& val)
{
	if (IsLoading())
	{
		const Json& field = Data()[name];
		if (field.is_number_unsigned())
		{
			val = field.get<i32>();
		}
	}
	else
	{
		Data()[name] = val;
	}
}

void JsonArchive::Serialize(const char* name, u32& val)
{
	if (IsLoading())
	{
		const Json& field = Data()[name];
		if (field.is_number_unsigned())
		{
			val = field.get<u32>();
		}
	}
	else
	{
		Data()[name] = val;
	}
}

void JsonArchive::Serialize(const char* name, float& val)
{
	if (IsLoading())
	{
		const Json& field = Data()[name];
		if (field.is_number())
		{
			val = field.get<float>();
		}
	}
	else
	{
		Data()[name] = val;
	}
}

void JsonArchive::Serialize(const char* name, String& val)
{
	if (IsLoading())
	{
		const Json& field = Data()[name];
		if (field.is_string())
		{
			val = field.get<String>();
		}
	}
	else
	{
		Data()[name] = val;
	}
}

void JsonArchive::Serialize(const char* name, Json& val)
{
	if (IsLoading())
	{
		val = Data()[name];
	}
	else
	{
		Data()[name] = val;
	}
}
