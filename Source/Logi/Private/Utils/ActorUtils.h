#pragma once

#include "BlueprintUtils.h"

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


namespace Logi::ActorUtils
{
	TArray<FName> FindAllMeshComponentsInBlueprint(const UBlueprint* Blueprint);

	TArray<UMaterialInterface*> FindAllMaterialsFromActorScsNode(const USCS_Node* ScsNode);

	TArray<USCS_Node*> FindUscsNodesForMeshComponentsFromBlueprint(const UBlueprint* Blueprint);

	UMeshComponent* FindActorMeshComponentFromName(const UBlueprint* Blueprint, const FName& MeshComponentName);

};
