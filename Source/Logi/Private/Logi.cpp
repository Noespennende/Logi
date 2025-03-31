// Copyright Epic Games, Inc. All Rights Reserved.

#include "Logi.h"
#include "LogiStyle.h"
#include "LogiCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "../Public/LogiStyle.h"
#include "../Public/Logi.h"
#include "HAL/FileManager.h" // For editing files
#include "AssetViewUtils.h" 
#include "AssetRegistry/AssetRegistryModule.h"
#include "KismetCompilerModule.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "UObject/UnrealType.h"    
#include "UObject/ObjectMacros.h"
#include "UObject/Class.h"
#include "KismetCompiler.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_MathExpression.h"
#include "BlueprintFunctionNodeSpawner.h"
#include "BlueprintNodeSpawner.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "K2Node_VariableSet.h"
#include "K2Node_MakeStruct.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor.h"
#include "ObjectTools.h"
#include "PackageTools.h"
#include <functional>

#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "K2Node_FunctionEntry.h"

#include "MF_Logi_ThermalMaterialFunction.h"


static const FName LogiTabName("Logi");

#define LOCTEXT_NAMESPACE "FLogiModule"

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

	// Set the folder color
	AssetViewUtils::SetPathColor("/Game/Logi_ThermalCamera", TOptional<FLinearColor>(FLinearColor(FColor::Cyan)));

}

UBlueprint* CreateBlueprintClass(FString filePath, TSubclassOf<UObject> ParentClass, bool& bOutSuccess, FString& statusMessage) {

	//Check if the file path is valid
	if (StaticLoadObject(UObject::StaticClass(), nullptr, *filePath) != nullptr) {
		bOutSuccess = false;
		statusMessage = FString::Printf(TEXT("Failed to create blueprint - an asset already exists at that location  '%s'"), * filePath);
		return nullptr;
	}

	// Check if parent class is valid
	if (!FKismetEditorUtilities::CanCreateBlueprintOfClass(ParentClass)) {
		bOutSuccess = false;
		statusMessage = FString::Printf(TEXT("Failed to create blueprint - parent class is invalid '%s'"), *filePath);
		return nullptr;
	}

	// Create package
	UPackage* Package = CreatePackage(*filePath);
	if (Package == nullptr) {
		bOutSuccess = false;
		statusMessage = FString::Printf(TEXT("Failed to create blueprint - failed to create package - Filepath is invalid '%s'"), *filePath);
		return nullptr;
	}

	// Find the blueprint class to create
	UClass* BpClass = nullptr;
	UClass* BpGenClass = nullptr;
	FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler").GetBlueprintTypesForClass(ParentClass, BpClass, BpGenClass);

	// Create blueprint
	UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(ParentClass, Package, *FPaths::GetBaseFilename(filePath), BPTYPE_Normal, BpClass, BpGenClass);

	// Register Blueprint in the asset registry
	FAssetRegistryModule::AssetCreated(Blueprint);
	Blueprint->MarkPackageDirty();

	// Return success
	bOutSuccess = true;
	statusMessage = FString::Printf(TEXT("Create blueprint success - created blueprint at '%s'"), *filePath);
	return Blueprint;

}

auto AddVariableToBlueprintClass(UBlueprint* blueprint, FName varName, FEdGraphPinType pinType, bool bInstanceEditable, FString defaultValue) {
	
	//Validate blueprint
	if (blueprint == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Adding variable to blueprint failed because the blueprint is null"));
		return;
	}

	// Check if the variable already exists, if so it skips the creation
	if (FBlueprintEditorUtils::FindNewVariableIndex(blueprint, varName) != INDEX_NONE) {
		UE_LOG(LogTemp, Warning, TEXT("Variable '%s' already exists in the blueprint '%s', skipping the creation of variable."), *varName.ToString(), *blueprint->GetName());
		return;
	}


	FBlueprintEditorUtils::AddMemberVariable(blueprint, varName, pinType, defaultValue);


	//Sets innstance editable
	if (bInstanceEditable) {
		FBlueprintEditorUtils::SetBlueprintOnlyEditableFlag(blueprint, varName, !bInstanceEditable);
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(blueprint, varName, nullptr, FBlueprintMetadata::MD_Private, TEXT("false"));
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(blueprint, varName, nullptr, "MD_EditAnywhere", TEXT("true"));
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(blueprint, varName, nullptr, FBlueprintMetadata::MD_ExposeOnSpawn, TEXT("true"));

	}
}

