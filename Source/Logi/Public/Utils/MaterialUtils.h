#pragma once
#include <optional>

#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionAppendVector.h"
#include "Materials/MaterialExpressionClamp.h"
#include "Materials/MaterialExpressionCollectionParameter.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant2Vector.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionDivide.h"
#include "Materials/MaterialExpressionFloor.h"
#include "Materials/MaterialExpressionFresnel.h"
#include "Materials/MaterialExpressionIf.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionMakeMaterialAttributes.h"
#include "Materials/MaterialExpressionMax.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionOneMinus.h"
#include "Materials/MaterialExpressionPixelNormalWS.h"
#include "Materials/MaterialExpressionPower.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionStep.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionTime.h"
#include "Materials/MaterialExpressionVectorNoise.h"
#include "Materials/MaterialExpressionFunctionOutput.h"

namespace Logi::MaterialUtils
{
    // Utility function for checking valid Outer type for node creations
    bool IsOuterAMaterialOrFunction(UObject* Outer);

    // General Utility Nodes
    UMaterialExpressionLinearInterpolate* CreateLerpNode(UObject* Outer, const FVector2D& EditorPos);
    UMaterialExpressionAdd* CreateAddNode(UObject* Outer, const FVector2D& EditorPos);
    UMaterialExpressionMultiply* CreateMultiplyNode(UObject* Outer, const FVector2D& EditorPos, std::optional<float> BValue = std::nullopt, std::optional<float> AValue = std::nullopt);
    UMaterialExpressionOneMinus* CreateOneMinusNode(UObject* Outer, const FVector2D& EditorPos);
    UMaterialExpressionMax* CreateMaxNode(UObject* Outer, const FVector2D& EditorPos);
    UMaterialExpressionStep* CreateStepNode(UObject* Outer, const FVector2D& EditorPos);
    UMaterialExpressionPower* CreatePowerNode(UObject* Outer, const FVector2D& EditorPos);
    UMaterialExpressionDivide* CreateDivideNode(UObject* Outer, const FVector2D& EditorPos);
    UMaterialExpressionIf* CreateIfNode(UObject* Outer, const FVector2D& EditorPos);
    UMaterialExpressionClamp* CreateClampNode(UObject* Outer, const FVector2D& EditorPos, std::optional<float> MinValue = std::nullopt, std::optional<float> MaxValue = std::nullopt);
    UMaterialExpressionFloor* CreateFloorNode(UObject* Outer, const FVector2D& EditorPos);
    UMaterialExpressionConstant* CreateConstantNode(UObject* Outer, const FVector2D& EditorPos, float Value);
    UMaterialExpressionConstant2Vector* CreateConstant2VectorNode(UObject* Outer, const FVector2D& EditorPos, float XValue, float YValue);
    UMaterialExpressionConstant3Vector* CreateConstant3VectorNode(UObject* Outer, const FVector2D& EditorPos, const FLinearColor& Constant);
    UMaterialExpressionPixelNormalWS* CreatePixelNormalWSNode(UObject* Outer, const FVector2D& EditorPos);
    UMaterialExpressionTime* CreateTimeNode(UObject* Outer, const FVector2D& EditorPos);
    UMaterialExpressionTextureCoordinate* CreateTextureCoordinateNode(UObject* Outer, const FVector2D& EditorPos);
    UMaterialExpressionVectorNoise* CreateVectorNoiseNode(UObject* Outer, const FVector2D& EditorPos);
    UMaterialExpressionAppendVector* CreateAppendVectorNode(UObject* Outer, const FVector2D& EditorPos);
    UMaterialExpressionComponentMask* CreateMaskNode(UObject* Outer, const FVector2D& EditorPos, bool R, bool G, bool B);

    // Material Function Nodes
    UMaterialExpressionMaterialFunctionCall* CreateScreenResolutionNode(UObject* Outer, const FVector2D& EditorPos);
    UMaterialExpressionMaterialFunctionCall* CreateViewSizeNode(UObject* Outer, const FVector2D& EditorPos);
    UMaterialExpressionMaterialFunctionCall* Create3ColorBlendNode(UObject* Outer, const FVector2D& EditorPos);
    UMaterialExpressionFunctionOutput* CreateOutputResultNode(UObject* Outer, const FVector2D& EditorPos, const FName& OutputName);

    // Parameter & Collection Nodes
    UMaterialExpressionScalarParameter* CreateScalarParameterNode(UObject* Outer, const FVector2D& EditorPos, const FName& ParameterName, float DefaultValue);
    UMaterialExpressionCollectionParameter* CreateThermalSettingsCPNode(UObject* Outer, const FVector2D& EditorPos, const FName& ParameterName, EThermalSettingsParamType ParamType);

    // Other Nodes
    UMaterialExpressionComment* CreateCommentNode(UObject* Outer, const FVector2D& EditorPos, const FVector2D& Size, const FString& CommentText, const FLinearColor& BoxColor = FLinearColor::White);
    UMaterialExpressionFresnel* CreateFresnelNode(UObject* Outer, const FVector2D& EditorPos, std::optional<float> BaseReflectFractionValue = std::nullopt, std::optional<float> ExponentValue = std::nullopt);
    UMaterialExpressionMakeMaterialAttributes* CreateMaterialAttributesNode(UObject* Outer, const FVector2D& EditorPos);

    // !!! 5.3.2 Workaround node creations - replace with proper node creation for UE version 5.4+ (See documentation)
    UMaterialExpressionMaterialFunctionCall* CreateSceneTexturePostProcessNode(UObject* Outer, const FVector2D& EditorPos);
    UMaterialExpressionMaterialFunctionCall* CreateSceneTextureBaseColorNode(UObject* Outer, const FVector2D& EditorPos);
    UMaterialExpressionMaterialFunctionCall* CreateSceneTextureWorldNormalNode(UObject* Outer, const FVector2D& EditorPos);
    UMaterialExpressionMaterialFunctionCall* CreateSceneTextureSceneDepthNode(UObject* Outer, const FVector2D& EditorPos);
    UMaterialExpressionMaterialFunctionCall* CreateSceneTextureCustomDepthNode(UObject* Outer, const FVector2D& EditorPos);

}
