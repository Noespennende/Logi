#include "ThermalSettings.h"

#include "LogiUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"


namespace Logi::MPCThermalSettings
{
	UMaterialParameterCollection* EnsureThermalSettingsExist(const UWorld* World)
	{
		if (!World) return nullptr;

		//Asset path
		const FString AssetPath = TEXT("/Game/Logi_ThermalCamera/Materials/MPC_Logi_ThermalSettings");

		// Try to load the existing MPC
		UMaterialParameterCollection* ThermalSettings = LoadObject<UMaterialParameterCollection>(nullptr, *AssetPath);

		//if the MPC does not exist, create new MPC
		if (!ThermalSettings)
		{
			UPackage* Package = CreatePackage(*FString("/Game/Logi_ThermalCamera/Materials"));
			ThermalSettings = NewObject<UMaterialParameterCollection>(Package, UMaterialParameterCollection::StaticClass(), FName("MPC_Logi_ThermalSettings"), RF_Public | RF_Standalone);
			if (ThermalSettings) {

				//Lambda functions for adding parameters to MPC
				auto AddScalar = [&ThermalSettings](const FName& Name, const float DefaultValue) -> void {
					FCollectionScalarParameter Param;
					Param.ParameterName = Name;
					Param.DefaultValue = DefaultValue;
					ThermalSettings->ScalarParameters.Add(Param);
					};

				auto AddVector = [&ThermalSettings](const FName& Name, const FLinearColor& DefaultValue) -> void {
					FCollectionVectorParameter Param;
					Param.ParameterName = Name;
					Param.DefaultValue = DefaultValue;
					ThermalSettings->VectorParameters.Add(Param);
					};
					
				//Scalar parameters to be added
				AddScalar(FName("ThermalCameraToggle"), 0.0);
				AddScalar(FName("BackgroundTemperature"), 0.0);
				AddScalar(FName("Blur"), 0.0);
				AddScalar(FName("NoiseAmount"), 0.05);
				AddScalar(FName("SkyTemperature"), 0.0);


				//vector parameters to be added
				AddVector(FName("Cold"), FLinearColor(1, 1, 1, 1));
				AddVector(FName("Mid"), FLinearColor(0, 1, 0, 1));
				AddVector(FName("Hot"), FLinearColor(1, 0, 0, 1));
				AddVector(FName("NoiseSize"), FLinearColor(0, 0, 0, 0));

				//Save the MPC with parameters
				ThermalSettings->MarkPackageDirty();
				FAssetRegistryModule::AssetCreated(ThermalSettings);
				const FString FilePath = FPackageName::LongPackageNameToFilename(AssetPath, FPackageName::GetAssetPackageExtension());
				UPackage::SavePackage(Package, ThermalSettings, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);
				UE_LOG(LogTemp, Warning, TEXT("Created MPC_Logi_ThermalSettings successfully."));
			}

			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to create MPC_Logi_ThermalSettings."));
			}
		}

		return ThermalSettings;
	}

	void MPCThermalSettings::SetupThermalSettings(const UWorld* World, bool& bSuccess, FString& StatusMessage)
	{
		if (!World) return;

		// Checks if MPC exists
		UMaterialParameterCollection* ThermalSettings = EnsureThermalSettingsExist(World);
		if (!ThermalSettings) return;

		// Checks if MPC has required parameters
		bool bUpdated = false;

		// checks if mpc has required scalar parameters if not add them
		TArray<FName> ScalarsRequired = {
			FName("ThermalCameraToggle"),
			FName("BackgroundTemperature"),
			FName("Blur"),
			FName("NoiseAmount"),
			FName("SkyTemperature"),
		};

		for (const FName& ScalarName : ScalarsRequired)
		{
			const bool bExists = ThermalSettings->ScalarParameters.ContainsByPredicate(
				[&ScalarName](const FCollectionScalarParameter& Param) { return Param.ParameterName == ScalarName; });

			if (!bExists)
			{
				FCollectionScalarParameter NewScalar;
				NewScalar.ParameterName = ScalarName;
				NewScalar.DefaultValue = (ScalarName == FName("NoiseAmount")) ? 0.05f : 0.0f;  
				ThermalSettings->ScalarParameters.Add(NewScalar);
				bUpdated = true;
			}
		}

		// checks if mpc has required vector parameters if not add them
		TArray<TPair<FName, FLinearColor>> VectorsRequired = {
			{ FName("Cold"), FLinearColor(1, 1, 1, 1) },
			{ FName("Mid"), FLinearColor(0, 1, 0, 1) },
			{ FName("Hot"), FLinearColor(1, 0, 0, 1) },
			{ FName("NoiseSize"), FLinearColor(0, 0, 0, 0) }
		};

		for (const TPair<FName, FLinearColor>& VectorPair : VectorsRequired)
		{
			const bool bExists = ThermalSettings->VectorParameters.ContainsByPredicate(
				[&VectorPair](const FCollectionVectorParameter& Param) { return Param.ParameterName == VectorPair.Key; });

			if (!bExists)
			{
				FCollectionVectorParameter NewVector;
				NewVector.ParameterName = VectorPair.Key;
				NewVector.DefaultValue = VectorPair.Value;
				ThermalSettings->VectorParameters.Add(NewVector);
				bUpdated = true;
			}
		}

		// save the parameters that were added
		if (bUpdated)
		{
			ThermalSettings->MarkPackageDirty();
			FAssetRegistryModule::AssetCreated(ThermalSettings);


			bool bSaved = LogiUtils::SaveAssetToDisk(ThermalSettings);

			if (bSaved)
			{
			
				StatusMessage = FString::Printf(TEXT("Blueprint %s created and successfully saved to: %s"), *ThermalSettings->GetName(), *ThermalSettings->GetPathName());
				bSuccess = true;
			}
			else
			{
				StatusMessage = FString::Printf(TEXT("Blueprint %s created, but failed to save properly. Manual save will be required: %s"), *ThermalSettings->GetName(), *ThermalSettings->GetPathName());
				bSuccess = true;
			}
			
			UE_LOG(LogTemp, Warning, TEXT("Added missing parameters to MPC_Logi_ThermalSettings"));
			
		}

		// set values of MPC asset
		UMaterialParameterCollectionInstance* Instance = World->GetParameterCollectionInstance(ThermalSettings);
		if (Instance)
		{
			Instance->SetScalarParameterValue(FName("ThermalCameraToggle"),0);
			Instance->SetScalarParameterValue(FName("BackgroundTemperature"), 0);
			Instance->SetScalarParameterValue(FName("Blur"), 0);
			Instance->SetScalarParameterValue(FName("NoiseAmount"), 0.05);
			Instance->SetScalarParameterValue(FName("SkyTemperature"), 0);

			Instance->SetVectorParameterValue(FName("Cold"), FLinearColor(0,0,0,0));
			Instance->SetVectorParameterValue(FName("Mid"), FLinearColor(0.5, 0.5, 0.5, 0));
			Instance->SetVectorParameterValue(FName("Hot"), FLinearColor(1,1,1,0));
			Instance->SetVectorParameterValue(FName("NoiseSize"), FLinearColor(1,1,1,0));

			UE_LOG(LogTemp, Warning, TEXT("Thermal settings applied."));
		}
		
	}

}
