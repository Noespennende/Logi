#include "ThermalController.h"

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
#include "LogiUtils.h"
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
#include "Utils/BlueprintUtils.h"


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

	//Find the MPC_Logi_ThermalSettings material parameter collection
	UMaterialParameterCollection* ThermalSettings = LoadObject<UMaterialParameterCollection>(
		nullptr,
		TEXT("/Game/Logi_ThermalCamera/Materials/MPC_Logi_ThermalSettings.MPC_Logi_ThermalSettings")
	);

	//Validate the material parameter collection
	if (!ThermalSettings) {
		UE_LOG(LogTemp, Error, TEXT("Failed to load the MPC_Logi_ThermalSettings material parameter collection"));
		return;
	}


	//Create a new branch node
	UK2Node_IfThenElse* BranchNode = Logi::BlueprintUtils::CreateBPBranchNode(eventGraph, 300, 420);

	//Connect the event tick node to the branch node
	UEdGraphPin* ExecPin = EventTick->FindPin(UEdGraphSchema_K2::PN_Then);
	UEdGraphPin* BranchExecPin = BranchNode->GetExecPin();
	if (ExecPin && BranchExecPin) {
		Schema->TryCreateConnection(ExecPin, BranchExecPin);
	}

	//Create ThermalCameraActive variable get node
	UK2Node_VariableGet* ThermalCameraActiveGetNode = Logi::BlueprintUtils::CreateBPGetterNode(eventGraph, FName("ThermalCameraActive"), 100, 550);

	//Connect the ThermalCameraActive variable get node to the branch node
	Schema->TryCreateConnection(ThermalCameraActiveGetNode->FindPin(FName("ThermalCameraActive")), BranchNode->GetConditionPin());


	//Create scalar parameter node - Thermal Camera Toggle true
	UK2Node_CallFunction* thermalToggleTrueScalarParameterNode = Logi::BlueprintUtils::CreateBPScalarParameterNode(eventGraph, 600, 200);
	//Set node input for parameterName and value for Thermal Camera Toggle true
	UEdGraphPin* ParamNamePin = thermalToggleTrueScalarParameterNode->FindPin(FName("ParameterName"));
	UEdGraphPin* ParamValuePin = thermalToggleTrueScalarParameterNode->FindPin(FName("ParameterValue"));
	UEdGraphPin* CollectionPin = thermalToggleTrueScalarParameterNode->FindPin(FName("Collection"));
	ParamNamePin->DefaultValue = "ThermalCameraToggle";
	ParamValuePin->DefaultValue = "1.0";
	CollectionPin->DefaultObject = ThermalSettings;

	//Create scalar parameter node - Thermal Camera Toggle false
	UK2Node_CallFunction* thermalToggleFalseScalarParameterNode = Logi::BlueprintUtils::CreateBPScalarParameterNode(eventGraph, 600, 600);
	//Set node input for parameterName and value for  ThermalCameraToggle False
	ParamNamePin = thermalToggleFalseScalarParameterNode->FindPin(FName("ParameterName"));
	ParamValuePin = thermalToggleFalseScalarParameterNode->FindPin(FName("ParameterValue"));
	CollectionPin = thermalToggleFalseScalarParameterNode->FindPin(FName("Collection"));
	ParamNamePin->DefaultValue = "ThermalCameraToggle";
	ParamValuePin->DefaultValue = "0.0";
	CollectionPin->DefaultObject = ThermalSettings;


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
		UK2Node_CallFunction* vectorNode = Logi::BlueprintUtils::CreateBPVectorParameterNode(eventGraph, nodePosition.X, nodePosition.Y);

		//Find the collection pin of the vector parameter node and set it to the ThermalSettings
		CollectionPin = vectorNode->FindPin(FName("Collection"));
		CollectionPin->DefaultObject = ThermalSettings;

		// Find the parameter pin for the vector parameter node and set the default value
		UEdGraphPin* ParamPin = vectorNode->FindPin(FName("ParameterName"));
		ParamPin->DefaultValue = Param;

		//create assosiated Getter node for value parameter and add it to the graph
		UK2Node_VariableGet* getNode = Logi::BlueprintUtils::CreateBPGetterNode(eventGraph, FName(*Param), (nodePosition.X - 100), (nodePosition.Y + 300));
	

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
		UK2Node_CallFunction* scalarNode = Logi::BlueprintUtils::CreateBPScalarParameterNode(eventGraph, nodePosition.X, nodePosition.Y);

		//Find the collection pin of the scalar parameter node and set it to the ThermalSettings
		CollectionPin = scalarNode->FindPin(FName("Collection"));
		CollectionPin->DefaultObject = ThermalSettings;

		// Find the parameterPin for the scalar node and set the default value
		UEdGraphPin* ParamPin = scalarNode->FindPin(FName("ParameterName"));
		ParamPin->DefaultValue = Param;

		//create assosiated Getter node for value parameter and add it to the graph
		UK2Node_VariableGet* getNode = Logi::BlueprintUtils::CreateBPGetterNode(eventGraph, FName(*Param), (nodePosition.X - 100), (nodePosition.Y + 300));

		//Spawn normalize to range node
		UK2Node_CallFunction* normalizeToRangeNode = Logi::BlueprintUtils::CreateBPNormalizeToRangeNode(eventGraph, (nodePosition.X), (nodePosition.Y + 350));
		//Find normalize to range nodes range min and max pins
		UEdGraphPin* rangeMinPin = normalizeToRangeNode->FindPin(FName("RangeMin"));
		UEdGraphPin* rangeMaxPin = normalizeToRangeNode->FindPin(FName("RangeMax"));

		//Spawn getter node for camera range and add them to the background temparature's normalize node
		if (Param == "BackgroundTemperature" || Param == "SkyTemperature") {
			//Spawn thermal camera range getters
			UK2Node_VariableGet* thermalCameraRangeMinNode = Logi::BlueprintUtils::CreateBPGetterNode(eventGraph, FName("ThermalCameraRangeMin"), getNode->NodePosX-150, (nodePosition.Y + 400));
			UK2Node_VariableGet* thermalCameraRangeMaxNode = Logi::BlueprintUtils::CreateBPGetterNode(eventGraph, FName("ThermalCameraRangeMax"), getNode->NodePosX-150, (nodePosition.Y + 500));


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
	UK2Node_VariableGet* noiseSizeGet = Logi::BlueprintUtils::CreateBPGetterNode(eventGraph, FName("NoiseSize"), nodePosition.X, (nodePosition.Y + 200));

	//Update node position
	nodePosition.X += 250;

	//Create makevector node to convert NoiseSize in to a 3D vector
	UK2Node_CallFunction* makeVectorNode = Logi::BlueprintUtils::CreateBPMakeVectorNode(eventGraph, nodePosition.X, (nodePosition.Y + 200));

	//Create NoiseVector setter node
	UK2Node_VariableSet* setVectorNode = Logi::BlueprintUtils::CreateBPSetterNode(eventGraph, FName("NoiseVector"), nodePosition.X, nodePosition.Y);


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
	UK2Node_CallFunction* noiseSizeVectorNode = Logi::BlueprintUtils::CreateBPVectorParameterNode(eventGraph, nodePosition.X, nodePosition.Y);

	//Find the collection pin of the noiseSize vector parameter node and set it to the ThermalSettings
	UEdGraphPin* noiseSizeCollectionPin = noiseSizeVectorNode->FindPin(FName("Collection"));
	noiseSizeCollectionPin->DefaultObject = ThermalSettings;

	// Set setVectorParameterValue node parameter name to NoiseSize
	UEdGraphPin* noiseSizevectorNodeParamPin = noiseSizeVectorNode->FindPin(FName("ParameterName"));
	noiseSizevectorNodeParamPin->DefaultValue = FString("NoiseSize");

	//Connect NoiseSize setVectorParameterValue node to setVector node
	Schema->TryCreateConnection(noiseSizeVectorNode->GetExecPin(), setVectorNode->GetThenPin());


	//Create getter node for the NoiseVector variable
	UK2Node_VariableGet* getNoiseVectorNode = Logi::BlueprintUtils::CreateBPGetterNode(eventGraph, FName("NoiseVector"), nodePosition.X, (nodePosition.Y + 300));

	//Connect the NoiseVector getter node to the NoiseSize setVectorParameterValue node
	Schema->TryCreateConnection(getNoiseVectorNode->FindPin(FName("NoiseVector")), noiseSizeVectorNode->FindPin(FName("ParameterValue")));


	//Compile the blueprint
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(blueprint);

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


 void ThermalController::CreateThermalController(bool& success, FString& statusMessage) {
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
	Logi::BlueprintUtils::AddVariableToBlueprintClass(thermalControllerBp, "ThermalCameraActive", BoolType, true, "false");
	Logi::BlueprintUtils::AddVariableToBlueprintClass(thermalControllerBp, "ThermalCameraRangeMin", FloatType, true, "0.0");
	Logi::BlueprintUtils::AddVariableToBlueprintClass(thermalControllerBp, "ThermalCameraRangeMax", FloatType, true, "25.0");
	Logi::BlueprintUtils::AddVariableToBlueprintClass(thermalControllerBp, "BackgroundTemperature", FloatType, true, "5.0");
	Logi::BlueprintUtils::AddVariableToBlueprintClass(thermalControllerBp, "SkyTemperature", FloatType, true, "5.0");
	Logi::BlueprintUtils::AddVariableToBlueprintClass(thermalControllerBp, "Blur", FloatType, true, "100");
	Logi::BlueprintUtils::AddVariableToBlueprintClass(thermalControllerBp, "NoiseSize", FloatType, true, "1.0");
	Logi::BlueprintUtils::AddVariableToBlueprintClass(thermalControllerBp, "NoiseAmount", FloatType, true, "5.0");
	Logi::BlueprintUtils::AddVariableToBlueprintClass(thermalControllerBp, "Cold", LinearColorType, true, "R=0.0,G=0.0,B=0.0,A=1.0");
	Logi::BlueprintUtils::AddVariableToBlueprintClass(thermalControllerBp, "Mid", LinearColorType, true, "R=0.5,G=0.5,B=0.5,A=1.0");
	Logi::BlueprintUtils::AddVariableToBlueprintClass(thermalControllerBp, "Hot", LinearColorType, true, "R=1.0,G=1.0,B=1.0,A=1.0");
	Logi::BlueprintUtils::AddVariableToBlueprintClass(thermalControllerBp, "NoiseVector", VectorType, false, "0, 0, 0");


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

	bool bSuccess = Logi::LogiUtils::SaveAssetToDisk(thermalControllerBp);

	if (bSuccess)
	{
		
		statusMessage = FString::Printf(TEXT("Blueprint %s created and successfully saved to: %s"), *thermalControllerBp->GetName(), *thermalControllerBp->GetPathName());
		success = true;
	}
	else
	{
		statusMessage = FString::Printf(TEXT("Blueprint %s created, but failed to save properly. Manual save will be required: %s"), *thermalControllerBp->GetName(), *thermalControllerBp->GetPathName());
		success = true;
	}
}