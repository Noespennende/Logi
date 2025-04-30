#pragma once
#include <optional>

#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionAppendVector.h"
#include "Materials/MaterialExpressionClamp.h"
#include "Materials/MaterialExpressionCollectionParameter.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant2Vector.h"
#include "Materials/MaterialExpressionDivide.h"
#include "Materials/MaterialExpressionFloor.h"
#include "Materials/MaterialExpressionFresnel.h"
#include "Materials/MaterialExpressionIf.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionMax.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionOneMinus.h"
#include "Materials/MaterialExpressionPower.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionStep.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionTime.h"
#include "Materials/MaterialExpressionVectorNoise.h"

enum class EThermalSettingsParamType
{
	Scalar,
	Vector
};


namespace Logi::MaterialUtils
{
	UMaterialExpressionLinearInterpolate* CreateLerpNode(UMaterial* Material, FVector2D Vector2);
	UMaterialExpressionCollectionParameter* CreateThermalSettingsCPNode(UMaterial* Material, FVector2D EditorPos, const FName& ParameterName, EThermalSettingsParamType ParamType);


	UMaterialExpressionMaterialFunctionCall* CreateSceneTexturePostProcess(UMaterial* Material, FVector2D Vector2);
	UMaterialExpressionComment* CreateCommentNode(UMaterial* Material, FVector2D EditorPos, FVector2D Size, const FString& CommentText, const FLinearColor BoxColor = FLinearColor::White);
	UMaterialExpressionAdd* CreateAddNode(UMaterial* Material, FVector2D Vector2);
	UMaterialExpressionComponentMask* CreateMaskNode(UMaterial* Material, FVector2D Vector2, bool bCond, bool bCond1, bool bCond2);
	UMaterialExpressionVectorNoise* CreateVectorNoiseNode(UMaterial* Material, FVector2D Vector2);
	UMaterialExpressionAppendVector* CreateAppendVectorNode(UMaterial* Material, FVector2D Vector2);
	UMaterialExpressionMultiply* CreateMultiplyNode(UMaterial* Material, FVector2D EditorPos, std::optional<float> BValue = std::nullopt, std::optional<float> AValue = std::nullopt);
	UMaterialExpressionTime* CreateTimeNode(UMaterial* Material, FVector2D Vector2);
	UMaterialExpressionConstant* CreateConstantNode(UMaterial* Material, FVector2D Vector2, float X);
	UMaterialExpressionFloor* CreateFloorNode(UMaterial* Material, FVector2D Vector2);
	UMaterialExpressionTextureCoordinate* CreateTextureCoordinateNode(UMaterial* Material, FVector2D Vector2);
	UMaterialExpressionMaterialFunctionCall* CreateViewSize(UMaterial* Material, FVector2D Vector2);
	UMaterialExpressionMaterialFunctionCall* Create3ColorBlendNode(UMaterial* Material, FVector2D Vector2);
	UMaterialExpressionOneMinus* CreateOneMinusNode(UMaterial* Material, FVector2D Vector2);
	UMaterialExpressionFresnel* CreateFresnelNode(UMaterial* Material, FVector2D Vector2);
	UMaterialExpressionScalarParameter* CreateScalarParameterNode(UMaterial* Material, FVector2D Vector2, const FName& ParameterName, float DefaultValue);
	UMaterialExpressionStep* CreateStepNode(UMaterial* Material, FVector2D Vector2);
	UMaterialExpressionMax* CreateMaxNode(UMaterial* Material, FVector2D Vector2);
	UMaterialExpressionMaterialFunctionCall* CreateSceneTextureBaseColor(UMaterial* Material, FVector2D Vector2);
	UMaterialExpressionMaterialFunctionCall* CreateSceneTextureWorldNormal(UMaterial* Material, FVector2D Vector2);
	UMaterialExpressionClamp* CreateClampNode(UMaterial* Material, FVector2D EditorPos, std::optional<float> MinValue = std::nullopt, std::optional<float> MaxValue = std::nullopt);
	UMaterialExpressionConstant2Vector* CreateConstant2VectorNode(UMaterial* Material, FVector2D Vector2, float X, float X1);
	UMaterialExpressionDivide* CreateDivideNode(UMaterial* Material, FVector2D Vector2);
	UMaterialExpressionMaterialFunctionCall* CreateScreenResolution(UMaterial* Material, FVector2D Vector2);
	UMaterialExpressionPower* CreatePowerNode(UMaterial* Material, FVector2D Vector2);
	UMaterialExpressionIf* CreateIfNode(UMaterial* Material, FVector2D Vector2);
	UMaterialExpressionMaterialFunctionCall* CreateSceneTextureSceneDepth(UMaterial* Material, FVector2D Vector2);
	UMaterialExpressionMaterialFunctionCall* CreateSceneTextureCustomDepth(UMaterial* Material, FVector2D Vector2);
};
