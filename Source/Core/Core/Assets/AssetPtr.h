// © 2019 Miguel Fernández Arce - All rights reserved

#pragma once

#include "CoreEngine.h"

#include <EASTL/type_traits.h>

#include "AssetInfo.h"
#include "AssetData.h"
#include "AssetManager.h"
//#include "Core/Serialization/Archive.h"
#include "../TypeTraits.h"


class BaseAssetPtr
{
	friend class AssetInfoPropertyWidget;

protected:

	AssetInfo info;
	mutable Ptr<AssetData> cachedAsset;

public:

	BaseAssetPtr() = default;
	BaseAssetPtr(AssetInfo info) : info{ info } {}

	AssetInfo GetInfo() const { return info; }
	void SetInfo(AssetInfo newInfo)
	{
		info = newInfo;
		cachedAsset.Reset();
	}

#if WITH_EDITOR
	static class Class* GetDetailsWidgetClass();
#endif
};


template<class T>
class TAssetPtr : public BaseAssetPtr
{
	static_assert(eastl::is_base_of<AssetData, T>::value, "AssetPtr type must inherit from AssetData");



public:

	using ItemType = T;

	TAssetPtr() : BaseAssetPtr() {}

	TAssetPtr(TAssetPtr&& other) { MoveFrom(MoveTemp(other)); }
	TAssetPtr(const TAssetPtr& other) { CopyFrom(other); }

	TAssetPtr(Name path) : BaseAssetPtr(path) {}
	TAssetPtr(const TCHAR* key) : TAssetPtr(Name{ key }) {}
	TAssetPtr(const AssetInfo& other) : AssetInfo(other) {}
	TAssetPtr(Ptr<ItemType> asset) {
		if (asset)
		{
			info = asset->GetInfo();
			cachedAsset = asset;
		}
	}

	TAssetPtr& operator=(TAssetPtr&& other)      { MoveFrom(MoveTemp(other)); return *this; }
	TAssetPtr& operator=(const TAssetPtr& other) { CopyFrom(other); return *this; }

	/**
	 * Tries to load this asset if it's not already
	 * @returns the loaded asset
	 */
	bool Load() const
	{
		if (IsNull())
		{
			return false;
		}
		else if (!IsValid())
		{
			if (auto manager = AssetManager::Get())
			{
				cachedAsset = manager->Load(info);
			}
		}
		return cachedAsset.IsValid();
	}

	/**
	 * Tries to load this asset and returns a new one if not found
	 * @returns the loaded asset
	 */
	bool LoadOrCreate() const
	{
		if (IsNull())
		{
			return false;
		}
		else if (!IsValid())
		{
			if (auto manager = AssetManager::Get())
			{
				cachedAsset = manager->LoadOrCreate(info, T::StaticClass());
			}
		}
		return cachedAsset.IsValid();
	}

	/**
	 * @returns the asset if it's valid and loaded
	 */
	Ptr<T> Get() const
	{
		if (!IsNull() && TryCacheAsset())
		{
			return cachedAsset.Cast<T>();
		}
		return {};
	}

	void Reset()
	{
		info = {};
		cachedAsset.Reset();
	}

	/** @returns true if this can never be pointed towards an asset */
	const bool IsNull() const { return info.IsNull(); }

	/** @returns true if this asset is loaded */
	const bool IsValid() const {
		return !IsNull() && TryCacheAsset();
	}

	inline const Name& GetPath()    const { return info.GetPath(); }
	inline const String& GetSPath() const { return info.GetSPath(); }

	operator bool() const { return IsValid(); };
	Ptr<T> operator*()  const { return Get(); }
	Ptr<T> operator->() const { return Get(); }

	FORCEINLINE bool Serialize(class Archive& ar, const char* inName)
	{
		return info.Serialize(ar, inName);
	}

private:

	bool TryCacheAsset() const
	{
		if (cachedAsset)
		{
			return true;
		}

		if (Ptr<AssetManager> manager = AssetManager::Get())
		{
			cachedAsset = manager->GetLoadedAsset(info);
		}
		return cachedAsset.IsValid();
	}

	void MoveFrom(TAssetPtr&& other)
	{
		eastl::swap(info, other.info);
		eastl::swap(cachedAsset, other.cachedAsset);
	}

	void CopyFrom(const TAssetPtr& other)
	{
		info = other.info;
		cachedAsset = other.cachedAsset;
	}
};

DEFINE_TEMPLATE_CLASS_TRAITS(TAssetPtr,
	HasCustomSerialize = true,
	HasDetailsWidget = true
);