auto AddThermalControlerReferenceToBlueprint(UBlueprint* blueprint, FName varName, bool bInstanceEditable) {

	if (!blueprint) {
		UE_LOG(LogTemp, Error, TEXT("Blueprint is null, can't add reference"));
		return;
	}

	if (FBlueprintEditorUtils::FindNewVariableIndex(blueprint, varName) != INDEX_NONE) {
		UE_LOG(LogTemp, Warning, TEXT("Variable '%s' already exists in blueprint '%s'"), *varName.ToString(), *blueprint->GetName());
		return;
	}

	// Load the blueprint asset (not just the generated class directly)
	UBlueprint* controllerBP = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, TEXT("/Game/Logi_ThermalCamera/Actors/BP_Logi_ThermalController.BP_Logi_ThermalController")));
	if (!controllerBP || !controllerBP->GeneratedClass) {
		UE_LOG(LogTemp, Error, TEXT("Could not load BP_Logi_ThermalController Blueprint or its GeneratedClass"));
		return;
	}

	// Use the generated class (this ensures Blueprint properties are recognized)
	UClass* controllerClass = controllerBP->GeneratedClass;

	// Create pin type
	FEdGraphPinType controllerRefType;
	controllerRefType.PinCategory = UEdGraphSchema_K2::PC_Object;
	controllerRefType.PinSubCategoryObject = controllerClass;

	// Add the variable
	FBlueprintEditorUtils::AddMemberVariable(blueprint, varName, controllerRefType);

	// Make it instance editable
	if (bInstanceEditable) {
		FBlueprintEditorUtils::SetBlueprintOnlyEditableFlag(blueprint, varName, !bInstanceEditable);
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(blueprint, varName, nullptr, FBlueprintMetadata::MD_Private, TEXT("false"));
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(blueprint, varName, nullptr, TEXT("EditAnywhere"), TEXT("true"));
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(blueprint, varName, nullptr, FBlueprintMetadata::MD_ExposeOnSpawn, TEXT("true"));
	}

	/*
	//Validate blueprint
	if (!blueprint) {
		UE_LOG(LogTemp, Error, TEXT("Blueprint is null, can't add referense to Thermal Controller in blueprint"));
		return;
	}

	// Checking if the variable already exists in blueprint
	if (FBlueprintEditorUtils::FindNewVariableIndex(blueprint, varName) != INDEX_NONE) {
		UE_LOG(LogTemp, Warning, TEXT("Variable '%s' already exists in the blueprint '%s', skipping the creation of variable."), *varName.ToString(), *blueprint->GetName());
		return;
	}

	//Finds the thermal controller blueprint class filepath
	const FString controllerPath = TEXT("/Game/Logi_ThermalCamera/Actors/BP_Logi_ThermalController.BP_Logi_ThermalController_C");

	//Loads the thermal controller blueprint class from the filepath
	UClass* controllerClass = LoadObject<UClass>(nullptr, *controllerPath);

	//Validates the thermal controller class
	if (!controllerClass || !controllerClass->IsChildOf<AActor>()) {
		UE_LOG(LogTemp, Error, TEXT("Failed to load generated class for BP_Logi_ThermalController from path: %s"), *controllerPath);
		return;
	}

	//Creates the variable type as an object referense to the thermal controller blueprint
	FEdGraphPinType controllerRefType;
	controllerRefType.PinCategory = UEdGraphSchema_K2::PC_Object;
	controllerRefType.PinSubCategoryObject = controllerClass;


	//Adds the variable to the blueprint
	FBlueprintEditorUtils::AddMemberVariable(blueprint, varName, controllerRefType);

	//Sets innstance editable
	if (bInstanceEditable) {
		FBlueprintEditorUtils::SetBlueprintOnlyEditableFlag(blueprint, varName, !bInstanceEditable);
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(blueprint, varName, nullptr, FBlueprintMetadata::MD_Private, TEXT("false"));
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(blueprint, varName, nullptr, "MD_EditAnywhere", TEXT("true"));
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(blueprint, varName, nullptr, FBlueprintMetadata::MD_ExposeOnSpawn, TEXT("true"));

	}

	*/
}

UEdGraphNode* AddNodeToBlueprint(UBlueprint* Blueprint, FName FunctionName, UClass* Class, FVector Location)
{
	if (!Blueprint)
	{
		// Blueprint is null, handle the error
		return nullptr;
	}

	if (!Class)
	{
		// The blueprint class is null, handle the error
		return nullptr;
	}

	UFunction* Function = Class->FindFunctionByName(FunctionName);
	if (!Function)
	{
		// The function does not exist in the blueprint class, handle the error
		UE_LOG(LogTemp, Warning, TEXT("Function Not Found"));
		return nullptr;
	}

	// Get the Event Graph of the blueprint
	UEdGraph* EventGraph = Blueprint->UbergraphPages[0];

	// Create a new function node in the Event Graph
	UEdGraphNode* NewNode = NewObject<UEdGraphNode>(EventGraph);


	UBlueprintFunctionNodeSpawner* FunctionNodeSpawner = UBlueprintFunctionNodeSpawner::Create(Function);
	if (FunctionNodeSpawner)
	{
		FunctionNodeSpawner->SetFlags(RF_Transactional);
		NewNode = FunctionNodeSpawner->Invoke(EventGraph, IBlueprintNodeBinder::FBindingSet(), FVector2D(Location.X, Location.Y));
		return NewNode;
	}

	return nullptr;

}

UK2Node_IfThenElse* createBPBranchNode(UEdGraph* eventGraph, int xPosition, int yPosition) {
	UK2Node_IfThenElse* branchNode = NewObject<UK2Node_IfThenElse>(eventGraph);
	branchNode->AllocateDefaultPins();
	eventGraph->AddNode(branchNode);
	branchNode->NodePosX = xPosition;
	branchNode->NodePosY = yPosition;

	return branchNode;
}

UK2Node_VariableGet* createBPGetterNode(UEdGraph* eventGraph, FName variableName, int xPosition, int yPosition) {
	UK2Node_VariableGet* getNode = NewObject<UK2Node_VariableGet>(eventGraph);
	getNode->VariableReference.SetSelfMember(variableName);
	getNode->AllocateDefaultPins();
	eventGraph->AddNode(getNode);
	getNode->NodePosX = xPosition;
	getNode->NodePosY = yPosition;

	return getNode;
}

UK2Node_VariableSet* createBPSetterNode(UEdGraph* eventGraph, FName variableName, int xPosition, int yPosition) {
	UK2Node_VariableSet* setterNode = NewObject<UK2Node_VariableSet>(eventGraph);
	setterNode->VariableReference.SetSelfMember(variableName);
	setterNode->AllocateDefaultPins();
	eventGraph->AddNode(setterNode);
	setterNode->NodePosX = xPosition;
	setterNode->NodePosY = yPosition;
	
	return setterNode;
}

