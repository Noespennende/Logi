#pragma once

#include "CoreMinimal.h"
#include "UObject/Package.h"
#include "Materials/Material.h"
#include "UObject/SavePackage.h"


namespace Logi::LogiUtils
{
	bool SaveAssetToDisk(UObject* Asset, const FSavePackageArgs& SaveArgs = FSavePackageArgs());
	
};
