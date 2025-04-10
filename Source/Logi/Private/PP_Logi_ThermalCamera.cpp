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


/// 


UMaterialExpressionCollectionParameter* CreateCollectionParameterNode(UMaterial* Material, FVector2D EditorPos)
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
    CollectionParamNode->ParameterName = TEXT("ThermalCameraToggle");

    // Set the nodes psoition in the editor
    CollectionParamNode->MaterialExpressionEditorX = EditorPos.X;
    CollectionParamNode->MaterialExpressionEditorY = EditorPos.Y;

    
    // Manually find and set GUID for the parameter wanted from MPC_ThermalSettings,
    // based on the known Parameter name
    // (This is needed as the Guid does not automatically transfer with MPC_ThermalSettings when creating the node)
    for (const FCollectionScalarParameter& Param : MPC_ThermalSettings->ScalarParameters)
    {
        // Iterates every parameter in ScalarParameters and checks for the wanted parameter name
        if (Param.ParameterName == CollectionParamNode->ParameterName)
        {
            CollectionParamNode->ParameterId = Param.Id;
            break;
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


UMaterialExpressionComment* CreateCommentNode(UMaterial* Material, FVector2D EditorPos, FVector2D Size, const FString& CommentText)
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

    return Comment;
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
    
    // LERP-node
    FVector2D WhiteLerpNodePos(-300, 200);
    UMaterialExpressionLinearInterpolate* NodeLerp = CreateLerpNode(Material, WhiteLerpNodePos);
    Expressions.Add(NodeLerp);
    
    // CollectionParameter-node
    FVector2D CollectionParamNodePos(-550, 300);
    UMaterialExpressionCollectionParameter* CollectionParamNode = CreateCollectionParameterNode(Material, CollectionParamNodePos);
    Expressions.Add(CollectionParamNode);
    
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


    
    /* LINKING */

    // Connect Lerp node to EmissiveColor
    Material->GetEditorOnlyData()->EmissiveColor.Connect(0, NodeLerp);
    
    
    // Connect MFSceneTexture node to B input of Lerp node
    MFSceneTextureNode->UpdateFromFunctionResource();
    NodeLerp->A.Connect(0, MFSceneTextureNode);

    NodeLerp->Alpha.Connect(0, CollectionParamNode);
    


    /* Finish */

    // Mark dirty ("There's been changes in the package/file!")
    // - Tells Unreal that changes have been made in the project, and need "saving" if ex. project is closed.
    Material->MarkPackageDirty();


    mfNodecreated = true;
    statusMessage = FString::Printf(TEXT("Material Function created and saved to: %s"), *FullAssetPath);

}