UK2Node_CallFunction* createBPScalarParameterNode(UEdGraph* eventGraph, int xPosition, int yPosition) {
	UK2Node_CallFunction* scalarParameterNode = NewObject<UK2Node_CallFunction>(eventGraph);
	scalarParameterNode->FunctionReference.SetExternalMember(FName("SetScalarParameterValue"), UKismetMaterialLibrary::StaticClass());
	scalarParameterNode->AllocateDefaultPins();
	eventGraph->AddNode(scalarParameterNode);
	scalarParameterNode->NodePosX = xPosition;
	scalarParameterNode->NodePosY = yPosition;

	return scalarParameterNode;
}

UK2Node_CallFunction* createBPVectorParameterNode(UEdGraph* eventGraph, int xPosition, int yPosition) {
	UK2Node_CallFunction* vectorNode = NewObject<UK2Node_CallFunction>(eventGraph);
	vectorNode->FunctionReference.SetExternalMember(FName("SetVectorParameterValue"), UKismetMaterialLibrary::StaticClass());
	vectorNode->AllocateDefaultPins();
	eventGraph->AddNode(vectorNode);
	vectorNode->NodePosX = xPosition;
	vectorNode->NodePosY = yPosition;

	return vectorNode;
}

UK2Node_CallFunction* createBPNormalizeToRangeNode(UEdGraph* eventGraph, UK2Node_VariableGet* getNode, int xPosition, int yPosition) {
	UK2Node_CallFunction* normalizeToRangeNode = NewObject<UK2Node_CallFunction>(eventGraph);
	normalizeToRangeNode->FunctionReference.SetExternalMember(FName("NormalizeToRange"), UKismetMathLibrary::StaticClass());
	normalizeToRangeNode->AllocateDefaultPins();
	eventGraph->AddNode(normalizeToRangeNode);
	normalizeToRangeNode->NodePosX = xPosition;
	normalizeToRangeNode->NodePosY = yPosition;

	return normalizeToRangeNode;
}

UK2Node_CallFunction* createBPMakeVectorNode(UEdGraph* eventGraph, int xPosition, int yPosition) {
	UK2Node_CallFunction* makeVectorNode = NewObject<UK2Node_CallFunction>(eventGraph);
	makeVectorNode->FunctionReference.SetExternalMember(
		GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, MakeVector),
		UKismetMathLibrary::StaticClass()
	);
	makeVectorNode->AllocateDefaultPins();
	eventGraph->AddNode(makeVectorNode);
	makeVectorNode->NodePosX = xPosition;
	makeVectorNode->NodePosY = yPosition;

	return makeVectorNode;
}

