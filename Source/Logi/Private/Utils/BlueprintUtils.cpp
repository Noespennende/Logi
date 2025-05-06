#include "Utils/BlueprintUtils.h"

#include "Kismet2/BlueprintEditorUtils.h"

#include "K2Node_CallFunction.h"
#include "K2Node_Event.h"
#include "K2Node_GetArrayItem.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_Select.h"
#include "KismetCompilerModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"


#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "HAL/FileManager.h" // For editing files
#include "UObject/UnrealType.h"    
#include "UObject/ObjectMacros.h"
#include "UObject/Class.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_VariableGet.h"
#include "BlueprintFunctionNodeSpawner.h"
#include "K2Node_VariableSet.h"
#include "Materials/MaterialParameterCollection.h"
#include "Editor.h"
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "Logi_Outliner.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionMaterialFunctionCall.h"
#include "Materials/MaterialExpression.h"
#include "MaterialShared.h"
#include "Materials/MaterialExpression.h"
#include "Materials/MaterialFunction.h"
#include "Materials/MaterialFunctionInterface.h"
#include "Materials/MaterialExpressionMaterialFunctionCall.h"
#include "Components/PrimitiveComponent.h"

namespace Logi::BlueprintUtils
{
	FName AddMaterialInstanceVariableToBlueprint(UBlueprint* blueprint) {

	//Validate blueprint
	if (blueprint == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Adding variable to blueprint failed because the blueprint is null"));
		return FName("");
	}

	//Create a new variable in the blueprint
	const FName variableName = FBlueprintEditorUtils::FindUniqueKismetName(blueprint, TEXT("Logi_DynamicMaterialInstance"));
	FEdGraphPinType variableType;
	variableType.PinCategory = UEdGraphSchema_K2::PC_Object;
	variableType.PinSubCategoryObject = UMaterialInstanceDynamic::StaticClass();

	//Add variable to the blueprint
	FBlueprintEditorUtils::AddMemberVariable(blueprint, variableName, variableType);

	return variableName;
}
	
	void AddThermalControlerReferenceToBlueprint(UBlueprint* blueprint, FName varName, bool bInstanceEditable) {

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

		//Set variable as blueprint editable
		FBlueprintEditorUtils::SetBlueprintOnlyEditableFlag(blueprint, varName, true);

		// Make it instance editable
		if (bInstanceEditable) {
			FBlueprintEditorUtils::SetBlueprintOnlyEditableFlag(blueprint, varName, !bInstanceEditable);
			FBlueprintEditorUtils::SetBlueprintVariableMetaData(blueprint, varName, nullptr, FBlueprintMetadata::MD_Private, TEXT("false"));
			FBlueprintEditorUtils::SetBlueprintVariableMetaData(blueprint, varName, nullptr, TEXT("EditAnywhere"), TEXT("true"));
			FBlueprintEditorUtils::SetBlueprintVariableMetaData(blueprint, varName, nullptr, FBlueprintMetadata::MD_ExposeOnSpawn, TEXT("true"));
		}
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

		//create spawner to spawn the node
		UBlueprintFunctionNodeSpawner* FunctionNodeSpawner = UBlueprintFunctionNodeSpawner::Create(Function);

		//spawn the node
		if (FunctionNodeSpawner)
		{
			FunctionNodeSpawner->SetFlags(RF_Transactional);
			NewNode = FunctionNodeSpawner->Invoke(EventGraph, IBlueprintNodeBinder::FBindingSet(), FVector2D(Location.X, Location.Y));
			return NewNode;
		}

		return nullptr;

	}

	UEdGraphNode* AddNodeToBlueprintFunction(UEdGraph* functionGraph, FName functionName, UClass* nodeClass, FVector location) {

		//validate function graph
		if (!functionGraph) {
			UE_LOG(LogTemp, Error, TEXT("Function graph is a nullpointer, can't add node"));
			return nullptr;
		}

		//Find node class function
		UFunction* function = nodeClass->FindFunctionByName(functionName);

		//Validate node class function
		if (!function)
		{
			// The function does not exist in the blueprint class, handle the error
			UE_LOG(LogTemp, Warning, TEXT("node class function not found"));
			return nullptr;
		}

		// Create a new function node in the function graph
		UEdGraphNode* newNode = NewObject<UEdGraphNode>(functionGraph);

		//Create spawner to spawn the node
		UBlueprintFunctionNodeSpawner* FunctionNodeSpawner = UBlueprintFunctionNodeSpawner::Create(function);

		//Spawn the node
		if (FunctionNodeSpawner)
		{
			FunctionNodeSpawner->SetFlags(RF_Transactional);
			newNode = FunctionNodeSpawner->Invoke(functionGraph, IBlueprintNodeBinder::FBindingSet(), FVector2D(location.X, location.Y));
			return newNode;
		}

		return nullptr;
	}

