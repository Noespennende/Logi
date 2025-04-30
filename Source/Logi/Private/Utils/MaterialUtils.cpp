#include "MaterialUtils.h"

#include <optional>

#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionAppendVector.h"
#include "Materials/MaterialExpressionClamp.h"
#include "Materials/MaterialExpressionCollectionParameter.h"
#include "Materials/MaterialExpressionComment.h"
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
#include "Materials/MaterialParameterCollection.h"


namespace Logi::MaterialUtils
{
	UMaterialExpressionCollectionParameter* CreateThermalSettingsCPNode(UMaterial* Material, FVector2D EditorPos, const FName& ParameterName, EThermalSettingsParamType ParamType)
{
    UE_LOG(LogTemp, Warning, TEXT("Starting to create Collection Parameter Node"));

    /* Load MPC_ThermalSettings */
    FString MPCPath = TEXT("/Game/Logi_ThermalCamera/Materials/MPC_Logi_ThermalSettings");
    UMaterialParameterCollection* MPC_ThermalSettings = LoadObject<UMaterialParameterCollection>(nullptr, *MPCPath);
    
    if (!MPC_ThermalSettings)
    {
        // If MPC_ThermalSettings was not loaded correctly, log the error and return nullptr (end)
        UE_LOG(LogTemp, Error, TEXT("Could not load Material Parameter Collection: %s"), *MPCPath);
        return nullptr; 
    }

    /* Create Collection Param node with MPC_ThermalSettings as the collection */
    UMaterialExpressionCollectionParameter* CollectionParamNode = NewObject<UMaterialExpressionCollectionParameter>(Material);
    
    CollectionParamNode->Collection = MPC_ThermalSettings;
    CollectionParamNode->ParameterName = ParameterName;

    // Set the nodes psoition in the editor
    CollectionParamNode->MaterialExpressionEditorX = EditorPos.X;
    CollectionParamNode->MaterialExpressionEditorY = EditorPos.Y;

    
    // Manually find and set GUID for the parameter wanted from MPC_ThermalSettings,
    // based on the known Parameter name
    // (This is needed as the Guid does not automatically transfer with MPC_ThermalSettings when creating the node)

    if (ParamType == EThermalSettingsParamType::Scalar)
    {
        UE_LOG(LogTemp, Warning, TEXT("Wanted Parameter Name: %s"), *CollectionParamNode->ParameterName.ToString());
        for (const FCollectionScalarParameter& ScalarParam : MPC_ThermalSettings->ScalarParameters)
        {
            if (ScalarParam.ParameterName == ParameterName)
            {
                UE_LOG(LogTemp, Warning, TEXT("MATCH: %s"), *ScalarParam.ParameterName.ToString());
                CollectionParamNode->ParameterId = ScalarParam.Id;
                break;
            }
        }
    }

    else if (ParamType == EThermalSettingsParamType::Vector)
    {
        UE_LOG(LogTemp, Warning, TEXT("Wanted Parameter Name: %s"), *CollectionParamNode->ParameterName.ToString());
        for (const FCollectionVectorParameter& VectorParam : MPC_ThermalSettings->VectorParameters)
        {
            // Iterates every parameter in ScalarParameters and checks for the wanted parameter name
            if (VectorParam.ParameterName == ParameterName)
            {
                UE_LOG(LogTemp, Warning, TEXT("MATCH: %s"), *VectorParam.ParameterName.ToString());
                CollectionParamNode->ParameterId = VectorParam.Id;
                break;
            }
            UE_LOG(LogTemp, Warning, TEXT("No match, found: %s"), *VectorParam.ParameterName.ToString());
        }
    }
    
    // Make sure the CollectionParamNode now has a valid collection assigned
    if (!CollectionParamNode->Collection)
    {
        UE_LOG(LogTemp, Error, TEXT("Collection Parameter Node does not have a valid collection assigned"));
        return nullptr;
    }

    return CollectionParamNode;
}


UMaterialExpressionLinearInterpolate* CreateLerpNode(UMaterial* Material, FVector2D EditorPos) {
    
    UMaterialExpressionLinearInterpolate* LerpNode = NewObject<UMaterialExpressionLinearInterpolate>(Material);
    LerpNode->MaterialExpressionEditorX = EditorPos.X;
    LerpNode->MaterialExpressionEditorY = EditorPos.Y;
    
    return LerpNode;
}


UMaterialExpressionMaterialFunctionCall* CreateSceneTexturePostProcess(UMaterial* Material, FVector2D EditorPos)
{

    // 1) Load the MF asset with SceneTexturePP functionality inside
    FString FilePath = TEXT("/Logi/MF_Logi_SceneTexturePostProcess.MF_Logi_SceneTexturePostProcess");
    UMaterialFunction* MaterialFunction = LoadObject<UMaterialFunction>(nullptr, *FilePath);

    if (!MaterialFunction)
    {
        UE_LOG(LogTemp, Warning, TEXT("MaterialFunction failed to load!"));
        return nullptr;
    }
    

    // 2) Create MaterialExpressionFunctionCall node, and assign the MF asset to it
    UMaterialExpressionMaterialFunctionCall* MFSceneTexturePostProcessNode = NewObject<UMaterialExpressionMaterialFunctionCall >(Material);
    MFSceneTexturePostProcessNode->MaterialFunction = MaterialFunction;  // Set the Function to the MF-asset from 1)

    // Nodes posititon in editor
    MFSceneTexturePostProcessNode->MaterialExpressionEditorX = EditorPos.X;
    MFSceneTexturePostProcessNode->MaterialExpressionEditorY = EditorPos.Y;

    return MFSceneTexturePostProcessNode;
}


UMaterialExpressionMaterialFunctionCall* CreateSceneTextureBaseColor(UMaterial* Material, FVector2D EditorPos)
{

    // 1) Load the MF asset with SceneTexturePP functionality inside
    FString FilePath = TEXT("/Logi/MF_Logi_SceneTextureBaseColor.MF_Logi_SceneTextureBaseColor");
    UMaterialFunction* MaterialFunction = LoadObject<UMaterialFunction>(nullptr, *FilePath);

    if (!MaterialFunction)
    {
        UE_LOG(LogTemp, Warning, TEXT("MaterialFunction failed to load!"));
        return nullptr;
    }
    

    // 2) Create MaterialExpressionFunctionCall node, and assign the MF asset to it
    UMaterialExpressionMaterialFunctionCall* MFSceneTextureBaseColorNode = NewObject<UMaterialExpressionMaterialFunctionCall >(Material);
    MFSceneTextureBaseColorNode->MaterialFunction = MaterialFunction;  // Set the Function to the MF-asset from 1)

    // Nodes posititon in editor
    MFSceneTextureBaseColorNode->MaterialExpressionEditorX = EditorPos.X;
    MFSceneTextureBaseColorNode->MaterialExpressionEditorY = EditorPos.Y;

    return MFSceneTextureBaseColorNode;
}


UMaterialExpressionMaterialFunctionCall* CreateSceneTextureWorldNormal(UMaterial* Material, FVector2D EditorPos)
{

    // 1) Load the MF asset with SceneTexturePP functionality inside
    FString FilePath = TEXT("/Logi/MF_Logi_SceneTextureWorldNormal.MF_Logi_SceneTextureWorldNormal");
    UMaterialFunction* MaterialFunction = LoadObject<UMaterialFunction>(nullptr, *FilePath);

    if (!MaterialFunction)
    {
        UE_LOG(LogTemp, Warning, TEXT("MaterialFunction failed to load!"));
        return nullptr;
    }
    

    // 2) Create MaterialExpressionFunctionCall node, and assign the MF asset to it
    UMaterialExpressionMaterialFunctionCall* MFSceneTextureWorldNormalNode = NewObject<UMaterialExpressionMaterialFunctionCall >(Material);
    MFSceneTextureWorldNormalNode->MaterialFunction = MaterialFunction;  // Set the Function to the MF-asset from 1)

    // Nodes posititon in editor
    MFSceneTextureWorldNormalNode->MaterialExpressionEditorX = EditorPos.X;
    MFSceneTextureWorldNormalNode->MaterialExpressionEditorY = EditorPos.Y;

    return MFSceneTextureWorldNormalNode;
}


UMaterialExpressionMaterialFunctionCall* CreateSceneTextureSceneDepth(UMaterial* Material, FVector2D EditorPos)
{

    // 1) Load the MF asset with SceneTexturePP functionality inside
    FString FilePath = TEXT("/Logi/MF_Logi_SceneTextureSceneDepth.MF_Logi_SceneTextureSceneDepth");
    UMaterialFunction* MaterialFunction = LoadObject<UMaterialFunction>(nullptr, *FilePath);

    if (!MaterialFunction)
    {
        UE_LOG(LogTemp, Warning, TEXT("MaterialFunction failed to load!"));
        return nullptr;
    }
    

    // 2) Create MaterialExpressionFunctionCall node, and assign the MF asset to it
    UMaterialExpressionMaterialFunctionCall* MFSceneTextureSceneDepthNode = NewObject<UMaterialExpressionMaterialFunctionCall >(Material);
    MFSceneTextureSceneDepthNode->MaterialFunction = MaterialFunction;  // Set the Function to the MF-asset from 1)

    // Nodes posititon in editor
    MFSceneTextureSceneDepthNode->MaterialExpressionEditorX = EditorPos.X;
    MFSceneTextureSceneDepthNode->MaterialExpressionEditorY = EditorPos.Y;

    return MFSceneTextureSceneDepthNode;
}


UMaterialExpressionMaterialFunctionCall* CreateSceneTextureCustomDepth(UMaterial* Material, FVector2D EditorPos)
{

    // 1) Load the MF asset with SceneTexturePP functionality inside
    FString FilePath = TEXT("/Logi/MF_Logi_SceneTextureCustomDepth.MF_Logi_SceneTextureCustomDepth");
    UMaterialFunction* MaterialFunction = LoadObject<UMaterialFunction>(nullptr, *FilePath);

    if (!MaterialFunction)
    {
        UE_LOG(LogTemp, Warning, TEXT("MaterialFunction failed to load!"));
        return nullptr;
    }
    

    // 2) Create MaterialExpressionFunctionCall node, and assign the MF asset to it
    UMaterialExpressionMaterialFunctionCall* MFSceneTextureCustomDepthNode = NewObject<UMaterialExpressionMaterialFunctionCall >(Material);
    MFSceneTextureCustomDepthNode->MaterialFunction = MaterialFunction;  // Set the Function to the MF-asset from 1)

    // Nodes posititon in editor
    MFSceneTextureCustomDepthNode->MaterialExpressionEditorX = EditorPos.X;
    MFSceneTextureCustomDepthNode->MaterialExpressionEditorY = EditorPos.Y;

    return MFSceneTextureCustomDepthNode;
}


UMaterialExpressionComment* CreateCommentNode(UMaterial* Material, FVector2D EditorPos, FVector2D Size, const FString& CommentText, const FLinearColor BoxColor)
{
    UMaterialExpressionComment* Comment = NewObject<UMaterialExpressionComment>(Material);

    // Posititon in editor
    Comment->MaterialExpressionEditorX = EditorPos.X;
    Comment->MaterialExpressionEditorY = EditorPos.Y;
    

    // Size of the comment box
    Comment->SizeX = Size.X;
    Comment->SizeY = Size.Y;

    // Comment text
    Comment->Text = CommentText;

    // Color of the comment box
    Comment->CommentColor = BoxColor; // Grey color
    // Comment color #FFF976FF
    // Comment->CommentColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); // White color

    return Comment;
}

UMaterialExpressionAdd* CreateAddNode(UMaterial* Material, FVector2D EditorPos)
{
    UMaterialExpressionAdd* AddNode = NewObject<UMaterialExpressionAdd>(Material);

    // Posititon in editor
    AddNode->MaterialExpressionEditorX = EditorPos.X;
    AddNode->MaterialExpressionEditorY = EditorPos.Y;
    
    return AddNode;
}

UMaterialExpressionComponentMask* CreateMaskNode(UMaterial* Material, FVector2D EditorPos, bool R, bool G, bool B)
{

    UMaterialExpressionComponentMask* MaskNode = NewObject<UMaterialExpressionComponentMask>(Material);
    MaskNode->MaterialExpressionEditorX = EditorPos.X;
    MaskNode->MaterialExpressionEditorY = EditorPos.Y;

    MaskNode->R = R; // Activate R-channel
    MaskNode->G = G; // Activate G-channel
    MaskNode->B = B; // Activate B-channel

    return MaskNode;
}

UMaterialExpressionVectorNoise* CreateVectorNoiseNode(UMaterial* Material, FVector2D EditorPos)
{

    UMaterialExpressionVectorNoise* VectorNoiseNode = NewObject<UMaterialExpressionVectorNoise>(Material);
    VectorNoiseNode->MaterialExpressionEditorX = EditorPos.X;
    VectorNoiseNode->MaterialExpressionEditorY = EditorPos.Y;

    return VectorNoiseNode;
}

UMaterialExpressionAppendVector* CreateAppendVectorNode(UMaterial* Material, FVector2D EditorPos)
{

    UMaterialExpressionAppendVector* AppendVectorNode = NewObject<UMaterialExpressionAppendVector>(Material);
    AppendVectorNode->MaterialExpressionEditorX = EditorPos.X;
    AppendVectorNode->MaterialExpressionEditorY = EditorPos.Y;

    return AppendVectorNode;
}

// BValue first of the optionals, because it is most likely to be the one that has a set value, if not both does.
UMaterialExpressionMultiply* CreateMultiplyNode(UMaterial* Material, FVector2D EditorPos, std::optional<float> BValue, std::optional<float> AValue)
{ 
    UMaterialExpressionMultiply* MultiplyNode = NewObject<UMaterialExpressionMultiply>(Material);
    MultiplyNode->MaterialExpressionEditorX = EditorPos.X;
    MultiplyNode->MaterialExpressionEditorY = EditorPos.Y;
    
    if (AValue.has_value())
    {
        MultiplyNode->ConstA = AValue.value();
    }
    
    if (BValue.has_value())
    {
        MultiplyNode->ConstB = BValue.value();
    }

    return MultiplyNode;
}

UMaterialExpressionTime* CreateTimeNode(UMaterial* Material, FVector2D EditorPos)
{

    UMaterialExpressionTime* TimeNode = NewObject<UMaterialExpressionTime>(Material);
    TimeNode->MaterialExpressionEditorX = EditorPos.X;
    TimeNode->MaterialExpressionEditorY = EditorPos.Y;

    return TimeNode;
}

UMaterialExpressionConstant* CreateConstantNode(UMaterial* Material, FVector2D EditorPos, float Value)
{
    UMaterialExpressionConstant* ConstantNode = NewObject<UMaterialExpressionConstant>(Material);
    ConstantNode->MaterialExpressionEditorX = EditorPos.X;
    ConstantNode->MaterialExpressionEditorY = EditorPos.Y;
    ConstantNode->R = Value; 

    return ConstantNode;
}

UMaterialExpressionConstant2Vector* CreateConstant2VectorNode(UMaterial* Material, FVector2D EditorPos, float X, float Y)
{
    UMaterialExpressionConstant2Vector* Constant2VectorNode = NewObject<UMaterialExpressionConstant2Vector>(Material);
    Constant2VectorNode->MaterialExpressionEditorX = EditorPos.X;
    Constant2VectorNode->MaterialExpressionEditorY = EditorPos.Y;
    Constant2VectorNode->R = X; 
    Constant2VectorNode->G = Y;

    return Constant2VectorNode;
}

UMaterialExpressionFloor* CreateFloorNode(UMaterial* Material, FVector2D EditorPos)
{
    UMaterialExpressionFloor* FloorNode = NewObject<UMaterialExpressionFloor>(Material);
    FloorNode->MaterialExpressionEditorX = EditorPos.X;
    FloorNode->MaterialExpressionEditorY = EditorPos.Y;

    return FloorNode;
}

UMaterialExpressionTextureCoordinate* CreateTextureCoordinateNode(UMaterial* Material, FVector2D EditorPos)
{
    UMaterialExpressionTextureCoordinate* TextureCoordinateNode = NewObject<UMaterialExpressionTextureCoordinate>(Material);
    TextureCoordinateNode->MaterialExpressionEditorX = EditorPos.X;
    TextureCoordinateNode->MaterialExpressionEditorY = EditorPos.Y;

    return TextureCoordinateNode;
}

UMaterialExpressionMaterialFunctionCall* CreateViewSize(UMaterial* Material, FVector2D EditorPos)
{

    // 1) Load the MF asset with SceneTexturePP functionality inside
    FString FilePath = TEXT("/Logi/MF_Logi_ViewSize.MF_Logi_ViewSize");
    UMaterialFunction* MaterialFunction = LoadObject<UMaterialFunction>(nullptr, *FilePath);

    if (!MaterialFunction)
    {
        UE_LOG(LogTemp, Warning, TEXT("MaterialFunction failed to load!"));
        return nullptr;
    }
    

    // 2) Create MaterialExpressionFunctionCall node, and assign the MF asset to it
    UMaterialExpressionMaterialFunctionCall* MFViewSizeNode = NewObject<UMaterialExpressionMaterialFunctionCall >(Material);
    MFViewSizeNode->MaterialFunction = MaterialFunction;  // Set the Function to the MF-asset from 1)

    // Nodes posititon in editor
    MFViewSizeNode->MaterialExpressionEditorX = EditorPos.X;
    MFViewSizeNode->MaterialExpressionEditorY = EditorPos.Y;

    return MFViewSizeNode;
}

// Create 3ColorBlend node
UMaterialExpressionMaterialFunctionCall* Create3ColorBlendNode(UMaterial* Material, FVector2D EditorPos)
{
    // Opprett en MaterialExpressionMaterialFunctionCall-node
    UMaterialExpressionMaterialFunctionCall* MaterialFunctionNode = NewObject<UMaterialExpressionMaterialFunctionCall>(Material);
    
    // 3ColorBlend MaterialFunction
    FString FilePath = TEXT("MaterialFunction'/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment/3ColorBlend.3ColorBlend'");
    UMaterialFunction* MFThreeColorBlendNode = LoadObject<UMaterialFunction>(nullptr, *FilePath);
    
    
    if (MFThreeColorBlendNode)
    {
        // Assign 3ColorBlend MF to the MaterialFunction
        MaterialFunctionNode->MaterialFunction = MFThreeColorBlendNode;

        MaterialFunctionNode->MaterialExpressionEditorX = EditorPos.X;
        MaterialFunctionNode->MaterialExpressionEditorY = EditorPos.Y;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load 3ColorBlend MaterialFunction!"));
    }

    return MaterialFunctionNode;
}


UMaterialExpressionOneMinus* CreateOneMinusNode(UMaterial* Material, FVector2D EditorPos)
{
    UMaterialExpressionOneMinus* OneMinusNode = NewObject<UMaterialExpressionOneMinus>(Material);
    OneMinusNode->MaterialExpressionEditorX = EditorPos.X;
    OneMinusNode->MaterialExpressionEditorY = EditorPos.Y;

    return OneMinusNode;
}

// CreateFresnelNode
UMaterialExpressionFresnel* CreateFresnelNode(UMaterial* Material, FVector2D EditorPos)
{
    UMaterialExpressionFresnel* FresnelNode = NewObject<UMaterialExpressionFresnel>(Material);
    FresnelNode->MaterialExpressionEditorX = EditorPos.X;
    FresnelNode->MaterialExpressionEditorY = EditorPos.Y;

    return FresnelNode;
}


UMaterialExpressionScalarParameter* CreateScalarParameterNode(UMaterial* Material, FVector2D EditorPos, const FName& ParameterName, float DefaultValue)
{
    UMaterialExpressionScalarParameter* ScalarParameterNode = NewObject<UMaterialExpressionScalarParameter>(Material);
    ScalarParameterNode->MaterialExpressionEditorX = EditorPos.X;
    ScalarParameterNode->MaterialExpressionEditorY = EditorPos.Y;
    ScalarParameterNode->ParameterName = ParameterName;
    ScalarParameterNode->DefaultValue = DefaultValue;

    return ScalarParameterNode;
}


UMaterialExpressionStep* CreateStepNode(UMaterial* Material, FVector2D EditorPos)
{
    UMaterialExpressionStep* StepNode = NewObject<UMaterialExpressionStep>(Material);
    StepNode->MaterialExpressionEditorX = EditorPos.X;
    StepNode->MaterialExpressionEditorY = EditorPos.Y;

    return StepNode;
}


UMaterialExpressionMax* CreateMaxNode(UMaterial* Material, FVector2D EditorPos)
{
    UMaterialExpressionMax* MaxNode = NewObject<UMaterialExpressionMax>(Material);
    MaxNode->MaterialExpressionEditorX = EditorPos.X;
    MaxNode->MaterialExpressionEditorY = EditorPos.Y;

    return MaxNode;
}


UMaterialExpressionClamp* CreateClampNode(UMaterial* Material, FVector2D EditorPos, std::optional<float> MinValue, std::optional<float> MaxValue)
{
    UMaterialExpressionClamp* ClampNode = NewObject<UMaterialExpressionClamp>(Material);
    ClampNode->MaterialExpressionEditorX = EditorPos.X;
    ClampNode->MaterialExpressionEditorY = EditorPos.Y;

    // Set MinDefault and MaxDefault if values are provided
    if (MinValue.has_value())
    {
        ClampNode->MinDefault = MinValue.value();
        //ClampNode->bUseMin = true;
    }

    if (MaxValue.has_value())
    {
        ClampNode->MaxDefault = MaxValue.value();
        //ClampNode->bUseMax = true;
    }

    return ClampNode;
}


UMaterialExpressionDivide* CreateDivideNode(UMaterial* Material, FVector2D EditorPos)
{
    UMaterialExpressionDivide* DivideNode = NewObject<UMaterialExpressionDivide>(Material);
    DivideNode->MaterialExpressionEditorX = EditorPos.X;
    DivideNode->MaterialExpressionEditorY = EditorPos.Y;

    return DivideNode;
}


UMaterialExpressionMaterialFunctionCall* CreateScreenResolution(UMaterial* Material, FVector2D EditorPos)
{
    // Opprett en MaterialExpressionMaterialFunctionCall-node
    UMaterialExpressionMaterialFunctionCall* MaterialFunctionNode = NewObject<UMaterialExpressionMaterialFunctionCall>(Material);
    
    // ScreenResolution MaterialFunction
    FString FilePath = TEXT("MaterialFunction'/Engine/Functions/Engine_MaterialFunctions02/ScreenResolution.ScreenResolution'");
    UMaterialFunction* MFScreenResolutionNode = LoadObject<UMaterialFunction>(nullptr, *FilePath);
    
    
    if (MFScreenResolutionNode)
    {
        // Assign 3ColorBlend MF to the MaterialFunction
        MaterialFunctionNode->MaterialFunction = MFScreenResolutionNode;

        MaterialFunctionNode->MaterialExpressionEditorX = EditorPos.X;
        MaterialFunctionNode->MaterialExpressionEditorY = EditorPos.Y;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load 3ColorBlend MaterialFunction!"));
    }

    return MaterialFunctionNode;
}


UMaterialExpressionPower* CreatePowerNode(UMaterial* Material, FVector2D EditorPos)
{
    UMaterialExpressionPower* PowerNode = NewObject<UMaterialExpressionPower>(Material);
    PowerNode->MaterialExpressionEditorX = EditorPos.X;
    PowerNode->MaterialExpressionEditorY = EditorPos.Y;

    return PowerNode;
}


// Create If node
UMaterialExpressionIf* CreateIfNode(UMaterial* Material, FVector2D EditorPos)
{
    UMaterialExpressionIf* IfNode = NewObject<UMaterialExpressionIf>(Material);
    IfNode->MaterialExpressionEditorX = EditorPos.X;
    IfNode->MaterialExpressionEditorY = EditorPos.Y;

    return IfNode;
}
}