void CreateThermalCameraControllerNodeSetup(UBlueprint* blueprint) {

	//Validate blueprint
	if (blueprint == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Creating Thermal Camera Controller Node Setup failed because the blueprint is null"));
		return;
	}

	//Finds the event graph for the blueprint
	UEdGraph* eventGraph = nullptr;
	for (UEdGraph* Graph : blueprint->UbergraphPages) {
		if (Graph->GetFName() == FName(TEXT("EventGraph"))) {
			eventGraph = Graph;
			break;
		}
	}

	//Validates the blueprint event graph
	if (eventGraph == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Creating Thermal Camera Controller Node Setup failed because the blueprint graph is null"));
		return;
	}

	//Get the event graph schema
	const UEdGraphSchema_K2* Schema = CastChecked<UEdGraphSchema_K2>(eventGraph->GetSchema());

	//Find the event tick node in the event graph
	UK2Node_Event* EventTick = nullptr;
	//Iterate over all the nodes in the blueprints event graph
	for (UEdGraphNode* Node : eventGraph->Nodes) {
		//Check if the node is an event node
		if (UK2Node_Event* EventNode = Cast<UK2Node_Event>(Node)) {
			//Check if the event node is the event tick node
			if (EventNode->EventReference.GetMemberName() == FName("ReceiveTick")) {
				EventTick = EventNode;
				break;  
			}
		}
	}

	//Validates that we actually found the event tick node
	if (!EventTick) {
		UE_LOG(LogTemp, Error, TEXT("Failed to find the Event Tick node in blueprints event graph"));
		return;
	}


	//Create a new branch node
	UK2Node_IfThenElse* BranchNode = createBPBranchNode(eventGraph, 300, 420);

	//Connect the event tick node to the branch node
	UEdGraphPin* ExecPin = EventTick->FindPin(UEdGraphSchema_K2::PN_Then);
	UEdGraphPin* BranchExecPin = BranchNode->GetExecPin();
	if (ExecPin && BranchExecPin) {
		Schema->TryCreateConnection(ExecPin, BranchExecPin);
	}

	//Create ThermalCameraActive variable get node
	UK2Node_VariableGet* ThermalCameraActiveGetNode = createBPGetterNode(eventGraph, FName("ThermalCameraActive"), 100, 550);

	//Connect the ThermalCameraActive variable get node to the branch node
	Schema->TryCreateConnection(ThermalCameraActiveGetNode->FindPin(FName("ThermalCameraActive")), BranchNode->GetConditionPin());


	//Create scalar parameter node - Thermal Camera Toggle true
	UK2Node_CallFunction* thermalToggleTrueScalarParameterNode = createBPScalarParameterNode(eventGraph, 600, 200);
	//Set node input for parameterName and value for Thermal Camera Toggle true
	UEdGraphPin* ParamNamePin = thermalToggleTrueScalarParameterNode->FindPin(FName("ParameterName"));
	UEdGraphPin* ParamValuePin = thermalToggleTrueScalarParameterNode->FindPin(FName("ParameterValue"));
	ParamNamePin->DefaultValue = "ThermalCameraToggle";
	ParamValuePin->DefaultValue = "1.0";

	//Create scalar parameter node - Thermal Camera Toggle false
	UK2Node_CallFunction* thermalToggleFalseScalarParameterNode = createBPScalarParameterNode(eventGraph, 600, 600);
	//Set node input for parameterName and value for  ThermalCameraToggle False
	ParamNamePin = thermalToggleFalseScalarParameterNode->FindPin(FName("ParameterName"));
	ParamValuePin = thermalToggleFalseScalarParameterNode->FindPin(FName("ParameterValue"));
	ParamNamePin->DefaultValue = "ThermalCameraToggle";
	ParamValuePin->DefaultValue = "0.0";


	//Connect thermal camera toggle parameter node to branch nodes true execution pin
	Schema->TryCreateConnection(BranchNode->GetThenPin(), thermalToggleTrueScalarParameterNode->GetExecPin());

	//Connect thermal camera toggle parameter node to branch nodes false execution pin
	Schema->TryCreateConnection(BranchNode->GetElsePin(), thermalToggleFalseScalarParameterNode->GetExecPin());

	//Create vector parameter nodes for Cold, Mid, Hot, BackgroundTemp temperatures
	FVector2D nodePosition(1000, 420);
	TArray<FString> vectorParams = { "Cold", "Mid", "Hot"};
	UK2Node_CallFunction* previousNode = nullptr;

	//Create vector parameter nodes
	for (const FString& Param : vectorParams) {
		//Create a vector parameter node
		UK2Node_CallFunction* vectorNode = createBPVectorParameterNode(eventGraph, nodePosition.X, nodePosition.Y);

		// Find the parameter pin for the vector node and set the default value
		UEdGraphPin* ParamPin = vectorNode->FindPin(FName("ParameterName"));
		ParamPin->DefaultValue = Param;

		//create assosiated Getter node for value parameter and add it to the graph
		UK2Node_VariableGet* getNode = createBPGetterNode(eventGraph, FName(*Param), (nodePosition.X - 100), (nodePosition.Y + 300));
	

		// connect the parameter nodes
		if (Param == "Cold") {
			//Connect Cold scalar paremeter node to the branch node
			Schema->TryCreateConnection(thermalToggleTrueScalarParameterNode->GetThenPin(), vectorNode->GetExecPin());
			Schema->TryCreateConnection(thermalToggleFalseScalarParameterNode->GetThenPin(), vectorNode->GetExecPin());
		}
		else {
			Schema->TryCreateConnection(vectorNode->GetExecPin(), previousNode->GetThenPin());
		}


		//Connect connect get node to the vector parameter node
		Schema->TryCreateConnection(getNode->FindPin(FName(*Param)), vectorNode->FindPin(FName("ParameterValue")));
		
		
		//Move node to the right for each iteration
		nodePosition.X += 400;

		//Set previous node
		previousNode = vectorNode;
	}


	//Create Scalar parameter nodes
	TArray<FString> scalarParams = { "BackgroundTemperature","SkyTemperature", "Blur", "NoiseAmount"};
	for (const FString& Param : scalarParams) {

		//Create a scalar parameter node
		UK2Node_CallFunction* scalarNode = createBPScalarParameterNode(eventGraph, nodePosition.X, nodePosition.Y);


		// Find the parameterPin for the scalar node and set the default value
		UEdGraphPin* ParamPin = scalarNode->FindPin(FName("ParameterName"));
		ParamPin->DefaultValue = Param;

		//create assosiated Getter node for value parameter and add it to the graph
		UK2Node_VariableGet* getNode = createBPGetterNode(eventGraph, FName(*Param), (nodePosition.X - 100), (nodePosition.Y + 300));

		//Spawn normalize to range node
		UK2Node_CallFunction* normalizeToRangeNode = createBPNormalizeToRangeNode(eventGraph, getNode, (nodePosition.X), (nodePosition.Y + 350));
		//Find normalize to range nodes range min and max pins
		UEdGraphPin* rangeMinPin = normalizeToRangeNode->FindPin(FName("RangeMin"));
		UEdGraphPin* rangeMaxPin = normalizeToRangeNode->FindPin(FName("RangeMax"));

		//Spawn getter node for camera range and add them to the background temparature's normalize node
		if (Param == "BackgroundTemperature" || Param == "SkyTemperature") {
			//Spawn thermal camera range getters
			UK2Node_VariableGet* thermalCameraRangeMinNode = createBPGetterNode(eventGraph, FName("ThermalCameraRangeMin"), getNode->NodePosX-150, (nodePosition.Y + 400));
			UK2Node_VariableGet* thermalCameraRangeMaxNode = createBPGetterNode(eventGraph, FName("ThermalCameraRangeMax"), getNode->NodePosX-150, (nodePosition.Y + 500));


			//Connect getter nodes to normalize to range node
			Schema->TryCreateConnection(thermalCameraRangeMinNode->FindPin(FName("ThermalCameraRangeMin")), normalizeToRangeNode->FindPin(FName("RangeMin")));
			Schema->TryCreateConnection(thermalCameraRangeMaxNode->FindPin(FName("ThermalCameraRangeMax")), normalizeToRangeNode->FindPin(FName("RangeMax")));
		}
		else {
			//Set normalize range's min and max values
			rangeMinPin->DefaultValue = TEXT("0.0");
			rangeMaxPin->DefaultValue = TEXT("100.0");
		}

		//Connect the getter to the normalize node
		Schema->TryCreateConnection(getNode->FindPin(FName(*Param)), normalizeToRangeNode->FindPin(FName("Value")));

		//Connect the normalize node to the scalar node
		Schema->TryCreateConnection(normalizeToRangeNode->FindPin(FName("ReturnValue")), scalarNode->FindPin(FName("ParameterValue")));

		//Connect the scalar node to the previous node
		Schema->TryCreateConnection(scalarNode->GetExecPin(), previousNode->GetThenPin());
		
		//Move node to the right for each iteration
		nodePosition.X += 400;

		//Set previous node
		previousNode = scalarNode;


	}

	//create NoiseSize variable getter node
	UK2Node_VariableGet* noiseSizeGet = createBPGetterNode(eventGraph, FName("NoiseSize"), nodePosition.X, (nodePosition.Y + 200));

	//Update node position
	nodePosition.X += 250;

	//Create makevector node to convert NoiseSize in to a 3D vector
	UK2Node_CallFunction* makeVectorNode = createBPMakeVectorNode(eventGraph, nodePosition.X, (nodePosition.Y + 200));

	//Create NoiseVector setter node
	UK2Node_VariableSet* setVectorNode = createBPSetterNode(eventGraph, FName("NoiseVector"), nodePosition.X, nodePosition.Y);

	//Connect NoiseSize to MakeVector.X/Y/Z
	UEdGraphPin* NoiseOut = noiseSizeGet->GetValuePin();
	Schema->TryCreateConnection(NoiseOut, makeVectorNode->FindPin(FName("X")));
	Schema->TryCreateConnection(NoiseOut, makeVectorNode->FindPin(FName("Y")));
	Schema->TryCreateConnection(NoiseOut, makeVectorNode->FindPin(FName("Z")));

	//Connect makeVector to setVector node
	Schema->TryCreateConnection(makeVectorNode->GetReturnValuePin(), setVectorNode->FindPin(FName("NoiseVector")));
	
	//Connect setVector node to previous node
	Schema->TryCreateConnection(setVectorNode->GetExecPin(), previousNode->GetThenPin());

	//Update node position
	nodePosition.X += 250;

	//Create Noisesize setVectorParameterValue node
	UK2Node_CallFunction* noiseSizeVectorNode = createBPVectorParameterNode(eventGraph, nodePosition.X, nodePosition.Y);

	// Set setVectorParameterValue node parameter name to NoiseSize
	UEdGraphPin* noiseSizevectorNodeParamPin = noiseSizeVectorNode->FindPin(FName("ParameterName"));
	noiseSizevectorNodeParamPin->DefaultValue = FString("NoiseSize");

	//Connect NoiseSize setVectorParameterValue node to setVector node
	Schema->TryCreateConnection(noiseSizeVectorNode->GetExecPin(), setVectorNode->GetThenPin());


	//Create getter node for the NoiseVector variable
	UK2Node_VariableGet* getNoiseVectorNode = createBPGetterNode(eventGraph, FName("NoiseVector"), nodePosition.X, (nodePosition.Y + 300));

	//Connect the NoiseVector getter node to the NoiseSize setVectorParameterValue node
	Schema->TryCreateConnection(getNoiseVectorNode->FindPin(FName("NoiseVector")), noiseSizeVectorNode->FindPin(FName("ParameterValue")));


	//Compile the blueprint
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(blueprint);

}

