// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "UI/Style.h"
#include "UI/UIImgui.h"
#include "UI/Widgets.h"

#include <PipeReflect.h>


namespace rift::UI
{
	struct ValueHandle
	{
	protected:
		void* container = nullptr;
		p::TypeId containerType;
		const p::TypeProperty* property = nullptr;
		p::i32 index                    = p::NO_INDEX;


	public:
		ValueHandle(void* container, p::TypeId containerType)
		    : container{container}, containerType{containerType}
		{}
		ValueHandle(void* container, p::TypeId containerType, const p::TypeProperty& property)
		    : container{container}, containerType{containerType}, property{&property}
		{}
		ValueHandle(void* data, const p::TypeProperty* property, p::i32 index)
		    : container{data}, property{property}, index{index}
		{
			// P_Check(GetArrayProperty());
		}
		ValueHandle(const ValueHandle& container, p::i32 index)
		    : ValueHandle(container.GetPtr(), container.GetProperty(), index)
		{}

		ValueHandle(const ValueHandle& other)            = default;
		ValueHandle& operator=(const ValueHandle& other) = default;

		const p::TypeProperty* GetProperty() const
		{
			return property;
		}

		bool IsArray() const
		{
			return !IsArrayItem() && property && property->HasFlag(p::PF_Array);
		}
		bool IsArrayItem() const
		{
			return index != p::NO_INDEX;
		}

		// const ArrayProperty* GetArrayProperty() const
		//{
		//	if (property && property->HasFlag(PF_Array))
		//	{
		//		return static_cast<const ArrayProperty*>(property);
		//	}
		//	return nullptr;
		// }

		virtual void GetDisplayName(p::String& name) const
		{
			if (index != p::NO_INDEX)
			{
				p::FormatTo(name, "{}", index);
			}
			else if (property)
			{
				name.append(p::Strings::ToSentenceCase(property->name.AsString()));
			}
		}

		p::TypeId GetType() const
		{
			return containerType;
		}

		void* GetContainer() const
		{
			P_Check(index != p::NO_INDEX);
			return container;
		}

		p::i32 GetIndex() const
		{
			return index;
		}

		void* GetPtr() const
		{
			// const auto* arrayProp = GetArrayProperty();
			// if (arrayProp && index != NO_INDEX)
			//{
			//	return arrayProp->GetItem(container, index);
			// }
			return container;
		}

		bool IsValid() const
		{
			return container != nullptr;
		}

		operator bool() const
		{
			return IsValid();
		}
	};


	using CustomKeyValue = p::TFunction<void(p::StringView label, void* data, p::TypeId type)>;


	void RegisterCustomInspection(p::TypeId type, const CustomKeyValue& custom);
	template<typename T>
	void RegisterCustomInspection(const CustomKeyValue& custom)
	{
		RegisterCustomInspection(p::GetTypeId<T>(), custom);
	}

	void DrawEnumValue(void* data, p::TypeId type);
	void DrawNativeValue(void* data, p::TypeId type);
	void DrawKeyValue(p::StringView label, void* data, p::TypeId type);

	void InspectProperty(const ValueHandle& handle);
	void InspectChildrenProperties(const ValueHandle& handle);

	inline void InspectStruct(void* data, p::TypeId type)
	{
		if (p::HasTypeFlags(type, p::TF_Struct))
		{
			InspectChildrenProperties({data, type});
		}
	}
	template<typename T>
	inline void InspectStruct(T* data) requires(p::IsStructOrClass<T>)
	{
		InspectStruct(data, p::GetTypeId<T>());
	}

	bool BeginCategory(p::StringView name, bool isLeaf);
	void EndCategory();

	bool BeginInspector(const char* name, p::v2 size = p::v2{0.f, 0.f});
	void EndInspector();

	void RegisterCoreKeyValueInspections();
}    // namespace rift::UI
