#include "ThermalMaterialFunction.h"
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
#include "Materials/MaterialExpressionPixelNormalWS.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Utils/MaterialUtils.h"


namespace ThermalMaterialFunction
{
    
    void CreateMaterialFunction(bool& bSuccess, FString& StatusMessage)
    {

        const FString AssetPath = "/Game/Logi_ThermalCamera/Materials";
        const FString AssetName = "MF_Logi_ThermalMaterialFunction";

        const FString FullAssetPath = AssetPath / AssetName;

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

        // === Before we start modifying the file... ===
        // Marks material as "about to be changed/modified"
        MaterialFunction->PreEditChange(nullptr);
        MaterialFunction->Modify(); 

         // Lets MaterialFunction to be exposed in Material Library
        MaterialFunction->SetMaterialFunctionUsage(EMaterialFunctionUsage::Default);

        // The list of nodes in the MaterialFunction - We will be adding the nodes to this list
        TArray<TObjectPtr<UMaterialExpression>>& Expressions = MaterialFunction->GetExpressionCollection().Expressions;


        // Creating MaterialAttributes-node
        const FVector2D NodeMaterialAttributesPos(-0, -0);
        UMaterialExpressionMakeMaterialAttributes* NodeMaterialAttributes = Logi::MaterialUtils::CreateMaterialAttributesNode(MaterialFunction, NodeMaterialAttributesPos);
        Expressions.Add(NodeMaterialAttributes);
        
        // Constant3Vector-node, the color to be linked to Specular
        const FVector2D NodeSpecularColorPos(-400, 0);
        const FLinearColor SpecularColor(0.0f, 0.0f, 0.0f);
        UMaterialExpressionConstant3Vector* NodeSpecularColor = Logi::MaterialUtils::CreateConstant3VectorNode(MaterialFunction, NodeSpecularColorPos, SpecularColor);
        Expressions.Add(NodeSpecularColor);

        // LERP-node
        const FVector2D NodeLerpPos(-200, 300);
        UMaterialExpressionLinearInterpolate* NodeLerp = Logi::MaterialUtils::CreateLerpNode(MaterialFunction, NodeLerpPos);
        Expressions.Add(NodeLerp);


        // Create CurrentTemperature-node (Lerp A)
        const FVector2D NodeCurrentTemperaturePos(-496, 250);
        UMaterialExpressionScalarParameter* NodeCurrentTemperature = Logi::MaterialUtils::CreateScalarParameterNode(MaterialFunction, NodeCurrentTemperaturePos, "CurrentTemperature", 1.0f);
        Expressions.Add(NodeCurrentTemperature);
        

        // Create BaseTemperature-node (Lerp B)
        const FVector2D NodeBaseTemperaturePos(-496, 350);
        UMaterialExpressionScalarParameter* NodeBaseTemperature = Logi::MaterialUtils::CreateScalarParameterNode(MaterialFunction, NodeBaseTemperaturePos, "BaseTemperature", 0.0f);
        Expressions.Add(NodeBaseTemperature);
        
        // Create Fresnel-node (Lerp Alpha)
        const FVector2D NodeLerpTempPos(-512, 512);
        UMaterialExpressionFresnel* NodeFresnel = Logi::MaterialUtils::CreateFresnelNode(MaterialFunction, NodeLerpTempPos, 0.04f);
        Expressions.Add(NodeFresnel);


        // Create 1.2-node (Fresnel ExponentIn input)
        const FVector2D NodeExponentInParamPos(-716, 512);
        UMaterialExpressionScalarParameter* NodeExponentInParam = Logi::MaterialUtils::CreateScalarParameterNode(MaterialFunction, NodeExponentInParamPos, "ExponentIn", 1.2f);
        Expressions.Add(NodeExponentInParam);


        // Create ComponentMask-node
        const FVector2D NodeMaskPos(-670, 620);
        UMaterialExpressionComponentMask* NodeComponentMask = Logi::MaterialUtils::CreateMaskNode(MaterialFunction, NodeMaskPos, true, true, true);
        Expressions.Add(NodeComponentMask);

        
        // Create PixelNormalWS
        const FVector2D NodePixelNormalWSPos(-850, 620);
        UMaterialExpressionPixelNormalWS* NodePixelNormalWS = Logi::MaterialUtils::CreatePixelNormalWSNode(MaterialFunction, NodePixelNormalWSPos);
        Expressions.Add(NodePixelNormalWS);

        
        // Create Output-node
        const FVector2D NodeOutputResultPos(400, 0);
        UMaterialExpressionFunctionOutput* NodeOutputResult = Logi::MaterialUtils::CreateOutputResultNode(MaterialFunction, NodeOutputResultPos, "Result");
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

        // === After we are done modifying the file... ===
        // Mark the material as "done being changed/modified"
        MaterialFunction->PostEditChange();
        // Mark as needing saving ("There's been changes in the package/file")
        MaterialFunction->MarkPackageDirty();
            

        // Register the new asset (material) in the AssetRegistry - ensures it appears and is visible in the editor/content browser
        FAssetRegistryModule::AssetCreated(MaterialFunction);

        /**/
        
        bool bSaved = Logi::LogiUtils::SaveAssetToDisk(MaterialFunction);

        if (bSaved)
        {
            StatusMessage = FString::Printf(TEXT("Material %s created and successfully saved to: %s"), *MaterialFunction->GetName(), *FullAssetPath);
            bSuccess = true;
        }
        else
        {
            StatusMessage = FString::Printf(TEXT("Material %s created, but failed to save properly. Manual save required: %s"), *MaterialFunction->GetName(), *FullAssetPath);
            bSuccess = true;
        }

    }

}
