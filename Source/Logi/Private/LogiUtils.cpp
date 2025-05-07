#include "LogiUtils.h"

#include "Engine/World.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "UObject/SavePackage.h"

namespace Logi::LogiUtils
{
	bool SaveAssetToDisk(UObject* Asset, const FSavePackageArgs& SaveArgs)
	{
		if (!Asset) return false;

		UPackage* Package = Asset->GetOutermost();
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());

		return UPackage::SavePackage(Package, Asset, *PackageFilename, SaveArgs);
	}
}

