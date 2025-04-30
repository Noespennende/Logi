#include "MF_Logi_ThermalMaterialFunction.h"
#include "Materials/MaterialFunction.h"
#include "Materials/MaterialExpressionMakeMaterialAttributes.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionFunctionOutput.h"
#include "AssetToolsModule.h"
#include "LogiUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/MaterialFunctionFactoryNew.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionFresnel.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionPixelNormalWS.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionComment.h"



UMaterialExpressionMakeMaterialAttributes* CreateMaterialAttributesNode(UMaterialFunction* MaterialFunction) {

    UMaterialExpressionMakeMaterialAttributes* MakeMatAttr = NewObject<UMaterialExpressionMakeMaterialAttributes>(MaterialFunction);

    MakeMatAttr->MaterialExpressionEditorX = 0;   // x = 0
    MakeMatAttr->MaterialExpressionEditorY = 0;   // y = 0


    return MakeMatAttr;
    
}

UMaterialExpressionConstant3Vector* CreateSpecularColorNode(UMaterialFunction* MaterialFunction) {

    //* Constant3Vector-node, the color to be linked to Specular
    UMaterialExpressionConstant3Vector* SpecularColor = NewObject<UMaterialExpressionConstant3Vector>(MaterialFunction);
    SpecularColor->Constant = FLinearColor(0.0f, 0.0f, 0.0f); // Black

    // Visuals - Move the Node on the graph tree
    SpecularColor->MaterialExpressionEditorX = -400; // x = -400, venstre for MakeMatAttr
    SpecularColor->MaterialExpressionEditorY = 0;

    return SpecularColor;

}

UMaterialExpressionLinearInterpolate* CreateLerpNode(UMaterialFunction* MaterialFunction) {
    //* Create Lerp-node
    UMaterialExpressionLinearInterpolate* LerpNode = NewObject<UMaterialExpressionLinearInterpolate>(MaterialFunction);
    LerpNode->MaterialExpressionEditorX = -200;
    LerpNode->MaterialExpressionEditorY = 300;

    return LerpNode;
}

UMaterialExpressionScalarParameter* CreateCurrentTemperatureNode(UMaterialFunction* MaterialFunction) {

    UMaterialExpressionScalarParameter* CurrentTemperature = NewObject<UMaterialExpressionScalarParameter>(MaterialFunction);
    CurrentTemperature->ParameterName = "CurrentTemperature";
    CurrentTemperature->DefaultValue = 1.0f;
    CurrentTemperature->MaterialExpressionEditorX = -496;
    CurrentTemperature->MaterialExpressionEditorY = 250;
   
    return CurrentTemperature;
}

UMaterialExpressionScalarParameter* CreateBaseTemperatureNode(UMaterialFunction* MaterialFunction) {

    UMaterialExpressionScalarParameter* BaseTemperature = NewObject<UMaterialExpressionScalarParameter>(MaterialFunction);
    BaseTemperature->ParameterName = "BaseTemperature";
    BaseTemperature->DefaultValue = 0.0f;
    BaseTemperature->MaterialExpressionEditorX = -496;
    BaseTemperature->MaterialExpressionEditorY = 350;

    return BaseTemperature;

}

UMaterialExpressionFresnel* CreateFresnelNode(UMaterialFunction* MaterialFunction) {

    UMaterialExpressionFresnel* FresnelNode = NewObject<UMaterialExpressionFresnel>(MaterialFunction);
    FresnelNode->BaseReflectFraction = 0.04f; // Set FresnelNodes BaseReflectFractionIn constant
    
    FresnelNode->MaterialExpressionEditorX = -512;
    FresnelNode->MaterialExpressionEditorY = 512;

    return FresnelNode;
}

UMaterialExpressionScalarParameter* CreateExponentInParamNode(UMaterialFunction* MaterialFunction) {
    
    UMaterialExpressionScalarParameter* ExponentIn = NewObject<UMaterialExpressionScalarParameter>(MaterialFunction);
    ExponentIn->ParameterName = "1.2";
    ExponentIn->DefaultValue = 1.2f;
    ExponentIn->MaterialExpressionEditorX = -716;
    ExponentIn->MaterialExpressionEditorY = 512;

    return ExponentIn;
    
}

UMaterialExpressionComponentMask* CreateMaskNode(UMaterialFunction* MaterialFunction) {

    UMaterialExpressionComponentMask* MaskNode = NewObject<UMaterialExpressionComponentMask>(MaterialFunction);
    MaskNode->MaterialExpressionEditorX = -670;
    MaskNode->MaterialExpressionEditorY = 620;

    MaskNode->R = true; // Activate R-channel
    MaskNode->G = true; // Activate G-channel
    MaskNode->B = true; // Activate B-channel

    return MaskNode;
}

UMaterialExpressionPixelNormalWS* CreatePixelNormalWSNode(UMaterialFunction* MaterialFunction) {
    
    UMaterialExpressionPixelNormalWS* PixelNormalWS = NewObject<UMaterialExpressionPixelNormalWS>(MaterialFunction);
    PixelNormalWS->MaterialExpressionEditorX = -850;
    PixelNormalWS->MaterialExpressionEditorY = 620;

    return PixelNormalWS;
}

UMaterialExpressionFunctionOutput* CreateOutputResultNode(UMaterialFunction* MaterialFunction) {
    
    UMaterialExpressionFunctionOutput* OutputResult = NewObject<UMaterialExpressionFunctionOutput>(MaterialFunction);
    OutputResult->OutputName = "Result";
    OutputResult->MaterialExpressionEditorX = 400;  // x = 400, right of MakeMatAttr
    OutputResult->MaterialExpressionEditorY = 0;    // y = 0, same level as MakeMatAttr

    return OutputResult;

}