	UK2Node_IfThenElse* CreateBPBranchNode(UEdGraph* eventGraph, int xPosition, int yPosition) {
		UK2Node_IfThenElse* branchNode = NewObject<UK2Node_IfThenElse>(eventGraph);
		branchNode->AllocateDefaultPins();
		eventGraph->AddNode(branchNode);
		branchNode->NodePosX = xPosition;
		branchNode->NodePosY = yPosition;
		branchNode->NodeGuid = FGuid::NewGuid();

		return branchNode;
	}

	UK2Node_VariableGet* CreateBPGetterNode(UEdGraph* eventGraph, FName variableName, int xPosition, int yPosition) {
		UK2Node_VariableGet* getNode = NewObject<UK2Node_VariableGet>(eventGraph);
		getNode->VariableReference.SetSelfMember(variableName);
		getNode->AllocateDefaultPins();
		eventGraph->AddNode(getNode);
		getNode->NodePosX = xPosition;
		getNode->NodePosY = yPosition;
		getNode->NodeGuid = FGuid::NewGuid();

		return getNode;
	}

	UK2Node_VariableGet* CreateBPExternalGetterNode(UEdGraph* eventGraph, FName variableName, const TCHAR* externalClassFilepath, int xPosition, int yPosition) {
		UK2Node_VariableGet* getNode = NewObject<UK2Node_VariableGet>(eventGraph);
		getNode->VariableReference.SetExternalMember(variableName, Cast<UClass>(StaticLoadObject(UClass::StaticClass(), nullptr, externalClassFilepath)));
		getNode->AllocateDefaultPins();
		eventGraph->AddNode(getNode);
		getNode->NodePosX = xPosition;
		getNode->NodePosY = yPosition;
		getNode->NodeGuid = FGuid::NewGuid();
		return getNode;
	}

	UK2Node_GetArrayItem* CreateBPArrayGetterNode(UEdGraph* functionGraph, int xPosition, int yPosition) {
		UK2Node_GetArrayItem* arrayGetNode = NewObject<UK2Node_GetArrayItem>(functionGraph);
		functionGraph->AddNode(arrayGetNode);
		arrayGetNode->NodePosX = xPosition;
		arrayGetNode->NodePosY = yPosition;
		arrayGetNode->AllocateDefaultPins();
		arrayGetNode->NodeGuid = FGuid::NewGuid();

		return arrayGetNode;
	}

	UK2Node_VariableSet* CreateBPSetterNode(UEdGraph* functionGraph, FName variableName, int xPosition, int yPosition) {
		UK2Node_VariableSet* setterNode = NewObject<UK2Node_VariableSet>(functionGraph);
		functionGraph->AddNode(setterNode, false, false);
		setterNode->VariableReference.SetSelfMember(variableName);
		setterNode->NodePosX = xPosition;
		setterNode->NodePosY = yPosition;
		setterNode->AllocateDefaultPins();
		setterNode->ReconstructNode();
		setterNode->NodeGuid = FGuid::NewGuid();

		return setterNode;
	}

	UK2Node_Select* CreateBPSelectNode(UEdGraph* functionGraph, int xPosition, int yPosition) {
		
		//Validate function graph
		if (!functionGraph)
		{
			UE_LOG(LogTemp, Error, TEXT("Function graph is nullptr! Cannot create Select node in function CreateBPSelectNode."));
			return nullptr;
		}

		UK2Node_Select* selectNode = NewObject<UK2Node_Select>(functionGraph);
		selectNode->SetFlags(RF_Transactional);
		selectNode->NodePosX = xPosition;
		selectNode->NodePosY = yPosition;
		selectNode->NodeGuid = FGuid::NewGuid();

		selectNode->ReconstructNode();

		FEdGraphPinType materialPinType;
		materialPinType.PinCategory = UEdGraphSchema_K2::PC_Object;
		materialPinType.PinSubCategoryObject = UMaterialInterface::StaticClass();

		for (UEdGraphPin* Pin : selectNode->Pins)
		{
			if (Pin->PinName.ToString().StartsWith("Option") || Pin->PinName == "ReturnValue")
			{
				Pin->PinType = materialPinType;
			}
		}

		functionGraph->AddNode(selectNode);


		return selectNode;
	}

