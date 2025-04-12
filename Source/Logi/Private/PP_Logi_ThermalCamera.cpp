#include "PP_Logi_ThermalCamera.h"
#include "Materials/MaterialFunction.h"
#include "Materials/MaterialExpressionMakeMaterialAttributes.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionFunctionOutput.h"
#include "AssetToolsModule.h"
#include "MaterialDomain.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/MaterialFactoryNew.h"
#include "Materials/MaterialExpressionCollectionParameter.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionFresnel.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionPixelNormalWS.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionComment.h"

#include "SceneTypes.h"
#include "Materials/MaterialExpressionSceneTexture.h"

#include "Materials/MaterialParameterCollection.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionSceneTexture.h"
#include "CoreUObject.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionAppendVector.h"
#include "Materials/MaterialExpressionFloor.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionTime.h"
#include "Materials/MaterialExpressionVectorNoise.h"
#include "Materials/MaterialExpressionViewSize.h"


/// 


UMaterialExpressionCollectionParameter* CreateThermalSettingsCPNode(UMaterial* Material, FVector2D EditorPos, const FName& ParameterName)
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
    for (const FCollectionScalarParameter& Param : MPC_ThermalSettings->ScalarParameters)
    {
        //Log
        UE_LOG(LogTemp, Warning, TEXT("Parameter Name: %s"), *Param.ParameterName.ToString());
        // Iterates every parameter in ScalarParameters and checks for the wanted parameter name
        if (Param.ParameterName == CollectionParamNode->ParameterName)
        {
            //Log
            UE_LOG(LogTemp, Warning, TEXT("Found parameter: %s"), *Param.ParameterName.ToString());
            CollectionParamNode->ParameterId = Param.Id;
            break;
        }

        //Log
        UE_LOG(LogTemp, Warning, TEXT("Did not find: %s"), *Param.ParameterName.ToString());
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


UMaterialExpressionMaterialFunctionCall* CreateSceneTexture(UMaterial* Material, FVector2D EditorPos)
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
    UMaterialExpressionMaterialFunctionCall* MFSceneTextureNode = NewObject<UMaterialExpressionMaterialFunctionCall >(Material);
    MFSceneTextureNode->MaterialFunction = MaterialFunction;  // Set the Function to the MF-asset from 1)

    // Nodes posititon in editor
    MFSceneTextureNode->MaterialExpressionEditorX = EditorPos.X;
    MFSceneTextureNode->MaterialExpressionEditorY = EditorPos.Y;

    return MFSceneTextureNode;
}


UMaterialExpressionComment* CreateCommentNode(UMaterial* Material, FVector2D EditorPos, FVector2D Size, const FString& CommentText, FLinearColor BoxColor = FLinearColor::White)
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

