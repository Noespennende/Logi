#pragma once
#include "K2Node_GetArrayItem.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_Select.h"

namespace Logi::BlueprintUtils
{

	void AddVariableToBlueprintClass(UBlueprint* Blueprint, const FName& VarName, const FEdGraphPinType& PinType, bool bInstanceEditable, const FString& DefaultValue);
	
	FName AddMaterialInstanceVariableToBlueprint(UBlueprint* Blueprint);
	
	void AddThermalControllerReferenceToBlueprint(UBlueprint* Blueprint, const FName& VarName, bool bInstanceEditable);

	UEdGraphNode* AddNodeToBlueprint(UBlueprint* Blueprint, const FName& FunctionName, const UClass* Class, const FVector& Location);

	UEdGraphNode* AddNodeToBlueprintFunction(UEdGraph* FunctionGraph, const FName& FunctionName, const UClass* NodeClass, const FVector& Location);

	UK2Node_IfThenElse* CreateBPBranchNode(UEdGraph* EventGraph, int XPosition, int YPosition);

	UK2Node_VariableGet* CreateBPGetterNode(UEdGraph* EventGraph, const FName& VariableName, int XPosition, int YPosition);

	UK2Node_VariableGet* CreateBPExternalGetterNode(UEdGraph* EventGraph, const FName& VariableName, const TCHAR* ExternalClassFilepath, int XPosition, int YPosition);

	UK2Node_GetArrayItem* CreateBPArrayGetterNode(UEdGraph* FunctionGraph, int XPosition, int YPosition);

	UK2Node_VariableSet* CreateBPSetterNode(UEdGraph* FunctionGraph, const FName& VariableName, int XPosition, int YPosition);

	UK2Node_Select* CreateBPSelectNode(UEdGraph* FunctionGraph, int XPosition, int YPosition);

	UK2Node_CallFunction* CreateBPSetMaterialNode(UEdGraph* FunctionGraph, int XPosition, int YPosition);

	UK2Node_CallFunction* CreateBPScalarParameterNode(UEdGraph* EventGraph, int XPosition, int YPosition);

	UK2Node_CallFunction* CreateBPDynamicMaterialInstanceScalarParameterNode(UEdGraph* EventGraph, int XPosition, int YPosition);

	UK2Node_CallFunction* CreateBPVectorParameterNode(UEdGraph* EventGraph, int XPosition, int YPosition);

	UK2Node_CallFunction* CreateBPNormalizeToRangeNode(UEdGraph* EventGraph , int XPosition, int YPosition);

	UK2Node_CallFunction* CreateBPMakeVectorNode(UEdGraph* EventGraph, int XPosition, int YPosition);

	UK2Node_CallFunction* CreateBPSetRenderDepthNode(UEdGraph* FunctionGraph, bool bDefaultValue, int XPosition, int YPosition);

	UK2Node_CallFunction* CreateBPGetAllActorsOfClassNode(UEdGraph* FunctionGraph, int XPosition, int YPosition);

	UK2Node_CallFunction* CreateBPDynamicMaterialInstanceNode(UEdGraph* FunctionGraph, int XPosition, int YPosition);

	UK2Node_CallFunction* CreateBPCallFunctionNode(UEdGraph* EventGraph, const FName& FunctionName, int XPosition, int YPosition);
	
};