void CreateThermalController(bool& success, FString& statusMessage) {
	//Create thermal controller blueprint
	UBlueprint* thermalControllerBp = CreateBlueprintClass("/Game/Logi_ThermalCamera/Actors/BP_Logi_ThermalController", AActor::StaticClass(), success, statusMessage);

	//Print status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *statusMessage);

	//Check if blueprint is valid
	if (!thermalControllerBp) {
		UE_LOG(LogTemp, Error, TEXT("Failed to create Thermal Controller Blueprint - Blueprint is NULL!"));
		success = false;
		statusMessage = "Failed to create Thermal Controller Blueprint - Blueprint is NULL!";
		return;
	}

	//Create bolean type
	FEdGraphPinType BoolType;
	BoolType.PinCategory = UEdGraphSchema_K2::PC_Boolean;

	//Create float type
	FEdGraphPinType FloatType;
	FloatType.PinCategory = UEdGraphSchema_K2::PC_Real;
	FloatType.PinSubCategory = UEdGraphSchema_K2::PC_Float;

	//Create Linear Color type
	FEdGraphPinType LinearColorType;
	LinearColorType.PinCategory = UEdGraphSchema_K2::PC_Struct;
	LinearColorType.PinSubCategoryObject = TBaseStructure<FLinearColor>::Get();

	//Create vector type
	FEdGraphPinType VectorType;
	VectorType.PinCategory = UEdGraphSchema_K2::PC_Struct;
	VectorType.PinSubCategoryObject = TBaseStructure<FVector>::Get();

	//Print status
	UE_LOG(LogTemp, Warning, TEXT("Adding variables to thermal controller blueprint"));

	//Add variables to the blueprint
	AddVariableToBlueprintClass(thermalControllerBp, "ThermalCameraActive", BoolType, true, "false");
	AddVariableToBlueprintClass(thermalControllerBp, "ThermalCameraRangeMin", FloatType, true, "0.0");
	AddVariableToBlueprintClass(thermalControllerBp, "ThermalCameraRangeMax", FloatType, true, "25.0");
	AddVariableToBlueprintClass(thermalControllerBp, "BackgroundTemperature", FloatType, true, "5.0");
	AddVariableToBlueprintClass(thermalControllerBp, "SkyTemperature", FloatType, true, "5.0");
	AddVariableToBlueprintClass(thermalControllerBp, "Blur", FloatType, true, "100");
	AddVariableToBlueprintClass(thermalControllerBp, "NoiseSize", FloatType, true, "1.0");
	AddVariableToBlueprintClass(thermalControllerBp, "NoiseAmount", FloatType, true, "5.0");
	AddVariableToBlueprintClass(thermalControllerBp, "Cold", LinearColorType, true, "R=0.0,G=0.0,B=0.0,A=1.0");
	AddVariableToBlueprintClass(thermalControllerBp, "Mid", LinearColorType, true, "R=0.5,G=0.5,B=0.5,A=1.0");
	AddVariableToBlueprintClass(thermalControllerBp, "Hot", LinearColorType, true, "R=1.0,G=1.0,B=1.0,A=1.0");
	AddVariableToBlueprintClass(thermalControllerBp, "NoiseVector", VectorType, false, "0, 0, 0");


	//Add nodes to the blueprint
	CreateThermalCameraControllerNodeSetup(thermalControllerBp);
	
	//Validates blueprint
	if (!thermalControllerBp->GeneratedClass) {
		success = false;
		statusMessage = "Thermal Controller Blueprint has an invalid generated class.";
		return;
	}

	//Gets the actors default object
	AActor* ActorDefaultObject = Cast<AActor>(thermalControllerBp->GeneratedClass->GetDefaultObject());

	//Sets default values to variables
	if (ActorDefaultObject) {

		//Creates anonymus functions to set default values to variables for each variabel type
		auto SetFloatProperty = [&](FName PropertyName, float Value) {
			FFloatProperty* FloatProp = FindFProperty<FFloatProperty>(ActorDefaultObject->GetClass(), PropertyName);
			if (FloatProp) {
				FloatProp->SetPropertyValue_InContainer(ActorDefaultObject, Value);
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Property %s not found in blueprint class"), *PropertyName.ToString());
			}
			};

		auto SetBoolProperty = [&](FName PropertyName, bool Value) {
			FBoolProperty* BoolProp = FindFProperty<FBoolProperty>(ActorDefaultObject->GetClass(), PropertyName);
			if (BoolProp) {
				BoolProp->SetPropertyValue_InContainer(ActorDefaultObject, Value);
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Property %s not found in blueprint class"), *PropertyName.ToString());
			}
			};

		auto SetLinearColorProperty = [&](FName PropertyName, FLinearColor Value) {
			FStructProperty* StructProp = FindFProperty<FStructProperty>(ActorDefaultObject->GetClass(), PropertyName);
			if (StructProp && StructProp->Struct == TBaseStructure<FLinearColor>::Get()) {
				FLinearColor* ColorPtr = StructProp->ContainerPtrToValuePtr<FLinearColor>(ActorDefaultObject);
				*ColorPtr = Value;
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Property %s not found in blueprint class"), *PropertyName.ToString());
			}
			};

		auto SetVectorProperty = [&](FName PropertyName, FVector Value) {
			FStructProperty* StructProp = FindFProperty<FStructProperty>(ActorDefaultObject->GetClass(), PropertyName);
			if (StructProp && StructProp->Struct == TBaseStructure<FVector>::Get()) {
				FVector* VectorPtr = StructProp->ContainerPtrToValuePtr<FVector>(ActorDefaultObject);
				*VectorPtr = Value;
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Property %s not found in blueprint class"), *PropertyName.ToString());
			}
			};

		//Sets default values
		SetBoolProperty("ThermalCameraActive", true);
		SetFloatProperty("ThermalCameraRangeMin", 0.0f);
		SetFloatProperty("ThermalCameraRangeMax", 100.0f);
		SetFloatProperty("BackgroundTemperature", 25.0f);
		SetFloatProperty("Blur", 5.0f);
		SetFloatProperty("NoiseSize", 1.0f);
		SetFloatProperty("NoiseAmount", 0.5f);
		SetLinearColorProperty("Cold", FLinearColor(0.0f, 0.0f, 1.0f, 1.0f)); // Blue
		SetLinearColorProperty("Mid", FLinearColor(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow
		SetLinearColorProperty("Hot", FLinearColor(1.0f, 0.0f, 0.0f, 1.0f)); // Red
		SetVectorProperty("NoiseVector", FVector(0.0f, 0.0f, 0.0f));
	}

	//Print status
	UE_LOG(LogTemp, Warning, TEXT("Compiling thermal controller blueprint"));

	//Compile blueprint
	FKismetEditorUtilities::CompileBlueprint(thermalControllerBp);
	thermalControllerBp->MarkPackageDirty();

	success = true;
	statusMessage = FString::Printf(TEXT("Thermal controller blueprint created and compiled successfully"));
}

void FindAllNonLogiActorBlueprintsInProject(TArray<FAssetData>& OutActorBlueprints) {
	//Get the asset registry module
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	//Get the asset registry inside the asset registry module
	IAssetRegistry& Registry = AssetRegistryModule.Get();

	// verify registry is up to date
	Registry.SearchAllAssets(true);

	// Create filert for the search
	FARFilter Filter;
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Add(FName("/Game"));
	Filter.bRecursivePaths = true;

	//create array to hold blueprints found
	TArray<FAssetData> AssetList;

	//Get all blueprints in the /games (content) folder and add them to the AssetsList list
	Registry.GetAssets(Filter, AssetList);

	//Filter out all blueprints that are not actors
	for (const FAssetData& Asset : AssetList) {

		// Skipping the Logi_ThermalCamera folder
		if (Asset.PackagePath.ToString().StartsWith("/Game/Logi_ThermalCamera")) {
			continue;
		}

		//Check if the blueprint has a parent class
		FString ParentClassPath;

		//Get the parent class path and set it to the ParentClassPath variable to that path
		if (Asset.GetTagValue<FString>("ParentClass", ParentClassPath))
		{
			// Remove the autogenerated "_C" suffix from the class path
			ParentClassPath.RemoveFromEnd(TEXT("_C"), ESearchCase::IgnoreCase);

			//Tries to find the actualparent class object from the classe's path
			UClass* ParentClass = FindObject<UClass>(nullptr, *ParentClassPath);

			//Check if the parent class is a child of the AActor, i.e if it's an Actor blueprint
			if (ParentClass && ParentClass->IsChildOf(AActor::StaticClass()))
			{
				//Actor blueprint added to the list
				OutActorBlueprints.Add(Asset);
			}
		}
	}
}

void AddLogiVariablesToActorBlueprint(const FAssetData& actor) {

	//Cast asset data to blueprint type
	UBlueprint* blueprint = Cast<UBlueprint>(actor.GetAsset());

	//Validate that the cast was successfull
	if(!blueprint) {
		UE_LOG(LogTemp, Error, TEXT("Failed to load blueprint from asset data: %s"), *actor.AssetName.ToString());
		return;
	}

	//Create bolean type
	FEdGraphPinType boolType;
	boolType.PinCategory = UEdGraphSchema_K2::PC_Boolean;

	//Create float type
	FEdGraphPinType floatType;
	floatType.PinCategory = UEdGraphSchema_K2::PC_Real;
	floatType.PinSubCategory = UEdGraphSchema_K2::PC_Float;

	//Create Linear Color type
	FEdGraphPinType linearColorType;
	linearColorType.PinCategory = UEdGraphSchema_K2::PC_Struct;
	linearColorType.PinSubCategoryObject = TBaseStructure<FLinearColor>::Get();

	//Create vector type
	FEdGraphPinType vectorType;
	vectorType.PinCategory = UEdGraphSchema_K2::PC_Struct;
	vectorType.PinSubCategoryObject = TBaseStructure<FVector>::Get();

	//Print status
	UE_LOG(LogTemp, Warning, TEXT("Adding variable to actor blueprint"));

	//Add variables to the blueprint
	AddVariableToBlueprintClass(blueprint, "Logi_Hot", boolType, true, "false");
	AddVariableToBlueprintClass(blueprint, "Logi_BaseTemperature", floatType, true, "0.0");
	AddVariableToBlueprintClass(blueprint, "Logi_MaxTemperature", floatType, true, "25.0");
	AddVariableToBlueprintClass(blueprint, "Logi_CurrentTemperature", floatType, true, "25.0");
	/*
	AddThermalControlerReferenceToBlueprint(blueprint, "Logi_ThermalController", false);
	*/
}

auto AddSetupFunctionToNonLogiActor(const FAssetData& actor) {

	UBlueprint* blueprint = Cast<UBlueprint>(actor.GetAsset());
	FName functionName = FName("Logi_ThermalMaterialUpdate");


	//Validate blueprint
	if (!blueprint) {
		UE_LOG(LogTemp, Error, TEXT("Blueprint is null, cannot add setup function."));
		return;
	}

	// Check if the setup function already exists in the blueprint
	for (UEdGraph* Graph : blueprint->FunctionGraphs)
	{
		if (Graph && Graph->GetFName() == functionName)
		{
			UE_LOG(LogTemp, Warning, TEXT("Function '%s' already exists in blueprint '%s', skipping implementation."), *functionName.ToString(), *blueprint->GetName());
			return;
		}
	}


	// Create the setup function
	UEdGraph* newFunctionGraph = FBlueprintEditorUtils::CreateNewGraph(blueprint, functionName, UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass());
	FBlueprintEditorUtils::AddFunctionGraph<UFunction>(blueprint, newFunctionGraph,true, nullptr);

	// Create setup function function entry node
	newFunctionGraph->Modify();
	newFunctionGraph->bAllowDeletion = true;
	UK2Node_FunctionEntry* entryNode = NewObject<UK2Node_FunctionEntry>(newFunctionGraph);
	entryNode->FunctionReference.SetSelfMember(functionName);
	entryNode->NodePosX = 0;
	entryNode->NodePosY = 0;
	entryNode->AllocateDefaultPins();

	newFunctionGraph->AddNode(entryNode);

	//Mark blueprint as modified
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(blueprint);

	UE_LOG(LogTemp, Log, TEXT("Function 'Logi_ThermalActorSetup' successfully added to blueprint '%s'."), *blueprint->GetName());
}

auto AddUpdateThermalMaterialFunctionToNonLogiActor(const FAssetData& actor) {

	UBlueprint* blueprint = Cast<UBlueprint>(actor.GetAsset());
	FName functionName = FName("Logi_UpdateThermalMaterial");

	//Validate blueprint
	if (!blueprint) {
		UE_LOG(LogTemp, Error, TEXT("Blueprint is null, cannot add setup function."));
		return;
	}

	// Check if the setup function already exists in the blueprint
	for (UEdGraph* Graph : blueprint->FunctionGraphs)
	{
		if (Graph && Graph->GetFName() == functionName)
		{
			UE_LOG(LogTemp, Warning, TEXT("Function '%s' already exists in blueprint '%s', skipping implementation."), *functionName.ToString(), *blueprint->GetName());
			return;
		}
	}


	// Create the setup function
	UEdGraph* newFunctionGraph = FBlueprintEditorUtils::CreateNewGraph(blueprint, functionName, UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass());
	FBlueprintEditorUtils::AddFunctionGraph<UFunction>(blueprint, newFunctionGraph, true, nullptr);

	// Create setup function function entry node
	newFunctionGraph->Modify();
	newFunctionGraph->bAllowDeletion = true;
	UK2Node_FunctionEntry* entryNode = NewObject<UK2Node_FunctionEntry>(newFunctionGraph);
	entryNode->FunctionReference.SetSelfMember(functionName);
	entryNode->NodePosX = 0;
	entryNode->NodePosY = 0;
	entryNode->AllocateDefaultPins();

	newFunctionGraph->AddNode(entryNode);

	//Mark blueprint as modified
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(blueprint);

	UE_LOG(LogTemp, Log, TEXT("Function 'Logi_ThermalActorSetup' successfully added to blueprint '%s'."), *blueprint->GetName());
}

void MakeProjectBPActorsLogiCompatible() {
	//Create a list to hold all the actor blueprints in the project
	TArray<FAssetData> projectActors;
	//Find all the blueprints of type Actor in the /games (content) folder and add them to the projectActors list
	FindAllNonLogiActorBlueprintsInProject(projectActors);

	//Add Logi variables to all the actor blueprints in the project
	for (FAssetData actor : projectActors) {
		//Prints status
		UE_LOG(LogTemp, Warning, TEXT("Adding Logi variables to actor: %s"), *actor.AssetName.ToString());

		//Add Logi variables to the actor blueprint
		AddLogiVariablesToActorBlueprint(actor);
		AddSetupFunctionToNonLogiActor(actor);
		AddUpdateThermalMaterialFunctionToNonLogiActor(actor);
	}
}

//Plugin functions

void FLogiModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FLogiStyle::Initialize();
	FLogiStyle::ReloadTextures();

	FLogiCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FLogiCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FLogiModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FLogiModule::RegisterMenus));
}

void FLogiModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FLogiStyle::Shutdown();

	FLogiCommands::Unregister();
}


void FLogiModule::PluginButtonClicked()
{
	bool success;
	FString statusMessage;

	// Create folder structure
	CreateFolderStructure(success, statusMessage);

	//Log status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *statusMessage);

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (World) {
		SetupThermalSettings(World);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Failed to get UWorld!"));
	}

	//Create Thermal Controller blueprint
	CreateThermalController(success, statusMessage);

	//Log status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *statusMessage);

	

	// Create Thermal MaterialFunction
	FMF_ThermalMaterialFunction::CreateMaterialFunction(success, statusMessage);

	//Log status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *statusMessage);
	//Make all project actors logi compatible
	MakeProjectBPActorsLogiCompatible();

}


void FLogiModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FLogiCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FLogiCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

UMaterialParameterCollection* FLogiModule::EnsureThermalSettingsExist(UWorld* World)
{
	if (!World) return nullptr;

	//Asset path
	FString AssetPath = TEXT("/Game/Logi_ThermalCamera/Materials//MPC_ThermalSettings");
	UMaterialParameterCollection* ThermalSettings = LoadObject<UMaterialParameterCollection>(nullptr, *AssetPath);

	//if the MPC does not exist, create new MPC
	if (!ThermalSettings)
	{
		UPackage* Package = CreatePackage(*AssetPath);
		ThermalSettings = NewObject<UMaterialParameterCollection>(Package, UMaterialParameterCollection::StaticClass(), FName("MPC_ThermalSettings"), RF_Public | RF_Standalone);
		if (ThermalSettings) {

			//Lambda functions for adding parameters to MPC
			auto AddScalar = [&ThermalSettings](const FName& Name, float DefaultValue) -> void {
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
			FString FilePath = FPackageName::LongPackageNameToFilename(AssetPath, FPackageName::GetAssetPackageExtension());
			UPackage::SavePackage(Package, ThermalSettings, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);
			UE_LOG(LogTemp, Warning, TEXT("Created MPC_LogiThermalSettings successfully."));
		}

		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create MPC_LogiThermalSettings."));
		}
	}

	return ThermalSettings;
}

void FLogiModule::SetupThermalSettings(UWorld* World)
{
	if (!World) return;

	// Checks if MPC exists
	UMaterialParameterCollection* ThermalSettings = EnsureThermalSettingsExist(World);
	if (!ThermalSettings) return;

	// Checks if MPc has required parameters
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
		bool bExists = ThermalSettings->ScalarParameters.ContainsByPredicate(
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

	for (const auto& VectorPair : VectorsRequired)
	{
		bool bExists = ThermalSettings->VectorParameters.ContainsByPredicate(
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
		FString AssetPath = TEXT("/Game/Logi_ThermalCamera/Materials/MPC_LogiThermalSettings");
		FString FilePath = FPackageName::LongPackageNameToFilename(AssetPath, FPackageName::GetAssetPackageExtension());
		UPackage::SavePackage(ThermalSettings->GetOutermost(), ThermalSettings, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);
		UE_LOG(LogTemp, Warning, TEXT("Added missing parameters to MPC_LogiThermalSettings"));
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


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLogiModule, Logi)