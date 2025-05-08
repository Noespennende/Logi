#include "FolderStructureHandler.h"

#include "AssetViewUtils.h"

namespace Logi::FolderStructureHandler
{
	
//Suport functions
void CreateFolder(const FString& FolderPath, bool& bFolderCreated, FString& StatusMessage) {

	// Convert project path to absolute path
	FString AbsoluteFolderPath;
	if (!FPackageName::TryConvertLongPackageNameToFilename(FolderPath, AbsoluteFolderPath)) {
		bFolderCreated = false;
		StatusMessage = FString::Printf(TEXT("Creating folders failed - Failed to convert folder path to absolute path: '% s'"), *AbsoluteFolderPath);
		return;
	}

	//Check if folder already exists
	if (IFileManager::Get().DirectoryExists(*AbsoluteFolderPath)) {
		bFolderCreated = false;
		StatusMessage = FString::Printf(TEXT("Folder aldready exist: '% s'"), *AbsoluteFolderPath);
		return;
	}

	// Create a new folder
	if (!IFileManager::Get().MakeDirectory(*AbsoluteFolderPath, true)) {
		bFolderCreated = false;
		StatusMessage = FString::Printf(TEXT("Creating folders failed - Failed to create folder: '% s'"), *AbsoluteFolderPath);
		return;
	}

	//return success
	bFolderCreated = true;
	StatusMessage = FString::Printf(TEXT("Creating folders succeeded - Folder created: '% s'"), *AbsoluteFolderPath);

}

void CreateFolderStructure(bool& bFolderCreated, FString& StatusMessage) {

	// Create Logi_ThermalCamera folder
	CreateFolder("/Game/Logi_ThermalCamera", bFolderCreated, StatusMessage);
	//Print status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *StatusMessage);

	// Create Actor folder
	CreateFolder("/Game/Logi_ThermalCamera/Actors", bFolderCreated, StatusMessage);
	//Print status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *StatusMessage);


	//Create Material folder
	CreateFolder("/Game/Logi_ThermalCamera/Materials", bFolderCreated, StatusMessage);
	// Print status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *StatusMessage);

	// Set the folder color
	AssetViewUtils::SetPathColor("/Game/Logi_ThermalCamera", TOptional<FLinearColor>(FLinearColor(FColor::Cyan)));

}
}