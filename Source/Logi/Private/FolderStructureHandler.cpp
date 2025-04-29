#include "FolderStructureHandler.h"

#include "AssetViewUtils.h"

namespace Logi::FolderStructureHandler
{
	
//Suport functions
void CreateFolder(FString folderPath, bool& folderCreated, FString& statusMessage) {

	// Convert project path to absolute path
	FString AbsoluteFolderPath;
	if (!FPackageName::TryConvertLongPackageNameToFilename(folderPath, AbsoluteFolderPath)) {
		folderCreated = false;
		statusMessage = FString::Printf(TEXT("Creating folders failed - Failed to convert folder path to absolute path: '% s'"), *AbsoluteFolderPath);
		return;
	}

	//Check if folder already exists
	if (IFileManager::Get().DirectoryExists(*AbsoluteFolderPath)) {
		folderCreated = false;
		statusMessage = FString::Printf(TEXT("Folder aldready exist: '% s'"), *AbsoluteFolderPath);
		return;
	}

	// Create a new folder
	if (!IFileManager::Get().MakeDirectory(*AbsoluteFolderPath, true)) {
		folderCreated = false;
		statusMessage = FString::Printf(TEXT("Creating folders failed - Failed to create folder: '% s'"), *AbsoluteFolderPath);
		return;
	}

	//return success
	folderCreated = true;
	statusMessage = FString::Printf(TEXT("Creating folders succeeded - Folder created: '% s'"), *AbsoluteFolderPath);

}

void CreateFolderStructure(bool& folderCreated, FString& statusMessage) {

	// Create Logi_ThermalCamera folder
	CreateFolder("/Game/Logi_ThermalCamera", folderCreated, statusMessage);
	//Print status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *statusMessage);

	// Create Actor folder
	CreateFolder("/Game/Logi_ThermalCamera/Actors", folderCreated, statusMessage);
	//Print status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *statusMessage);


	//Create Material folder
	CreateFolder("/Game/Logi_ThermalCamera/Materials", folderCreated, statusMessage);
	// Print status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *statusMessage);

	//Create ActorMaterials folder inside the materials folder
	CreateFolder("/Game/Logi_ThermalCamera/Materials/ActorMaterials", folderCreated, statusMessage);
	// Print status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *statusMessage);

	// Set the folder color
	AssetViewUtils::SetPathColor("/Game/Logi_ThermalCamera", TOptional<FLinearColor>(FLinearColor(FColor::Cyan)));

}
}