	UK2Node_CallFunction* CreateBPSetMaterialNode(UEdGraph* functionGraph, int xPosition, int yPosition) {
		//Validate function graph
		if (!functionGraph)
		{
			UE_LOG(LogTemp, Error, TEXT("Function graph is nullptr! Cannot create Select node in function CreateBPSetMaterialNode."));
			return nullptr;
		}

		UK2Node_CallFunction* setMaterialNode = NewObject<UK2Node_CallFunction>(functionGraph);
		functionGraph->AddNode(setMaterialNode);
		setMaterialNode->NodePosX = xPosition;
		setMaterialNode->NodePosY = yPosition;
		setMaterialNode->NodeGuid = FGuid::NewGuid();
		setMaterialNode->FunctionReference.SetExternalMember(
			FName(TEXT("SetMaterial")),
			UPrimitiveComponent::StaticClass()
		);

		setMaterialNode->AllocateDefaultPins();

		return setMaterialNode;
	}

	UK2Node_CallFunction* CreateBPScalarParameterNode(UEdGraph* eventGraph, int xPosition, int yPosition) {
		UK2Node_CallFunction* scalarParameterNode = NewObject<UK2Node_CallFunction>(eventGraph);
		scalarParameterNode->FunctionReference.SetExternalMember(FName("SetScalarParameterValue"), UKismetMaterialLibrary::StaticClass());
		scalarParameterNode->AllocateDefaultPins();
		eventGraph->AddNode(scalarParameterNode);
		scalarParameterNode->NodePosX = xPosition;
		scalarParameterNode->NodePosY = yPosition;
		scalarParameterNode->NodeGuid = FGuid::NewGuid();

		return scalarParameterNode;
	}

	UK2Node_CallFunction* CreateBPDynamicMaterialInstanceScalarParameterNode(UEdGraph* eventGraph, int xPosition, int yPosition) {

		UFunction* function = UMaterialInstanceDynamic::StaticClass()->FindFunctionByName(FName("SetScalarParameterValue"));
		if (!function)
		{
			UE_LOG(LogTemp, Error, TEXT("Could not find UMaterialInstanceDynamic::SetScalarParameterValue"));
			return nullptr;
		}

		UK2Node_CallFunction* scalarParameterNode = NewObject<UK2Node_CallFunction>(eventGraph);
		scalarParameterNode->SetFromFunction(function);
		scalarParameterNode->AllocateDefaultPins();
		eventGraph->AddNode(scalarParameterNode, false, false);
		scalarParameterNode->NodePosX = xPosition;
		scalarParameterNode->NodePosY = yPosition;
		scalarParameterNode->ReconstructNode();
		scalarParameterNode->NodeGuid = FGuid::NewGuid();

		return scalarParameterNode;
	}

	UK2Node_CallFunction* CreateBPVectorParameterNode(UEdGraph* eventGraph, int xPosition, int yPosition) {
		UK2Node_CallFunction* vectorNode = NewObject<UK2Node_CallFunction>(eventGraph);
		vectorNode->FunctionReference.SetExternalMember(FName("SetVectorParameterValue"), UKismetMaterialLibrary::StaticClass());
		vectorNode->AllocateDefaultPins();
		eventGraph->AddNode(vectorNode);
		vectorNode->NodePosX = xPosition;
		vectorNode->NodePosY = yPosition;
		vectorNode->NodeGuid = FGuid::NewGuid();

		return vectorNode;
	}

	UK2Node_CallFunction* CreateBPNormalizeToRangeNode(UEdGraph* eventGraph , int xPosition, int yPosition) {
		UK2Node_CallFunction* normalizeToRangeNode = NewObject<UK2Node_CallFunction>(eventGraph);
		normalizeToRangeNode->FunctionReference.SetExternalMember(FName("NormalizeToRange"), UKismetMathLibrary::StaticClass());
		normalizeToRangeNode->AllocateDefaultPins();
		eventGraph->AddNode(normalizeToRangeNode);
		normalizeToRangeNode->NodePosX = xPosition;
		normalizeToRangeNode->NodePosY = yPosition;
		normalizeToRangeNode->NodeGuid = FGuid::NewGuid();

		return normalizeToRangeNode;
	}