UMaterialExpressionMultiply* CreateMultiplyNode(UMaterial* Material, FVector2D EditorPos)
{

    UMaterialExpressionMultiply* MultiplyNode = NewObject<UMaterialExpressionMultiply>(Material);
    MultiplyNode->MaterialExpressionEditorX = EditorPos.X;
    MultiplyNode->MaterialExpressionEditorY = EditorPos.Y;

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


void FPP_ThermalCamera::CreateThermalCamera(bool& mfNodecreated, FString& statusMessage)
{
    
    FString AssetPath = "/Game/Logi_ThermalCamera/Materials";
    FString AssetName = "PP_Logi_ThermalCamera";

    FString FullAssetPath = AssetPath / AssetName;

    // Get AssetTools-module
    FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");

    /* * Create Material */

    // We need a factory instance, for the creation of the Material
    UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();

    // Actual creation of the Material asset
    // - CreateAsset creates assets, and uses the Factory given as to how the asset is to be created (Material properties)
    // - It returns a UObject, so we cast it to UMaterial
    UMaterial* Material = Cast<UMaterial>(AssetToolsModule.Get().CreateAsset(AssetName, AssetPath, UMaterial::StaticClass(), Factory));
    
    if (!Material)
    {
        // Error log if Material == nullptr
        UE_LOG(LogTemp, Error, TEXT("Could not create Material: %s"), *FullAssetPath);
        return;
    }

    // Set material domain to Post Process
    Material->MaterialDomain = MD_PostProcess;
    

    /* * Creating nodes * */
    
    // The list of nodes in the MaterialFunction - We will be adding the nodes to this list
    TArray<TObjectPtr<UMaterialExpression>>& Expressions = Material->GetExpressionCollection().Expressions;


    /* 1 - White area */

    // LERP-node
    FVector2D WhiteLerpNodePos(-300, 200);
    UMaterialExpressionLinearInterpolate* WhiteLerpNode = CreateLerpNode(Material, WhiteLerpNodePos);
    Expressions.Add(WhiteLerpNode);
    
    // ThermalSettingsCameraToggle-node
    FVector2D ThermalSettingsCameraToggleNodePos(-550, 300);
    UMaterialExpressionCollectionParameter* ThermalSettingsCameraToggleNode = CreateThermalSettingsCPNode(Material, ThermalSettingsCameraToggleNodePos, TEXT("ThermalCameraToggle"));
    Expressions.Add(ThermalSettingsCameraToggleNode);
    
    // MF with SceneTexture-PostProcessInput0 inside -node
    FVector2D MFSceneTextureNodePos(-600, 150);
    UMaterialExpressionMaterialFunctionCall* MFSceneTextureNode = CreateSceneTexture(Material, MFSceneTextureNodePos);
    Expressions.Add(MFSceneTextureNode);
    
    // White comment box (1) - Is thermal camera on?
    FVector2D WhiteCommentPos(-630, 75);
    FVector2D WhiteCommentSize(500, 450);
    FString WhiteCommentText = TEXT("Is thermal camera on?");
    UMaterialExpressionComment* Comment = CreateCommentNode(Material,WhiteCommentPos, WhiteCommentSize, WhiteCommentText);
    Expressions.Add(Comment);

    /* Linking */

    // Connect Lerp node to EmissiveColor
    Material->GetEditorOnlyData()->EmissiveColor.Connect(0, WhiteLerpNode);

    // Connect MFSceneTexture node to B input of Lerp node
    MFSceneTextureNode->UpdateFromFunctionResource();
    WhiteLerpNode->A.Connect(0, MFSceneTextureNode);

    // Connect (CollectionParam) MPC_ThermalSettings node  to A input of Lerp node
    WhiteLerpNode->Alpha.Connect(0, ThermalSettingsCameraToggleNode);


    /* 2 - Yellow area */
    
    // LERP-node
    FVector2D YelloLerpNodePos(-1000, 210);
    UMaterialExpressionLinearInterpolate* YellowLerpNode = CreateLerpNode(Material, YelloLerpNodePos);
    Expressions.Add(YellowLerpNode);

    // (Connects Yellow area to White area)
    WhiteLerpNode->B.Connect(0, YellowLerpNode);


    // ThermalSettingsNoiseAmount-node
    FVector2D ThermalSettingsNoiseAmountPos(-1255, 400);
    UMaterialExpressionCollectionParameter* ThermalSettingsNoiseAmountNode = CreateThermalSettingsCPNode(Material, ThermalSettingsNoiseAmountPos, TEXT("NoiseAmount"));
    Expressions.Add(ThermalSettingsNoiseAmountNode);

    YellowLerpNode->Alpha.Connect(0, ThermalSettingsNoiseAmountNode);

    // Add-node
    FVector2D AddNodePos(-1255, 280);
    UMaterialExpressionAdd* AddNode = CreateAddNode(Material, AddNodePos);
    Expressions.Add(AddNode);

    YellowLerpNode->B.Connect(0, AddNode);


    /** Add noise to image area **/
    
    // Mask-node
    FVector2D NoiseMaskPos(-1550, 450);
    UMaterialExpressionComponentMask* NoiseMaskNode = CreateMaskNode(Material, NoiseMaskPos, true, false, false);
    Expressions.Add(NoiseMaskNode);
    
    AddNode->B.Connect(0, NoiseMaskNode);

    // VectorNoise-node
    FVector2D VectorNoisePos(-1750, 450);
    UMaterialExpressionVectorNoise* VectorNoiseNode = CreateVectorNoiseNode(Material, VectorNoisePos);
    Expressions.Add(VectorNoiseNode);

    NoiseMaskNode->Input.Connect(0, VectorNoiseNode);


    /*** Convert vector 2 to vector 3 ***/
    
    // AppendVector-node
    FVector2D AppendVectorPos(-2000, 460);
    UMaterialExpressionAppendVector* AppendVectorNode = CreateAppendVectorNode(Material, AppendVectorPos);
    Expressions.Add(AppendVectorNode);

    // Connect AppendVector to VectorNoise
    VectorNoiseNode->Position.Connect(0, AppendVectorNode);


    // Multiply-node
    FVector2D ConvertMultiplyNodePos(-2200, 600);
    UMaterialExpressionMultiply* ConvertMultiplyNode = CreateMultiplyNode(Material, ConvertMultiplyNodePos);
    Expressions.Add(ConvertMultiplyNode);


    AppendVectorNode->B.Connect(0, ConvertMultiplyNode);

    // Time-node
    FVector2D TimeNodePos(-2350, 585);
    UMaterialExpressionTime* TimeNode = CreateTimeNode(Material, TimeNodePos);
    Expressions.Add(TimeNode);

    ConvertMultiplyNode->A.Connect(0, TimeNode);


    // Constant-node
    FVector2D ConstantNodePos(-2380, 700);
    UMaterialExpressionConstant* ConstantNode = CreateConstantNode(Material, ConstantNodePos, 60.0f);
    Expressions.Add(ConstantNode);

    ConvertMultiplyNode->B.Connect(0, ConstantNode);

    
    // Yellow inside comment node - Convert vector 2 to vector 3
    FVector2D CommentYellowConvertPos(-2420, 400);
    FVector2D CommentYellowConvertSize(600, 500);
    FString CommentYellowConvertText = TEXT("Convert vector 2 to vector 3");
    UMaterialExpressionComment* CommentYellowConvert = CreateCommentNode(Material, CommentYellowConvertPos, CommentYellowConvertSize, CommentYellowConvertText);
    Expressions.Add(CommentYellowConvert);


    /*** Deside size of noise pixels ***/

    // Multiply-node
    FVector2D DesideMultiplyNodePos(-2600, 460);
    UMaterialExpressionMultiply* DesideMultiplyNode = CreateMultiplyNode(Material, DesideMultiplyNodePos);
    Expressions.Add(DesideMultiplyNode);

    AppendVectorNode->A.Connect(0, DesideMultiplyNode);
    
    // Mask-node
    FVector2D DesideMaskNodePos(-2750, 600);
    UMaterialExpressionComponentMask* DesideMaskNode = CreateMaskNode(Material, DesideMaskNodePos, true, true, false);
    Expressions.Add(DesideMaskNode);

    DesideMultiplyNode->B.Connect(0, DesideMaskNode);

    
    // MPC_ThermalSettings node
    FVector2D ThermalSettingsNoiseSizePos(-3000, 640);
    UMaterialExpressionCollectionParameter* ThermalSettingsNoiseSizeNode = CreateThermalSettingsCPNode(
        Material, ThermalSettingsNoiseSizePos, TEXT("NoiseSize"));
    Expressions.Add(ThermalSettingsNoiseSizeNode);

    DesideMaskNode->Input.Connect(0, ThermalSettingsNoiseSizeNode);
    
    // Yellow inside comment node 2 - Deside size of noise pixels
    FVector2D CommentYellowDesidePos(-3030, 400);
    FVector2D CommentYellowDesideSize(600, 500);
    FString CommentYellowDesideText = TEXT("Deside size of noise pixels");
    UMaterialExpressionComment* CommentYellowDeside = CreateCommentNode(Material, CommentYellowDesidePos, CommentYellowDesideSize, CommentYellowDesideText);
    Expressions.Add(CommentYellowDeside);


    /*** Get screen coordinates ***/

    // Floor-node
    FVector2D CoordinatesFloorNodePos(-3200, 460);
    UMaterialExpressionFloor* CoordinatesFloorNode = CreateFloorNode(Material, CoordinatesFloorNodePos);
    Expressions.Add(CoordinatesFloorNode);
    
    DesideMultiplyNode->A.Connect(0, CoordinatesFloorNode);
    

    // Multiply-node
    FVector2D CoordinatesMultiplyNodePos(-3400, 460);
    UMaterialExpressionMultiply* CoordinatesMultiplyNode = CreateMultiplyNode(Material, CoordinatesMultiplyNodePos);
    Expressions.Add(CoordinatesMultiplyNode);

    CoordinatesFloorNode->Input.Connect(0, CoordinatesMultiplyNode);

    
    // TextureCoordinate-node
    FVector2D CoordinatesTextureCoordinateNodePos(-3620, 460);
    UMaterialExpressionTextureCoordinate* CoordinatesTextureCoordinateNode = CreateTextureCoordinateNode(Material, CoordinatesTextureCoordinateNodePos);
    Expressions.Add(CoordinatesTextureCoordinateNode);

    CoordinatesMultiplyNode->A.Connect(0, CoordinatesTextureCoordinateNode);

    
    // MF with ViewSize inside -node
    FVector2D CoordinatesViewSizeNodePos(-3620, 600);
    UMaterialExpressionMaterialFunctionCall* CoordinatesViewSizeNode = CreateViewSize(Material, CoordinatesViewSizeNodePos);
    Expressions.Add(CoordinatesViewSizeNode);

    CoordinatesViewSizeNode->UpdateFromFunctionResource();
    CoordinatesMultiplyNode->B.Connect(0, CoordinatesViewSizeNode);
    

    // Yellow inside comment node 2 - Deside size of noise pixels
    FVector2D CommentYellowCoordinatesPos(-3650, 400);
    FVector2D CommentYellowCoordinatesSize(600, 500);
    FString CommentYellowCoordinatesText = TEXT("Get screen coordinates");
    UMaterialExpressionComment* CommentYellowCoordinates = CreateCommentNode(Material, CommentYellowCoordinatesPos, CommentYellowCoordinatesSize, CommentYellowCoordinatesText);
    Expressions.Add(CommentYellowCoordinates);


    // 
    // Yellow comment box (2) - Add noise
    FVector2D YellowCommentPos(-3700, 75);
    FVector2D YellowCommentSize(2900, 900);
    FString YellowCommentText = TEXT("Add noise");
    FColor YellowCommentColor =FColor::FromHex(TEXT("FFF976FF"));
    UMaterialExpressionComment* YellowComment = CreateCommentNode(Material, YellowCommentPos, YellowCommentSize, YellowCommentText, YellowCommentColor);
    
    Expressions.Add(YellowComment);
    
    
    /* Finish */

    // Mark dirty ("There's been changes in the package/file!")
    // - Tells Unreal that changes have been made in the project, and need "saving" if ex. project is closed.
    Material->MarkPackageDirty();


    mfNodecreated = true;
    statusMessage = FString::Printf(TEXT("Material Function created and saved to: %s"), *FullAssetPath);

}




