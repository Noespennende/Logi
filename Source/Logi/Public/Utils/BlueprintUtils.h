#pragma once
#include "K2Node_GetArrayItem.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_Select.h"

namespace Logi::BlueprintUtils
{

	void AddVariableToBlueprintClass(UBlueprint* blueprint, FName varName, FEdGraphPinType pinType, bool bInstanceEditable, FString defaultValue);
	
	FName AddMaterialInstanceVariableToBlueprint(UBlueprint* blueprint);
	
	void AddThermalControlerReferenceToBlueprint(UBlueprint* blueprint, FName varName, bool bInstanceEditable);

	UEdGraphNode* AddNodeToBlueprint(UBlueprint* Blueprint, FName FunctionName, UClass* Class, FVector Location);

	UEdGraphNode* AddNodeToBlueprintFunction(UEdGraph* functionGraph, FName functionName, UClass* nodeClass, FVector location);

	UK2Node_IfThenElse* CreateBPBranchNode(UEdGraph* eventGraph, int xPosition, int yPosition);

	UK2Node_VariableGet* CreateBPGetterNode(UEdGraph* eventGraph, FName variableName, int xPosition, int yPosition);

	UK2Node_VariableGet* CreateBPExternalGetterNode(UEdGraph* eventGraph, FName variableName, const TCHAR* externalClassFilepath, int xPosition, int yPosition);

	UK2Node_GetArrayItem* CreateBPArrayGetterNode(UEdGraph* functionGraph, int xPosition, int yPosition);

	UK2Node_VariableSet* CreateBPSetterNode(UEdGraph* functionGraph, FName variableName, int xPosition, int yPosition);

	UK2Node_Select* CreateBPSelectNode(UEdGraph* functionGraph, int xPosition, int yPosition);

	UK2Node_CallFunction* CreateBPSetMaterialNode(UEdGraph* functionGraph, int xPosition, int yPosition);

	UK2Node_CallFunction* CreateBPScalarParameterNode(UEdGraph* eventGraph, int xPosition, int yPosition);

	UK2Node_CallFunction* CreateBPDynamicMaterialInstanceScalarParameterNode(UEdGraph* eventGraph, int xPosition, int yPosition);

	UK2Node_CallFunction* CreateBPVectorParameterNode(UEdGraph* eventGraph, int xPosition, int yPosition);

	UK2Node_CallFunction* CreateBPNormalizeToRangeNode(UEdGraph* eventGraph , int xPosition, int yPosition);

	UK2Node_CallFunction* CreateBPMakeVectorNode(UEdGraph* eventGraph, int xPosition, int yPosition);

	UK2Node_CallFunction* CreateBPSetRenderDepthNode(UEdGraph* functionGraph, bool defaultValue, int xPosition, int yPosition);

	UK2Node_CallFunction* CreateBPGetAllActorsOfClassNode(UEdGraph* functionGraph, int xPosition, int yPosition);

	UK2Node_CallFunction* CreateBPDynamicMaterialInstanceNode(UEdGraph* functionGraph, int xPosition, int yPosition);

	UK2Node_CallFunction* CreateBPCallFunctionNode(UEdGraph* eventGraph, FName functionName, int xPosition, int yPosition);
	
};