	UK2Node_CallFunction* CreateBPMakeVectorNode(UEdGraph* eventGraph, int xPosition, int yPosition) {
		UK2Node_CallFunction* makeVectorNode = NewObject<UK2Node_CallFunction>(eventGraph);
		makeVectorNode->FunctionReference.SetExternalMember(
			GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, MakeVector),
			UKismetMathLibrary::StaticClass()
		);
		makeVectorNode->AllocateDefaultPins();
		eventGraph->AddNode(makeVectorNode);
		makeVectorNode->NodePosX = xPosition;
		makeVectorNode->NodePosY = yPosition;
		makeVectorNode->NodeGuid = FGuid::NewGuid();

		return makeVectorNode;
	}

	UK2Node_CallFunction* CreateBPSetRenderDepthNode(UEdGraph* functionGraph, bool defaultValue, int xPosition, int yPosition) {
		UK2Node_CallFunction* setRenderDepthNode = NewObject<UK2Node_CallFunction>(functionGraph);
		setRenderDepthNode->FunctionReference.SetExternalMember(FName("SetRenderCustomDepth"), UPrimitiveComponent::StaticClass());
		setRenderDepthNode->AllocateDefaultPins();
		functionGraph->AddNode(setRenderDepthNode);
		setRenderDepthNode->NodePosX = xPosition;
		setRenderDepthNode->NodePosY = yPosition;
		setRenderDepthNode->NodeGuid = FGuid::NewGuid();

		//Finds the value pin
		UEdGraphPin* valuePin = setRenderDepthNode->FindPin(FName("bValue"));

		//Sets the value pin as default value
		if (valuePin) {
			valuePin->DefaultValue = defaultValue ? TEXT("true") : TEXT("false");
		}

		return setRenderDepthNode;
	}

	UK2Node_CallFunction* CreateBPGetAllActorsOfClassNode(UEdGraph* functionGraph, int xPosition, int yPosition) {
		UK2Node_CallFunction* getAllActorsNode = NewObject<UK2Node_CallFunction>(functionGraph);
		getAllActorsNode->FunctionReference.SetExternalMember(FName("GetAllActorsOfClass"), UGameplayStatics::StaticClass());
		getAllActorsNode->NodePosX = xPosition;
		getAllActorsNode->NodePosY = yPosition;
		getAllActorsNode->NodeGuid = FGuid::NewGuid();
		getAllActorsNode->AllocateDefaultPins();
		functionGraph->AddNode(getAllActorsNode);

		return getAllActorsNode;
	}

	UK2Node_CallFunction* CreateBPDynamicMaterialInstanceNode(UEdGraph* functionGraph, int xPosition, int yPosition) {

		//Validate function graph
		if (!functionGraph) {
			UE_LOG(LogTemp, Error, TEXT("Function graph is a nullpointer, can't add node"));
			return nullptr;
		}

		//Get the target function
		UFunction* targetFunction = UKismetMaterialLibrary::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UKismetMaterialLibrary, CreateDynamicMaterialInstance));

		//Validate target function
		if (!targetFunction) {
			UE_LOG(LogTemp, Error, TEXT("Target function is a nullpointer, can't add node"));
			return nullptr;
		}

		//Create a new function node in the function graph
		UK2Node_CallFunction* dynamicMaterialInstanceNode = NewObject<UK2Node_CallFunction>(functionGraph);
		functionGraph->AddNode(dynamicMaterialInstanceNode, false, false);
		dynamicMaterialInstanceNode->SetFromFunction(targetFunction);
		dynamicMaterialInstanceNode->AllocateDefaultPins();
		dynamicMaterialInstanceNode->NodePosX = xPosition;
		dynamicMaterialInstanceNode->NodePosY = yPosition;
		dynamicMaterialInstanceNode->NodeGuid = FGuid::NewGuid();
		dynamicMaterialInstanceNode->ReconstructNode();

		return dynamicMaterialInstanceNode;

	}

	UK2Node_CallFunction* CreateBPCallFunctionNode(UEdGraph* eventGraph, FName functionName, int xPosition, int yPosition) {
		UK2Node_CallFunction* functionCallNode = NewObject<UK2Node_CallFunction>(eventGraph);
		eventGraph->AddNode(functionCallNode);
		functionCallNode->FunctionReference.SetSelfMember(functionName);
		functionCallNode->NodePosX = xPosition;
		functionCallNode->NodePosY = yPosition;
		functionCallNode->AllocateDefaultPins();
		functionCallNode->NodeGuid = FGuid::NewGuid();

		return functionCallNode;
	}

	void AddVariableToBlueprintClass(UBlueprint* blueprint, FName varName, FEdGraphPinType pinType, bool bInstanceEditable, FString defaultValue) {
	
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


	
}