void FMF_ThermalMaterialFunction::CreateMaterialFunction(bool& mfCreated, FString& statusMessage)
{

    FString AssetPath = "/Game/Logi_ThermalCamera/Materials";
    FString AssetName = "MF_Logi_ThermalMaterialFunction";

    FString FullAssetPath = AssetPath / AssetName;

    // Get AssetTools-module
    FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");

    /* * Create Material Function */
    // We need a factory instance, for the creation of the Material Function
    UMaterialFunctionFactoryNew* Factory = NewObject<UMaterialFunctionFactoryNew>();

    // Actual creation of the MaterialFunction asset
    // - CreateAsset creates assets, and uses the Factory given as to how the asset is to be created (MaterialFunction properties)
    // - It returns a UObject, so we cast it to UMaterialFunction
    UMaterialFunction* MaterialFunction = Cast<UMaterialFunction>(AssetToolsModule.Get().CreateAsset(AssetName, AssetPath, UMaterialFunction::StaticClass(), Factory));

    if (!MaterialFunction)
    {
        // Error log if MaterialFunction == nullptr
        UE_LOG(LogTemp, Error, TEXT("Could not create Material Function: %s"), *FullAssetPath);
        return;
    }

    /* * Creating nodes * */

     // Eksponere i Material Library
    MaterialFunction->SetMaterialFunctionUsage(EMaterialFunctionUsage::Default);

    // The list of nodes in the MaterialFunction - We will be adding the nodes to this list
    TArray<TObjectPtr<UMaterialExpression>>& Expressions = MaterialFunction->GetExpressionCollection().Expressions;


    // Creating MaterialAttributes-node
    UMaterialExpressionMakeMaterialAttributes* NodeMaterialAttributes = CreateMaterialAttributesNode(MaterialFunction);
    Expressions.Add(NodeMaterialAttributes);


    //* Constant3Vector-node, the color to be linked to Specular
    UMaterialExpressionConstant3Vector* NodeSpecularColor = CreateSpecularColorNode(MaterialFunction);
    Expressions.Add(NodeSpecularColor);

    // LERP-node
    UMaterialExpressionLinearInterpolate* NodeLerp = CreateLerpNode(MaterialFunction);
    Expressions.Add(NodeLerp);


    // Create CurrentTemperature-node (Lerp A)
    UMaterialExpressionScalarParameter* NodeCurrentTemperature = CreateCurrentTemperatureNode(MaterialFunction);
    Expressions.Add(NodeCurrentTemperature);


    // Create BaseTemperature-node (Lerp B)
    UMaterialExpressionScalarParameter* NodeBaseTemperature = CreateBaseTemperatureNode(MaterialFunction);
    Expressions.Add(NodeBaseTemperature);


    //-* Create Fresnel-node (Lerp Alpha)
    UMaterialExpressionFresnel* NodeFresnel = CreateFresnelNode(MaterialFunction);
    Expressions.Add(NodeFresnel);


    // Create 1.2-node (Fresnel ExponentIn input)
    UMaterialExpressionScalarParameter* NodeExponentInParam = CreateExponentInParamNode(MaterialFunction);
    Expressions.Add(NodeExponentInParam);


    //--* Create ComponentMask-node
    UMaterialExpressionComponentMask* NodeComponentMask = CreateMaskNode(MaterialFunction);
    Expressions.Add(NodeComponentMask);


    // Create PixelNormalWS
    UMaterialExpressionPixelNormalWS* NodePixelNormalWS = CreatePixelNormalWSNode(MaterialFunction);
    Expressions.Add(NodePixelNormalWS);


    // Create Output-node
    UMaterialExpressionFunctionOutput* NodeOutputResult = CreateOutputResultNode(MaterialFunction);
    Expressions.Add(NodeOutputResult);


    /* LINKING */

    // Link SpecularColor to Specular
    NodeMaterialAttributes->Specular.Connect(0, NodeSpecularColor);

    // Lerp-node connections
    NodeLerp->A.Connect(0, NodeCurrentTemperature);
    NodeLerp->B.Connect(0, NodeBaseTemperature);
    NodeLerp->Alpha.Connect(0, NodeFresnel);

    // Fresnel connections
    NodeFresnel->ExponentIn.Connect(0, NodeExponentInParam);
    NodeFresnel->Normal.Connect(0, NodeComponentMask); // Link Mask to Normal on Fresnel

    // ComponentMask connections
    NodeComponentMask->Input.Connect(0, NodePixelNormalWS);

    // OutputResult connection
    NodeOutputResult->A.Connect(0, NodeMaterialAttributes); // A == the input on the "Output" node

    // Link Lerp to EmissiveColor on MaterialAttributes-node
    NodeMaterialAttributes->EmissiveColor.Connect(0, NodeLerp);


    /* Finish */

    // "Something has changed/happened"
    MaterialFunction->PostEditChange();
    
    // Mark dirty ("There's been changes in the package/file")
    // - Tells Unreal that changes have been made in the project, and need actual "saving" if ex. project is closed.
    MaterialFunction->MarkPackageDirty();

    FAssetRegistryModule::AssetCreated(MaterialFunction);

    /**/
    
    bool bSuccess = Logi::LogiUtils::SaveAssetToDisk(MaterialFunction);

    if (bSuccess)
    {
        statusMessage = FString::Printf(TEXT("Material %s created and successfully saved to: %s"), *MaterialFunction->GetName(), *FullAssetPath);
        mfCreated = true;
    }

    statusMessage = FString::Printf(TEXT("Material %s created, but failed to save properly. Manual save required: %s"), *MaterialFunction->GetName(), *FullAssetPath);
    mfCreated = true;

}
