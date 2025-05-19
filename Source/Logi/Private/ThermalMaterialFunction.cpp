#include "ThermalMaterialFunction.h"
#include "Materials/MaterialFunction.h"
#include "Materials/MaterialExpressionMakeMaterialAttributes.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionFunctionOutput.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/MaterialFunctionFactoryNew.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionFresnel.h"
#include "Materials/MaterialExpressionPixelNormalWS.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Utils/LogiUtils.h"
#include "Utils/MaterialUtils.h"


namespace Logi::ThermalMaterialFunction
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
        UMaterialExpressionMakeMaterialAttributes* NodeMaterialAttributes = MaterialUtils::CreateMaterialAttributesNode(MaterialFunction, NodeMaterialAttributesPos);
        Expressions.Add(NodeMaterialAttributes);
        
        // Constant3Vector-node, the color to be linked to Specular
        const FVector2D NodeSpecularColorPos(-400, 0);
        const FLinearColor SpecularColor(0.0f, 0.0f, 0.0f);
        UMaterialExpressionConstant3Vector* NodeSpecularColor = MaterialUtils::CreateConstant3VectorNode(MaterialFunction, NodeSpecularColorPos, SpecularColor);
        Expressions.Add(NodeSpecularColor);

        // 3ColorBlend-node
        const FVector2D EmissiveColor3ColorBlendPos(-400, 300);
        UMaterialExpressionMaterialFunctionCall* EmissiveColor3ColorBlendNode = MaterialUtils::Create3ColorBlendNode(MaterialFunction, EmissiveColor3ColorBlendPos);
        Expressions.Add(EmissiveColor3ColorBlendNode);

        EmissiveColor3ColorBlendNode->UpdateFromFunctionResource();

        // Create BaseTemperature-node (3ColorBlend A input)
        const FVector2D NodeBaseTemperaturePos(-850, 0);
        UMaterialExpressionScalarParameter* NodeBaseTemperature = MaterialUtils::CreateScalarParameterNode(MaterialFunction, NodeBaseTemperaturePos, "BaseTemperature", 0.0f);
        Expressions.Add(NodeBaseTemperature);

        
        // Create CurrentTemperature-node (3ColorBlend B input)
        const FVector2D NodeCurrentTemperaturePos(-850, 250);
        UMaterialExpressionScalarParameter* NodeCurrentTemperature = MaterialUtils::CreateScalarParameterNode(MaterialFunction, NodeCurrentTemperaturePos, "CurrentTemperature", 0.5f);
        Expressions.Add(NodeCurrentTemperature);

        // Create MaxTemperature-node (3ColorBlend C input)
        const FVector2D NodeMaxTemperaturePos(-850, 500);
        UMaterialExpressionScalarParameter* NodeMaxTemperature = MaterialUtils::CreateScalarParameterNode(MaterialFunction, NodeMaxTemperaturePos, "MaxTemperature", 1.0f);
        Expressions.Add(NodeMaxTemperature);

        // 3ColorBlend-node
        const FVector2D AlphaColor3ColorBlendPos(-850, 750);
        UMaterialExpressionMaterialFunctionCall* AlphaColor3ColorBlendNode = MaterialUtils::Create3ColorBlendNode(MaterialFunction, AlphaColor3ColorBlendPos);
        Expressions.Add(AlphaColor3ColorBlendNode);

        AlphaColor3ColorBlendNode->UpdateFromFunctionResource();

        // Connecting nodes to the 3ColorBlend-node inputs
        for (FFunctionExpressionInput& Input : EmissiveColor3ColorBlendNode->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("A"))
            {
                Input.Input.Connect(0, NodeBaseTemperature);
            }
            if (Input.Input.InputName == TEXT("B"))
            {
                Input.Input.Connect(0, NodeCurrentTemperature);
            }
            if (Input.Input.InputName == TEXT("C"))
            {
                Input.Input.Connect(0, NodeMaxTemperature);
            }
            if (Input.Input.InputName == TEXT("Alpha"))
            {
                Input.Input.Connect(0, AlphaColor3ColorBlendNode);
            }
        }

        // Constant3Vector-node (Alpa3ColorBlend A input)
        const FVector2D NodeAlpha3ColorBlendConstantAPos(-1350, 600);
        const FLinearColor Alpha3ColorBlendConstantA(1.0f, 1.0f, 1.0f);
        UMaterialExpressionConstant3Vector* NodeAlpha3ColorBlendConstantA = MaterialUtils::CreateConstant3VectorNode(MaterialFunction, NodeAlpha3ColorBlendConstantAPos,Alpha3ColorBlendConstantA);
        Expressions.Add(NodeAlpha3ColorBlendConstantA);

        // Constant3Vector-node (Alpa3ColorBlend B input)
        const FVector2D NodeAlpha3ColorBlendConstantBPos(-1465, 870);
        const FLinearColor Alpha3ColorBlendConstantB(0.067708f, 0.067708f, 0.067708f);
        UMaterialExpressionConstant3Vector* NodeAlpha3ColorBlendConstantB = MaterialUtils::CreateConstant3VectorNode(MaterialFunction, NodeAlpha3ColorBlendConstantBPos, Alpha3ColorBlendConstantB);
        Expressions.Add(NodeAlpha3ColorBlendConstantB);

        // Constant3Vector-node (Alpa3ColorBlend C input)
        const FVector2D NodeAlpha3ColorBlendConstantCPos(-1350, 1140);
        const FLinearColor Alpha3ColorBlendConstantC(0.0f, 0.0f, 0.0f);
        UMaterialExpressionConstant3Vector* NodeAlpha3ColorBlendConstantC = MaterialUtils::CreateConstant3VectorNode(MaterialFunction, NodeAlpha3ColorBlendConstantCPos, Alpha3ColorBlendConstantC);
        Expressions.Add(NodeAlpha3ColorBlendConstantC);

        // CheapContrast_RGB Node  (Alpha3ColorBlend Alpha input)
        const FVector2D NodeCheapContrastRGBPos(-1310, 1390);
        UMaterialExpressionMaterialFunctionCall* NodeCheapContrastRGB = MaterialUtils::CreatCheapContrastRGBNode(MaterialFunction, NodeCheapContrastRGBPos);
        Expressions.Add(NodeCheapContrastRGB);

        NodeCheapContrastRGB->UpdateFromFunctionResource();

        // Connecting nodes to the Alpha3ColorBlend-node inputs
        for (FFunctionExpressionInput& Input : AlphaColor3ColorBlendNode->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("A"))
            {
                Input.Input.Connect(0, NodeAlpha3ColorBlendConstantA);
            }
            if (Input.Input.InputName == TEXT("B"))
            {
                Input.Input.Connect(0, NodeAlpha3ColorBlendConstantB);
            }
            if (Input.Input.InputName == TEXT("C"))
            {
                Input.Input.Connect(0, NodeAlpha3ColorBlendConstantC);
            }
            if (Input.Input.InputName == TEXT("Alpha"))
            {
                Input.Input.Connect(0, NodeCheapContrastRGB);
            }
        }
        
        
        // Create Fresnel-node (Lerp Alpha)
        const FVector2D NodeFresnelPos(-1590, 1390);
        UMaterialExpressionFresnel* NodeFresnel = MaterialUtils::CreateFresnelNode(MaterialFunction, NodeFresnelPos, 0.005f);
        Expressions.Add(NodeFresnel);


        // Constant-node
        const FVector2D ContrastConstantNode(-1540, 1605);
        UMaterialExpressionConstant* NodeContrastConstant = MaterialUtils::CreateConstantNode(MaterialFunction, ContrastConstantNode, 0.1f);
        Expressions.Add(NodeContrastConstant);

        for (FFunctionExpressionInput& Input : NodeCheapContrastRGB->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("In"))
            {
                Input.Input.Connect(0, NodeFresnel);
            }
            if (Input.Input.InputName == TEXT("Contrast"))
            {
                Input.Input.Connect(0, NodeContrastConstant);
            }
        }

        // Create ExponentIn-node (Fresnel ExponentIn input)
        const FVector2D NodeExponentInParamPos(-1890, 1390);
        const FName NodeExponentInParamName("ExponentIn");
        const float NodeExponentInParamDefaultValue = 0.5f;
        
        UMaterialExpressionScalarParameter* NodeExponentInParam = MaterialUtils::CreateScalarParameterNode(
            MaterialFunction, NodeExponentInParamPos, NodeExponentInParamName, NodeExponentInParamDefaultValue);
        Expressions.Add(NodeExponentInParam);
        
        NodeFresnel->ExponentIn.Connect(0, NodeExponentInParam);

        // ComponentMask-node
        const FVector2D NodeMaskPos(-1890, 1605);
        UMaterialExpressionComponentMask* NodeComponentMask = MaterialUtils::CreateMaskNode(MaterialFunction, NodeMaskPos, true, true, true);
        Expressions.Add(NodeComponentMask);

        NodeFresnel->Normal.Connect(0, NodeComponentMask); // Link Mask to Normal on Fresnel

        
        // Create PixelNormalWS
        const FVector2D NodePixelNormalWSPos(-2090, 1605);
        UMaterialExpressionPixelNormalWS* NodePixelNormalWS = MaterialUtils::CreatePixelNormalWSNode(MaterialFunction, NodePixelNormalWSPos);
        Expressions.Add(NodePixelNormalWS);

        NodeComponentMask->Input.Connect(0, NodePixelNormalWS);
        
        // Create Output-node
        const FVector2D NodeOutputResultPos(400, 0);
        UMaterialExpressionFunctionOutput* NodeOutputResult = MaterialUtils::CreateOutputResultNode(MaterialFunction, NodeOutputResultPos, "Result");
        Expressions.Add(NodeOutputResult);


        /* LINKING */

        // Link SpecularColor to Specular
        NodeMaterialAttributes->Specular.Connect(0, NodeSpecularColor);

        // Fresnel connections
        
        

        // ComponentMask connections
       

        // OutputResult connection
        NodeOutputResult->A.Connect(0, NodeMaterialAttributes); // A == the input on the "Output" node

        // Link Lerp to EmissiveColor on MaterialAttributes-node
        NodeMaterialAttributes->EmissiveColor.Connect(0, EmissiveColor3ColorBlendNode);


        /* Finish */

        // === After we are done modifying the file... ===
        // Mark the material as "done being changed/modified"
        MaterialFunction->PostEditChange();
        // Mark as needing saving ("There's been changes in the package/file")
        MaterialFunction->MarkPackageDirty();
            

        // Register the new asset (material) in the AssetRegistry - ensures it appears and is visible in the editor/content browser
        FAssetRegistryModule::AssetCreated(MaterialFunction);

        /**/
        
        bool bSaved = LogiUtils::SaveAssetToDisk(MaterialFunction);

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
