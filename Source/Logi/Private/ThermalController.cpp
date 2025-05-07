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
#include "Utils/BlueprintUtils.h"

namespace Logi::ThermalController
{
	
	void CreateThermalCameraControllerNodeSetup(UBlueprint* Blueprint) {

		//Validate blueprint
		if (Blueprint == nullptr) {
			UE_LOG(LogTemp, Error, TEXT("Creating Thermal Camera Controller Node Setup failed because the blueprint is null"));
			return;
		}

		//Finds the event graph for the blueprint
		UEdGraph* EventGraph = nullptr;
		
		for (UEdGraph* Graph : Blueprint->UbergraphPages) {
			if (Graph->GetFName() == FName(TEXT("EventGraph"))) {
				EventGraph = Graph;
				break;
			}
		}

		//Validates the blueprint event graph
		if (EventGraph == nullptr) {
			UE_LOG(LogTemp, Error, TEXT("Creating Thermal Camera Controller Node Setup failed because the blueprint graph is null"));
			return;
		}

		//Get the event graph schema
		const UEdGraphSchema_K2* Schema = CastChecked<UEdGraphSchema_K2>(EventGraph->GetSchema());

		//Find the event tick node in the event graph
		UK2Node_Event* EventTick = nullptr;
		
		//Iterate over all the nodes in the blueprints event graph
		for (UEdGraphNode* Node : EventGraph->Nodes) {
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
		UK2Node_IfThenElse* BranchNode = BlueprintUtils::CreateBPBranchNode(EventGraph, 300, 420);

		//Connect the event tick node to the branch node
		UEdGraphPin* ExecPin = EventTick->FindPin(UEdGraphSchema_K2::PN_Then);
		UEdGraphPin* BranchExecPin = BranchNode->GetExecPin();

		if (ExecPin && BranchExecPin) {
			Schema->TryCreateConnection(ExecPin, BranchExecPin);
		}

		//Create ThermalCameraActive variable get node
		UK2Node_VariableGet* ThermalCameraActiveGetNode = BlueprintUtils::CreateBPGetterNode(EventGraph, FName("ThermalCameraActive"), 100, 550);

		//Connect the ThermalCameraActive variable get node to the branch node
		Schema->TryCreateConnection(ThermalCameraActiveGetNode->FindPin(FName("ThermalCameraActive")), BranchNode->GetConditionPin());


		//Create scalar parameter node - Thermal Camera Toggle true
		UK2Node_CallFunction* ThermalToggleTrueScalarParameterNode = BlueprintUtils::CreateBPScalarParameterNode(EventGraph, 600, 200);
		//Set node input for parameterName and value for Thermal Camera Toggle true
		UEdGraphPin* ParamNamePin = ThermalToggleTrueScalarParameterNode->FindPin(FName("ParameterName"));
		UEdGraphPin* ParamValuePin = ThermalToggleTrueScalarParameterNode->FindPin(FName("ParameterValue"));
		UEdGraphPin* CollectionPin = ThermalToggleTrueScalarParameterNode->FindPin(FName("Collection"));
		ParamNamePin->DefaultValue = "ThermalCameraToggle";
		ParamValuePin->DefaultValue = "1.0";
		CollectionPin->DefaultObject = ThermalSettings;

		//Create scalar parameter node - Thermal Camera Toggle false
		UK2Node_CallFunction* ThermalToggleFalseScalarParameterNode = BlueprintUtils::CreateBPScalarParameterNode(EventGraph, 600, 600);
		//Set node input for parameterName and value for  ThermalCameraToggle False
		ParamNamePin = ThermalToggleFalseScalarParameterNode->FindPin(FName("ParameterName"));
		ParamValuePin = ThermalToggleFalseScalarParameterNode->FindPin(FName("ParameterValue"));
		CollectionPin = ThermalToggleFalseScalarParameterNode->FindPin(FName("Collection"));
		ParamNamePin->DefaultValue = "ThermalCameraToggle";
		ParamValuePin->DefaultValue = "0.0";
		CollectionPin->DefaultObject = ThermalSettings;


		//Connect thermal camera toggle parameter node to branch nodes true execution pin
		Schema->TryCreateConnection(BranchNode->GetThenPin(), ThermalToggleTrueScalarParameterNode->GetExecPin());

		//Connect thermal camera toggle parameter node to branch nodes false execution pin
		Schema->TryCreateConnection(BranchNode->GetElsePin(), ThermalToggleFalseScalarParameterNode->GetExecPin());

		// Create vector parameter nodes for Cold, Mid, Hot, BackgroundTemp temperatures
		FVector2D NodePosition(1000, 420);
		const TArray<FString> VectorParams = { "Cold", "Mid", "Hot"};
		UK2Node_CallFunction* PreviousNode = nullptr;

		//Create vector parameter nodes
		for (const FString& Param : VectorParams) {
			//Create a vector parameter node
			UK2Node_CallFunction* VectorNode = BlueprintUtils::CreateBPVectorParameterNode(EventGraph, NodePosition.X, NodePosition.Y);

			//Find the collection pin of the vector parameter node and set it to the ThermalSettings
			CollectionPin = VectorNode->FindPin(FName("Collection"));
			CollectionPin->DefaultObject = ThermalSettings;

			// Find the parameter pin for the vector parameter node and set the default value
			UEdGraphPin* ParamPin = VectorNode->FindPin(FName("ParameterName"));
			ParamPin->DefaultValue = Param;

			//create assosiated Getter node for value parameter and add it to the graph
			UK2Node_VariableGet* GetNode = BlueprintUtils::CreateBPGetterNode(EventGraph, FName(*Param), (NodePosition.X - 100), (NodePosition.Y + 300));
			
			// connect the parameter nodes
			if (Param == "Cold") {
				//Connect Cold scalar paremeter node to the branch node
				Schema->TryCreateConnection(ThermalToggleTrueScalarParameterNode->GetThenPin(), VectorNode->GetExecPin());
				Schema->TryCreateConnection(ThermalToggleFalseScalarParameterNode->GetThenPin(), VectorNode->GetExecPin());
			}
			else {
				Schema->TryCreateConnection(VectorNode->GetExecPin(), PreviousNode->GetThenPin());
			}


			// Connect get node to the vector parameter node
			Schema->TryCreateConnection(GetNode->FindPin(FName(*Param)), VectorNode->FindPin(FName("ParameterValue")));
			
			
			//Move node to the right for each iteration
			NodePosition.X += 400;

			//Set previous node
			PreviousNode = VectorNode;
		}


		//Create Scalar parameter nodes
		const TArray<FString> ScalarParams = { "BackgroundTemperature","SkyTemperature", "Blur", "NoiseAmount"};
		for (const FString& Param : ScalarParams) {

			//Create a scalar parameter node
			UK2Node_CallFunction* ScalarNode = BlueprintUtils::CreateBPScalarParameterNode(EventGraph, NodePosition.X, NodePosition.Y);

			//Find the collection pin of the scalar parameter node and set it to the ThermalSettings
			CollectionPin = ScalarNode->FindPin(FName("Collection"));
			CollectionPin->DefaultObject = ThermalSettings;

			// Find the parameterPin for the scalar node and set the default value
			UEdGraphPin* ParamPin = ScalarNode->FindPin(FName("ParameterName"));
			ParamPin->DefaultValue = Param;

			//create assosiated Getter node for value parameter and add it to the graph
			UK2Node_VariableGet* GetNode = BlueprintUtils::CreateBPGetterNode(EventGraph, FName(*Param), (NodePosition.X - 100), (NodePosition.Y + 300));

			//Spawn normalize to range node
			UK2Node_CallFunction* NormalizeToRangeNode = BlueprintUtils::CreateBPNormalizeToRangeNode(EventGraph, (NodePosition.X), (NodePosition.Y + 350));
			//Find normalize to range nodes range min and max pins
			UEdGraphPin* RangeMinPin = NormalizeToRangeNode->FindPin(FName("RangeMin"));
			UEdGraphPin* RangeMaxPin = NormalizeToRangeNode->FindPin(FName("RangeMax"));

			//Spawn getter node for camera range and add them to the background temparature's normalize node
			if (Param == "BackgroundTemperature" || Param == "SkyTemperature") {
				//Spawn thermal camera range getters
				UK2Node_VariableGet* ThermalCameraRangeMinNode = BlueprintUtils::CreateBPGetterNode(EventGraph, FName("ThermalCameraRangeMin"), GetNode->NodePosX-150, (NodePosition.Y + 400));
				UK2Node_VariableGet* ThermalCameraRangeMaxNode = BlueprintUtils::CreateBPGetterNode(EventGraph, FName("ThermalCameraRangeMax"), GetNode->NodePosX-150, (NodePosition.Y + 500));


				//Connect getter nodes to normalize to range node
				Schema->TryCreateConnection(ThermalCameraRangeMinNode->FindPin(FName("ThermalCameraRangeMin")), NormalizeToRangeNode->FindPin(FName("RangeMin")));
				Schema->TryCreateConnection(ThermalCameraRangeMaxNode->FindPin(FName("ThermalCameraRangeMax")), NormalizeToRangeNode->FindPin(FName("RangeMax")));
			}
			else {
				//Set normalize range's min and max values
				RangeMinPin->DefaultValue = TEXT("0.0");
				RangeMaxPin->DefaultValue = TEXT("100.0");
			}

			//Connect the getter to the normalize node
			Schema->TryCreateConnection(GetNode->FindPin(FName(*Param)), NormalizeToRangeNode->FindPin(FName("Value")));

			//Connect the normalize node to the scalar node
			Schema->TryCreateConnection(NormalizeToRangeNode->FindPin(FName("ReturnValue")), ScalarNode->FindPin(FName("ParameterValue")));

			//Connect the scalar node to the previous node
			Schema->TryCreateConnection(ScalarNode->GetExecPin(), PreviousNode->GetThenPin());
			
			//Move node to the right for each iteration
			NodePosition.X += 400;

			//Set previous node
			PreviousNode = ScalarNode;


		}

		//create NoiseSize variable getter node
		UK2Node_VariableGet* NoiseSizeGet = BlueprintUtils::CreateBPGetterNode(EventGraph, FName("NoiseSize"), NodePosition.X, (NodePosition.Y + 200));

		//Update node position
		NodePosition.X += 250;

		//Create makevector node to convert NoiseSize in to a 3D vector
		UK2Node_CallFunction* MakeVectorNode = BlueprintUtils::CreateBPMakeVectorNode(EventGraph, NodePosition.X, (NodePosition.Y + 200));

		//Create NoiseVector setter node
		UK2Node_VariableSet* SetVectorNode = BlueprintUtils::CreateBPSetterNode(EventGraph, FName("NoiseVector"), NodePosition.X, NodePosition.Y);


		//Connect NoiseSize to MakeVector.X/Y/Z
		UEdGraphPin* NoiseOut = NoiseSizeGet->GetValuePin();
		Schema->TryCreateConnection(NoiseOut, MakeVectorNode->FindPin(FName("X")));
		Schema->TryCreateConnection(NoiseOut, MakeVectorNode->FindPin(FName("Y")));
		Schema->TryCreateConnection(NoiseOut, MakeVectorNode->FindPin(FName("Z")));

		//Connect makeVector to setVector node
		Schema->TryCreateConnection(MakeVectorNode->GetReturnValuePin(), SetVectorNode->FindPin(FName("NoiseVector")));
		
		//Connect setVector node to previous node
		Schema->TryCreateConnection(SetVectorNode->GetExecPin(), PreviousNode->GetThenPin());

		//Update node position
		NodePosition.X += 250;

		//Create Noisesize setVectorParameterValue node
		UK2Node_CallFunction* NoiseSizeVectorNode = BlueprintUtils::CreateBPVectorParameterNode(EventGraph, NodePosition.X, NodePosition.Y);

		//Find the collection pin of the noiseSize vector parameter node and set it to the ThermalSettings
		UEdGraphPin* NoiseSizeCollectionPin = NoiseSizeVectorNode->FindPin(FName("Collection"));
		NoiseSizeCollectionPin->DefaultObject = ThermalSettings;

		// Set setVectorParameterValue node parameter name to NoiseSize
		UEdGraphPin* NoiseSizevectorNodeParamPin = NoiseSizeVectorNode->FindPin(FName("ParameterName"));
		NoiseSizevectorNodeParamPin->DefaultValue = FString("NoiseSize");

		//Connect NoiseSize setVectorParameterValue node to setVector node
		Schema->TryCreateConnection(NoiseSizeVectorNode->GetExecPin(), SetVectorNode->GetThenPin());


		//Create getter node for the NoiseVector variable
		UK2Node_VariableGet* GetNoiseVectorNode = BlueprintUtils::CreateBPGetterNode(EventGraph, FName("NoiseVector"), NodePosition.X, (NodePosition.Y + 300));

		//Connect the NoiseVector getter node to the NoiseSize setVectorParameterValue node
		Schema->TryCreateConnection(GetNoiseVectorNode->FindPin(FName("NoiseVector")), NoiseSizeVectorNode->FindPin(FName("ParameterValue")));


		//Compile the blueprint
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

	}

	UBlueprint* CreateBlueprintClass(const FString& FilePath, TSubclassOf<UObject> ParentClass, bool& bOutSuccess, FString& StatusMessage) {

		//Check if the file path is valid
		if (StaticLoadObject(UObject::StaticClass(), nullptr, *FilePath) != nullptr) {
			bOutSuccess = false;
			StatusMessage = FString::Printf(TEXT("Failed to create blueprint - an asset already exists at that location  '%s'"), * FilePath);
			return nullptr;
		}

		// Check if parent class is valid
		if (!FKismetEditorUtilities::CanCreateBlueprintOfClass(ParentClass)) {
			bOutSuccess = false;
			StatusMessage = FString::Printf(TEXT("Failed to create blueprint - parent class is invalid '%s'"), *FilePath);
			return nullptr;
		}

		// Create package
		UPackage* Package = CreatePackage(*FilePath);
		if (Package == nullptr) {
			bOutSuccess = false;
			StatusMessage = FString::Printf(TEXT("Failed to create blueprint - failed to create package - Filepath is invalid '%s'"), *FilePath);
			return nullptr;
		}

		// Find the blueprint class to create
		UClass* BpClass = nullptr;
		UClass* BpGenClass = nullptr;
		FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler").GetBlueprintTypesForClass(ParentClass, BpClass, BpGenClass);

		// Create blueprint
		UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(ParentClass, Package, *FPaths::GetBaseFilename(FilePath), BPTYPE_Normal, BpClass, BpGenClass);

		// Register Blueprint in the asset registry
		FAssetRegistryModule::AssetCreated(Blueprint);
		Blueprint->MarkPackageDirty();

		// Return success
		bOutSuccess = true;
		StatusMessage = FString::Printf(TEXT("Create blueprint success - created blueprint at '%s'"), *FilePath);
		return Blueprint;

	}
	
	 void CreateThermalController(bool& bSuccess, FString& StatusMessage) {
		//Create thermal controller blueprint
		UBlueprint* ThermalControllerBp = CreateBlueprintClass("/Game/Logi_ThermalCamera/Actors/BP_Logi_ThermalController", AActor::StaticClass(), bSuccess, StatusMessage);

		//Print status
		UE_LOG(LogTemp, Warning, TEXT("%s"), *StatusMessage);

		//Check if blueprint is valid
		if (!ThermalControllerBp) {
			UE_LOG(LogTemp, Error, TEXT("Failed to create Thermal Controller Blueprint - Blueprint is NULL!"));
			bSuccess = false;
			StatusMessage = "Failed to create Thermal Controller Blueprint - Blueprint is NULL!";
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
		BlueprintUtils::AddVariableToBlueprintClass(ThermalControllerBp, "ThermalCameraActive", BoolType, true, "false");
		BlueprintUtils::AddVariableToBlueprintClass(ThermalControllerBp, "ThermalCameraRangeMin", FloatType, true, "0.0");
		BlueprintUtils::AddVariableToBlueprintClass(ThermalControllerBp, "ThermalCameraRangeMax", FloatType, true, "25.0");
		BlueprintUtils::AddVariableToBlueprintClass(ThermalControllerBp, "BackgroundTemperature", FloatType, true, "5.0");
		BlueprintUtils::AddVariableToBlueprintClass(ThermalControllerBp, "SkyTemperature", FloatType, true, "5.0");
		BlueprintUtils::AddVariableToBlueprintClass(ThermalControllerBp, "Blur", FloatType, true, "100");
		BlueprintUtils::AddVariableToBlueprintClass(ThermalControllerBp, "NoiseSize", FloatType, true, "1.0");
		BlueprintUtils::AddVariableToBlueprintClass(ThermalControllerBp, "NoiseAmount", FloatType, true, "5.0");
		BlueprintUtils::AddVariableToBlueprintClass(ThermalControllerBp, "Cold", LinearColorType, true, "R=0.0,G=0.0,B=0.0,A=1.0");
		BlueprintUtils::AddVariableToBlueprintClass(ThermalControllerBp, "Mid", LinearColorType, true, "R=0.5,G=0.5,B=0.5,A=1.0");
		BlueprintUtils::AddVariableToBlueprintClass(ThermalControllerBp, "Hot", LinearColorType, true, "R=1.0,G=1.0,B=1.0,A=1.0");
		BlueprintUtils::AddVariableToBlueprintClass(ThermalControllerBp, "NoiseVector", VectorType, false, "0, 0, 0");


		//Add nodes to the blueprint
		CreateThermalCameraControllerNodeSetup(ThermalControllerBp);
		
		//Validates blueprint
		if (!ThermalControllerBp->GeneratedClass) {
			bSuccess = false;
			StatusMessage = "Thermal Controller Blueprint has an invalid generated class.";
			return;
		}

		//Gets the actors default object
		AActor* ActorDefaultObject = Cast<AActor>(ThermalControllerBp->GeneratedClass->GetDefaultObject());

		//Sets default values to variables
		if (ActorDefaultObject) {

			//Creates anonymus functions to set default values to variables for each variabel type
			const TFunction<void(const FName&, const float)> SetFloatProperty = [&](const FName& PropertyName, const float Value)
			{
				const FFloatProperty* FloatProp = FindFProperty<FFloatProperty>(ActorDefaultObject->GetClass(), PropertyName);

				if (FloatProp) {
					FloatProp->SetPropertyValue_InContainer(ActorDefaultObject, Value);
				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("Property %s not found in blueprint class"), *PropertyName.ToString());
				}
				};

			const TFunction<void(const FName&, const bool)> SetBoolProperty = [&](const FName& PropertyName, const bool Value)
			{
				const FBoolProperty* BoolProp = FindFProperty<FBoolProperty>(ActorDefaultObject->GetClass(), PropertyName);

				if (BoolProp) {
					BoolProp->SetPropertyValue_InContainer(ActorDefaultObject, Value);
				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("Property %s not found in blueprint class"), *PropertyName.ToString());
				}
			};

			const TFunction<void(const FName&, const FLinearColor&)> SetLinearColorProperty = [&](const FName& PropertyName, const FLinearColor& Value)
			{
				const FStructProperty* StructProp = FindFProperty<FStructProperty>(ActorDefaultObject->GetClass(), PropertyName);

				if (StructProp && StructProp->Struct == TBaseStructure<FLinearColor>::Get()) {
					FLinearColor* ColorPtr = StructProp->ContainerPtrToValuePtr<FLinearColor>(ActorDefaultObject);
					*ColorPtr = Value;
				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("Property %s not found in blueprint class"), *PropertyName.ToString());
				}
				};

			const TFunction<void(const FName&, const FVector&)> SetVectorProperty = [&](const FName& PropertyName, const FVector& Value)
			{
				const FStructProperty* StructProp = FindFProperty<FStructProperty>(ActorDefaultObject->GetClass(), PropertyName);

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
		FKismetEditorUtilities::CompileBlueprint(ThermalControllerBp);
		ThermalControllerBp->MarkPackageDirty();

		bool bSaved = LogiUtils::SaveAssetToDisk(ThermalControllerBp);

		if (bSaved)
		{
			
			StatusMessage = FString::Printf(TEXT("Blueprint %s created and successfully saved to: %s"), *ThermalControllerBp->GetName(), *ThermalControllerBp->GetPathName());
			bSaved = true;
		}
		else
		{
			StatusMessage = FString::Printf(TEXT("Blueprint %s created, but failed to save properly. Manual save will be required: %s"), *ThermalControllerBp->GetName(), *ThermalControllerBp->GetPathName());
			bSaved = true;
		}
	}
}
