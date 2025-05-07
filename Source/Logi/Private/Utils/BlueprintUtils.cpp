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
#include "LogiOutliner.h"
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
	FName AddMaterialInstanceVariableToBlueprint(UBlueprint* Blueprint) {

	//Validate blueprint
	if (Blueprint == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Adding variable to blueprint failed because the blueprint is null"));
		return FName("");
	}

	//Create a new variable in the blueprint
	const FName VariableName = FBlueprintEditorUtils::FindUniqueKismetName(Blueprint, TEXT("Logi_DynamicMaterialInstance"));
	FEdGraphPinType VariableType;
	VariableType.PinCategory = UEdGraphSchema_K2::PC_Object;
	VariableType.PinSubCategoryObject = UMaterialInstanceDynamic::StaticClass();

	//Add variable to the blueprint
	FBlueprintEditorUtils::AddMemberVariable(Blueprint, VariableName, VariableType);

	return VariableName;
}
	
	void AddThermalControllerReferenceToBlueprint(UBlueprint* Blueprint,const FName& VarName, const bool bInstanceEditable) {

		if (!Blueprint) {
			UE_LOG(LogTemp, Error, TEXT("Blueprint is null, can't add reference"));
			return;
		}

		if (FBlueprintEditorUtils::FindNewVariableIndex(Blueprint, VarName) != INDEX_NONE) {
			UE_LOG(LogTemp, Warning, TEXT("Variable '%s' already exists in blueprint '%s'"), *VarName.ToString(), *Blueprint->GetName());
			return;
		}

		// Load the blueprint asset (not just the generated class directly)
		UBlueprint* ControllerBP = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, TEXT("/Game/Logi_ThermalCamera/Actors/BP_Logi_ThermalController.BP_Logi_ThermalController")));
		if (!ControllerBP || !ControllerBP->GeneratedClass) {
			UE_LOG(LogTemp, Error, TEXT("Could not load BP_Logi_ThermalController Blueprint or its GeneratedClass"));
			return;
		}

		// Use the generated class (this ensures Blueprint properties are recognized)
		UClass* ControllerClass = ControllerBP->GeneratedClass;

		// Create pin type
		FEdGraphPinType ControllerRefType;
		ControllerRefType.PinCategory = UEdGraphSchema_K2::PC_Object;
		ControllerRefType.PinSubCategoryObject = ControllerClass;

		// Add the variable
		FBlueprintEditorUtils::AddMemberVariable(Blueprint, VarName, ControllerRefType);

		//Set variable as blueprint editable
		FBlueprintEditorUtils::SetBlueprintOnlyEditableFlag(Blueprint, VarName, true);

		// Make it instance editable
		if (bInstanceEditable) {
			FBlueprintEditorUtils::SetBlueprintOnlyEditableFlag(Blueprint, VarName, !bInstanceEditable);
			FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint, VarName, nullptr, FBlueprintMetadata::MD_Private, TEXT("false"));
			FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint, VarName, nullptr, TEXT("EditAnywhere"), TEXT("true"));
			FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint, VarName, nullptr, FBlueprintMetadata::MD_ExposeOnSpawn, TEXT("true"));
		}
	}

	UEdGraphNode* AddNodeToBlueprint(UBlueprint* Blueprint, const FName& FunctionName, UClass* Class, const FVector& Location)
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

	UEdGraphNode* AddNodeToBlueprintFunction(UEdGraph* FunctionGraph, const FName& FunctionName, UClass* NodeClass, const FVector& Location) {

		//validate function graph
		if (!FunctionGraph) {
			UE_LOG(LogTemp, Error, TEXT("Function graph is a nullpointer, can't add node"));
			return nullptr;
		}

		//Find node class function
		UFunction* Function = NodeClass->FindFunctionByName(FunctionName);

		//Validate node class function
		if (!Function)
		{
			// The function does not exist in the blueprint class, handle the error
			UE_LOG(LogTemp, Warning, TEXT("node class function not found"));
			return nullptr;
		}

		// Create a new function node in the function graph
		UEdGraphNode* NewNode = NewObject<UEdGraphNode>(FunctionGraph);

		//Create spawner to spawn the node
		UBlueprintFunctionNodeSpawner* FunctionNodeSpawner = UBlueprintFunctionNodeSpawner::Create(Function);

		//Spawn the node
		if (FunctionNodeSpawner)
		{
			FunctionNodeSpawner->SetFlags(RF_Transactional);
			NewNode = FunctionNodeSpawner->Invoke(FunctionGraph, IBlueprintNodeBinder::FBindingSet(), FVector2D(Location.X, Location.Y));
			return NewNode;
		}

		return nullptr;
	}

	UK2Node_IfThenElse* CreateBPBranchNode(UEdGraph* EventGraph, int XPosition, int YPosition) {
		UK2Node_IfThenElse* BranchNode = NewObject<UK2Node_IfThenElse>(EventGraph);
		BranchNode->AllocateDefaultPins();
		EventGraph->AddNode(BranchNode);
		BranchNode->NodePosX = XPosition;
		BranchNode->NodePosY = YPosition;
		BranchNode->NodeGuid = FGuid::NewGuid();

		return BranchNode;
	}

	UK2Node_VariableGet* CreateBPGetterNode(UEdGraph* EventGraph, const FName& VariableName, int XPosition, int YPosition) {
		UK2Node_VariableGet* GetNode = NewObject<UK2Node_VariableGet>(EventGraph);
		GetNode->VariableReference.SetSelfMember(VariableName);
		GetNode->AllocateDefaultPins();
		EventGraph->AddNode(GetNode);
		GetNode->NodePosX = XPosition;
		GetNode->NodePosY = YPosition;
		GetNode->NodeGuid = FGuid::NewGuid();

		return GetNode;
	}

	UK2Node_VariableGet* CreateBPExternalGetterNode(UEdGraph* EventGraph, const FName& VariableName, const TCHAR* ExternalClassFilepath, int XPosition, int YPosition) {
		UK2Node_VariableGet* GetNode = NewObject<UK2Node_VariableGet>(EventGraph);
		GetNode->VariableReference.SetExternalMember(VariableName, Cast<UClass>(StaticLoadObject(UClass::StaticClass(), nullptr, ExternalClassFilepath)));
		GetNode->AllocateDefaultPins();
		EventGraph->AddNode(GetNode);
		GetNode->NodePosX = XPosition;
		GetNode->NodePosY = YPosition;
		GetNode->NodeGuid = FGuid::NewGuid();
		return GetNode;
	}

	UK2Node_GetArrayItem* CreateBPArrayGetterNode(UEdGraph* FunctionGraph, int XPosition, int YPosition) {
		UK2Node_GetArrayItem* ArrayGetNode = NewObject<UK2Node_GetArrayItem>(FunctionGraph);
		FunctionGraph->AddNode(ArrayGetNode);
		ArrayGetNode->NodePosX = XPosition;
		ArrayGetNode->NodePosY = YPosition;
		ArrayGetNode->AllocateDefaultPins();
		ArrayGetNode->NodeGuid = FGuid::NewGuid();

		return ArrayGetNode;
	}

	UK2Node_VariableSet* CreateBPSetterNode(UEdGraph* FunctionGraph, const FName& VariableName, int XPosition, int YPosition) {
		UK2Node_VariableSet* SetterNode = NewObject<UK2Node_VariableSet>(FunctionGraph);
		FunctionGraph->AddNode(SetterNode, false, false);
		SetterNode->VariableReference.SetSelfMember(VariableName);
		SetterNode->NodePosX = XPosition;
		SetterNode->NodePosY = YPosition;
		SetterNode->AllocateDefaultPins();
		SetterNode->ReconstructNode();
		SetterNode->NodeGuid = FGuid::NewGuid();

		return SetterNode;
	}

	UK2Node_Select* CreateBPSelectNode(UEdGraph* FunctionGraph, int XPosition, int YPosition) {
		
		UE_LOG(LogTemp, Error, TEXT("HIIIT"));

		//Validate function graph
		if (!FunctionGraph)
		{
			UE_LOG(LogTemp, Error, TEXT("Function graph is nullptr! Cannot create Select node in function CreateBPSelectNode."));
			return nullptr;
		}

		UK2Node_Select* selectNode = NewObject<UK2Node_Select>(FunctionGraph);
		selectNode->SetFlags(RF_Transactional);
		selectNode->NodePosX = XPosition;
		selectNode->NodePosY = YPosition;
		selectNode->NodeGuid = FGuid::NewGuid();

		selectNode->ReconstructNode();

		FEdGraphPinType MaterialPinType;
		MaterialPinType.PinCategory = UEdGraphSchema_K2::PC_Object;
		MaterialPinType.PinSubCategoryObject = UMaterialInterface::StaticClass();

		for (UEdGraphPin* Pin : selectNode->Pins)
		{
			if (Pin->PinName.ToString().StartsWith("Option") || Pin->PinName == "ReturnValue")
			{
				Pin->Modify();
				Pin->PinType = MaterialPinType;
			}
		}

		FunctionGraph->AddNode(selectNode);


		return selectNode;
	}

	UK2Node_CallFunction* CreateBPSetMaterialNode(UEdGraph* FunctionGraph, int XPosition, int YPosition) {
		//Validate function graph
		if (!FunctionGraph)
		{
			UE_LOG(LogTemp, Error, TEXT("Function graph is nullptr! Cannot create Select node in function CreateBPSetMaterialNode."));
			return nullptr;
		}

		UK2Node_CallFunction* SetMaterialNode = NewObject<UK2Node_CallFunction>(FunctionGraph);
		FunctionGraph->AddNode(SetMaterialNode);
		SetMaterialNode->NodePosX = XPosition;
		SetMaterialNode->NodePosY = YPosition;
		SetMaterialNode->NodeGuid = FGuid::NewGuid();
		SetMaterialNode->FunctionReference.SetExternalMember(
			FName(TEXT("SetMaterial")),
			UPrimitiveComponent::StaticClass()
		);

		SetMaterialNode->AllocateDefaultPins();

		return SetMaterialNode;
	}

	UK2Node_CallFunction* CreateBPScalarParameterNode(UEdGraph* EventGraph, int XPosition, int YPosition) {
		UK2Node_CallFunction* ScalarParameterNode = NewObject<UK2Node_CallFunction>(EventGraph);
		ScalarParameterNode->FunctionReference.SetExternalMember(FName("SetScalarParameterValue"), UKismetMaterialLibrary::StaticClass());
		ScalarParameterNode->AllocateDefaultPins();
		EventGraph->AddNode(ScalarParameterNode);
		ScalarParameterNode->NodePosX = XPosition;
		ScalarParameterNode->NodePosY = YPosition;
		ScalarParameterNode->NodeGuid = FGuid::NewGuid();

		return ScalarParameterNode;
	}

	UK2Node_CallFunction* CreateBPDynamicMaterialInstanceScalarParameterNode(UEdGraph* EventGraph, int XPosition, int YPosition) {

		UFunction* Function = UMaterialInstanceDynamic::StaticClass()->FindFunctionByName(FName("SetScalarParameterValue"));
		if (!Function)
		{
			UE_LOG(LogTemp, Error, TEXT("Could not find UMaterialInstanceDynamic::SetScalarParameterValue"));
			return nullptr;
		}

		UK2Node_CallFunction* ScalarParameterNode = NewObject<UK2Node_CallFunction>(EventGraph);
		ScalarParameterNode->SetFromFunction(Function);
		ScalarParameterNode->AllocateDefaultPins();
		EventGraph->AddNode(ScalarParameterNode, false, false);
		ScalarParameterNode->NodePosX = XPosition;
		ScalarParameterNode->NodePosY = YPosition;
		ScalarParameterNode->ReconstructNode();
		ScalarParameterNode->NodeGuid = FGuid::NewGuid();

		return ScalarParameterNode;
	}

	UK2Node_CallFunction* CreateBPVectorParameterNode(UEdGraph* EventGraph, int XPosition, int YPosition) {
		UK2Node_CallFunction* VectorNode = NewObject<UK2Node_CallFunction>(EventGraph);
		VectorNode->FunctionReference.SetExternalMember(FName("SetVectorParameterValue"), UKismetMaterialLibrary::StaticClass());
		VectorNode->AllocateDefaultPins();
		EventGraph->AddNode(VectorNode);
		VectorNode->NodePosX = XPosition;
		VectorNode->NodePosY = YPosition;
		VectorNode->NodeGuid = FGuid::NewGuid();

		return VectorNode;
	}

	UK2Node_CallFunction* CreateBPNormalizeToRangeNode(UEdGraph* EventGraph , int XPosition, int YPosition) {
		UK2Node_CallFunction* NormalizeToRangeNode = NewObject<UK2Node_CallFunction>(EventGraph);
		NormalizeToRangeNode->FunctionReference.SetExternalMember(FName("NormalizeToRange"), UKismetMathLibrary::StaticClass());
		NormalizeToRangeNode->AllocateDefaultPins();
		EventGraph->AddNode(NormalizeToRangeNode);
		NormalizeToRangeNode->NodePosX = XPosition;
		NormalizeToRangeNode->NodePosY = YPosition;
		NormalizeToRangeNode->NodeGuid = FGuid::NewGuid();

		return NormalizeToRangeNode;
	}

	UK2Node_CallFunction* CreateBPMakeVectorNode(UEdGraph* EventGraph, int XPosition, int YPosition) {
		UK2Node_CallFunction* MakeVectorNode = NewObject<UK2Node_CallFunction>(EventGraph);
		MakeVectorNode->FunctionReference.SetExternalMember(
			GET_FUNCTION_NAME_CHECKED(UKismetMathLibrary, MakeVector),
			UKismetMathLibrary::StaticClass()
		);
		MakeVectorNode->AllocateDefaultPins();
		EventGraph->AddNode(MakeVectorNode);
		MakeVectorNode->NodePosX = XPosition;
		MakeVectorNode->NodePosY = YPosition;
		MakeVectorNode->NodeGuid = FGuid::NewGuid();

		return MakeVectorNode;
	}

	UK2Node_CallFunction* CreateBPSetRenderDepthNode(UEdGraph* FunctionGraph, bool bDefaultValue, int XPosition, int YPosition) {
		UK2Node_CallFunction* SetRenderDepthNode = NewObject<UK2Node_CallFunction>(FunctionGraph);
		SetRenderDepthNode->FunctionReference.SetExternalMember(FName("SetRenderCustomDepth"), UPrimitiveComponent::StaticClass());
		SetRenderDepthNode->AllocateDefaultPins();
		FunctionGraph->AddNode(SetRenderDepthNode);
		SetRenderDepthNode->NodePosX = XPosition;
		SetRenderDepthNode->NodePosY = YPosition;
		SetRenderDepthNode->NodeGuid = FGuid::NewGuid();

		//Finds the value pin
		UEdGraphPin* valuePin = SetRenderDepthNode->FindPin(FName("bValue"));

		//Sets the value pin as default value
		if (valuePin) {
			valuePin->DefaultValue = bDefaultValue ? TEXT("true") : TEXT("false");
		}

		return SetRenderDepthNode;
	}

	UK2Node_CallFunction* CreateBPGetAllActorsOfClassNode(UEdGraph* FunctionGraph, int XPosition, int YPosition) {
		UK2Node_CallFunction* GetAllActorsNode = NewObject<UK2Node_CallFunction>(FunctionGraph);
		GetAllActorsNode->FunctionReference.SetExternalMember(FName("GetAllActorsOfClass"), UGameplayStatics::StaticClass());
		GetAllActorsNode->NodePosX = XPosition;
		GetAllActorsNode->NodePosY = YPosition;
		GetAllActorsNode->NodeGuid = FGuid::NewGuid();
		GetAllActorsNode->AllocateDefaultPins();
		FunctionGraph->AddNode(GetAllActorsNode);

		return GetAllActorsNode;
	}

	UK2Node_CallFunction* CreateBPDynamicMaterialInstanceNode(UEdGraph* FunctionGraph, int XPosition, int YPosition) {

		//Validate function graph
		if (!FunctionGraph) {
			UE_LOG(LogTemp, Error, TEXT("Function graph is a nullpointer, can't add node"));
			return nullptr;
		}

		//Get the target function
		UFunction* TargetFunction = UKismetMaterialLibrary::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UKismetMaterialLibrary, CreateDynamicMaterialInstance));

		//Validate target function
		if (!TargetFunction) {
			UE_LOG(LogTemp, Error, TEXT("Target function is a nullpointer, can't add node"));
			return nullptr;
		}

		//Create a new function node in the function graph
		UK2Node_CallFunction* DynamicMaterialInstanceNode = NewObject<UK2Node_CallFunction>(FunctionGraph);
		FunctionGraph->AddNode(DynamicMaterialInstanceNode, false, false);
		DynamicMaterialInstanceNode->SetFromFunction(TargetFunction);
		DynamicMaterialInstanceNode->AllocateDefaultPins();
		DynamicMaterialInstanceNode->NodePosX = XPosition;
		DynamicMaterialInstanceNode->NodePosY = YPosition;
		DynamicMaterialInstanceNode->NodeGuid = FGuid::NewGuid();
		DynamicMaterialInstanceNode->ReconstructNode();

		return DynamicMaterialInstanceNode;

	}

	UK2Node_CallFunction* CreateBPCallFunctionNode(UEdGraph* EventGraph, const FName& FunctionName, int XPosition, int YPosition) {
		UK2Node_CallFunction* FunctionCallNode = NewObject<UK2Node_CallFunction>(EventGraph);
		EventGraph->AddNode(FunctionCallNode);
		FunctionCallNode->FunctionReference.SetSelfMember(FunctionName);
		FunctionCallNode->NodePosX = XPosition;
		FunctionCallNode->NodePosY = YPosition;
		FunctionCallNode->AllocateDefaultPins();
		FunctionCallNode->NodeGuid = FGuid::NewGuid();

		return FunctionCallNode;
	}

	void AddVariableToBlueprintClass(UBlueprint* Blueprint, const FName& VarName, const FEdGraphPinType& PinType, bool bInstanceEditable, const FString& DefaultValue) {
	
		//Validate blueprint
		if (Blueprint == nullptr) {
			UE_LOG(LogTemp, Error, TEXT("Adding variable to blueprint failed because the blueprint is null"));
			return;
		}

		// Check if the variable already exists, if so it skips the creation
		if (FBlueprintEditorUtils::FindNewVariableIndex(Blueprint, VarName) != INDEX_NONE) {
			UE_LOG(LogTemp, Warning, TEXT("Variable '%s' already exists in the blueprint '%s', skipping the creation of variable."), *VarName.ToString(), *Blueprint->GetName());
			return;
		}


		FBlueprintEditorUtils::AddMemberVariable(Blueprint, VarName, PinType, DefaultValue);


		//Sets innstance editable
		if (bInstanceEditable) {
			FBlueprintEditorUtils::SetBlueprintOnlyEditableFlag(Blueprint, VarName, !bInstanceEditable);
			FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint, VarName, nullptr, FBlueprintMetadata::MD_Private, TEXT("false"));
			FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint, VarName, nullptr, "MD_EditAnywhere", TEXT("true"));
			FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint, VarName, nullptr, FBlueprintMetadata::MD_ExposeOnSpawn, TEXT("true"));

		}
	}


	
}