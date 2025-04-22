#pragma once

#include "CoreMinimal.h"
#include "UObject/Package.h"
#include "Materials/Material.h"
#include "UObject/SavePackage.h"


class LOGI_API FLogiUtils
{
public:
	static bool SaveAssetToDisk(UObject* Asset, const FSavePackageArgs& SaveArgs = FSavePackageArgs());
};
