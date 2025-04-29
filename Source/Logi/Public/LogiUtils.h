#pragma once

#include "CoreMinimal.h"
#include "UObject/Package.h"
#include "Materials/Material.h"
#include "UObject/SavePackage.h"


namespace FLogiUtils
{
	bool SaveAssetToDisk(UObject* Asset, const FSavePackageArgs& SaveArgs = FSavePackageArgs());
	
};
