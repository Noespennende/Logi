#include "LogiUtils.h"

#include "Engine/World.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"

bool FLogiUtils::SaveAssetToDisk(UObject* Asset, const FSavePackageArgs& SaveArgs)
{
	if (!Asset) return false;

	UPackage* Package = Asset->GetOutermost();
	FString PackageFilename = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());

	return UPackage::SavePackage(Package, Asset, *PackageFilename, SaveArgs);
}
