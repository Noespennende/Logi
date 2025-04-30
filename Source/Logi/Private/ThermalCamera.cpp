#include "ThermalCamera.h"

#include <optional>

#include "AssetToolsModule.h"
#include "MaterialDomain.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/MaterialFactoryNew.h"
#include "SceneTypes.h"

#include "Materials/Material.h"
#include "Materials/MaterialFunction.h"
#include "Materials/MaterialExpressionCollectionParameter.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionFresnel.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionComment.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionAppendVector.h"
#include "Materials/MaterialExpressionClamp.h"
#include "Materials/MaterialExpressionConstant2Vector.h"
#include "Materials/MaterialExpressionDivide.h"
#include "Materials/MaterialExpressionFloor.h"
#include "Materials/MaterialExpressionIf.h"
#include "Materials/MaterialExpressionMax.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionOneMinus.h"
#include "Materials/MaterialExpressionPower.h"
#include "Materials/MaterialExpressionStep.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionTime.h"
#include "Materials/MaterialExpressionVectorNoise.h"

#include "LogiUtils.h"
#include "Utils/MaterialUtils.h"

namespace Logi::ThermalCamera
{
    
    static UMaterialExpressionLinearInterpolate* CreateNodeArea1(UMaterial* Material, TArray<TObjectPtr<UMaterialExpression>>& Expressions)
    {
        /* 1 - White area - Is Thermal Camera on? */

        // LERP-node
        FVector2D Area1WhiteLerpNodePos(-300, 200);
        UMaterialExpressionLinearInterpolate* Area1WhiteLerpNode = MaterialUtils::CreateLerpNode(Material, Area1WhiteLerpNodePos);
        Expressions.Add(Area1WhiteLerpNode);
        
        // ThermalSettingsCameraToggle-node
        FVector2D ThermalSettingsCameraToggleNodePos(-550, 300);
        UMaterialExpressionCollectionParameter* ThermalSettingsCameraToggleNode = MaterialUtils::CreateThermalSettingsCPNode(Material, ThermalSettingsCameraToggleNodePos, TEXT("ThermalCameraToggle"), EThermalSettingsParamType::Scalar);
        Expressions.Add(ThermalSettingsCameraToggleNode);
        
        // MF with SceneTexture-PostProcessInput0 inside -node
        FVector2D MFSceneTextureNodePos(-600, 150);
        UMaterialExpressionMaterialFunctionCall* MFSceneTextureNode = MaterialUtils::CreateSceneTexturePostProcess(Material, MFSceneTextureNodePos);
        Expressions.Add(MFSceneTextureNode);
        
        // White comment box (1) - Is thermal camera on?
        FVector2D WhiteCommentPos(-630, 75);
        FVector2D WhiteCommentSize(500, 450);
        FString WhiteCommentText = TEXT("Is thermal camera on?");
        UMaterialExpressionComment* Comment = MaterialUtils::CreateCommentNode(Material,WhiteCommentPos, WhiteCommentSize, WhiteCommentText);
        Expressions.Add(Comment);

        /* Linking */

        // Connect Lerp node to EmissiveColor
        Material->GetEditorOnlyData()->EmissiveColor.Connect(0, Area1WhiteLerpNode);

        // Connect MFSceneTexture node to B input of Lerp node
        MFSceneTextureNode->UpdateFromFunctionResource();
        Area1WhiteLerpNode->A.Connect(0, MFSceneTextureNode);

        // Connect (CollectionParam) MPC_ThermalSettings node  to A input of Lerp node
        Area1WhiteLerpNode->Alpha.Connect(0, ThermalSettingsCameraToggleNode);

        return Area1WhiteLerpNode;
    }

    struct FNodeArea2Result
    {
        UMaterialExpressionLinearInterpolate* Area2YellowLerpNode = nullptr;
        UMaterialExpressionAdd* Area2YellowAddNode = nullptr;
    };

    static FNodeArea2Result CreateNodeArea2(UMaterial* Material, TArray<TObjectPtr<UMaterialExpression>>& Expressions, UMaterialExpressionLinearInterpolate* Area1WhiteLerpNode)
    {
        /* 2 - Yellow area  - Add noise */

        FNodeArea2Result Result;

        // 
        // Yellow comment box (2) - Add noise
        FVector2D YellowCommentPos(-3700, 75);
        FVector2D YellowCommentSize(2900, 900);
        FString YellowCommentText = TEXT("Add noise");
        FColor YellowCommentColor =FColor::FromHex(TEXT("FFF976FF"));
        UMaterialExpressionComment* YellowComment = MaterialUtils::CreateCommentNode(Material, YellowCommentPos, YellowCommentSize, YellowCommentText, YellowCommentColor);
        
        Expressions.Add(YellowComment);
        
        // LERP-node
        FVector2D YelloLerpNodePos(-1000, 210);
        Result.Area2YellowLerpNode = MaterialUtils::CreateLerpNode(Material, YelloLerpNodePos);
        Expressions.Add(Result.Area2YellowLerpNode);

        // ThermalSettingsNoiseAmount-node
        FVector2D ThermalSettingsNoiseAmountPos(-1255, 400);
        UMaterialExpressionCollectionParameter* ThermalSettingsNoiseAmountNode = MaterialUtils::CreateThermalSettingsCPNode(Material, ThermalSettingsNoiseAmountPos, TEXT("NoiseAmount"), EThermalSettingsParamType::Scalar);
        Expressions.Add(ThermalSettingsNoiseAmountNode);

        Result.Area2YellowLerpNode->Alpha.Connect(0, ThermalSettingsNoiseAmountNode);

        // Add-node
        FVector2D Area2YellowAddNodePos(-1255, 280);
        Result.Area2YellowAddNode = MaterialUtils::CreateAddNode(Material, Area2YellowAddNodePos);
        Expressions.Add(Result.Area2YellowAddNode);

        Result.Area2YellowLerpNode->B.Connect(0, Result.Area2YellowAddNode);


        /** Add noise to image area **/

        //FVector2D CommentYellowCoordinatesPos(-3650, 400);
        //FVector2D CommentYellowCoordinatesSize(600, 500);

        // Yellow inside comment node 1 - Add noise to image
        FVector2D CommentNoiseImageAreaPos(-3680, 350);
        FVector2D CommentNoiseImageAreaSize(2280, 560);
        FString CommentNoiseImageAreaText = TEXT("Add noise to image");
        UMaterialExpressionComment* CommentNoiseImageArea = MaterialUtils::CreateCommentNode(Material, CommentNoiseImageAreaPos, CommentNoiseImageAreaSize, CommentNoiseImageAreaText);
        Expressions.Add(CommentNoiseImageArea);
        
        // Mask-node
        FVector2D NoiseMaskPos(-1550, 450);
        UMaterialExpressionComponentMask* NoiseMaskNode = MaterialUtils::CreateMaskNode(Material, NoiseMaskPos, true, false, false);
        Expressions.Add(NoiseMaskNode);
        
        Result.Area2YellowAddNode->B.Connect(0, NoiseMaskNode);

        // VectorNoise-node
        FVector2D VectorNoisePos(-1750, 450);
        UMaterialExpressionVectorNoise* VectorNoiseNode = MaterialUtils::CreateVectorNoiseNode(Material, VectorNoisePos);
        Expressions.Add(VectorNoiseNode);

        NoiseMaskNode->Input.Connect(0, VectorNoiseNode);


        /*** Convert vector 2 to vector 3 ***/
        
        // AppendVector-node
        FVector2D AppendVectorPos(-2000, 460);
        UMaterialExpressionAppendVector* AppendVectorNode = MaterialUtils::CreateAppendVectorNode(Material, AppendVectorPos);
        Expressions.Add(AppendVectorNode);

        // Connect AppendVector to VectorNoise
        VectorNoiseNode->Position.Connect(0, AppendVectorNode);


        // Multiply-node
        FVector2D ConvertMultiplyNodePos(-2200, 600);
        UMaterialExpressionMultiply* ConvertMultiplyNode = MaterialUtils::CreateMultiplyNode(Material, ConvertMultiplyNodePos);
        Expressions.Add(ConvertMultiplyNode);


        AppendVectorNode->B.Connect(0, ConvertMultiplyNode);

        // Time-node
        FVector2D TimeNodePos(-2350, 585);
        UMaterialExpressionTime* TimeNode = MaterialUtils::CreateTimeNode(Material, TimeNodePos);
        Expressions.Add(TimeNode);

        ConvertMultiplyNode->A.Connect(0, TimeNode);


        // Constant-node
        FVector2D ConstantNodePos(-2380, 700);
        UMaterialExpressionConstant* ConstantNode = MaterialUtils::CreateConstantNode(Material, ConstantNodePos, 60.0f);
        Expressions.Add(ConstantNode);

        ConvertMultiplyNode->B.Connect(0, ConstantNode);

        
        // Yellow inside comment node - Convert vector 2 to vector 3
        FVector2D CommentYellowConvertPos(-2420, 400);
        FVector2D CommentYellowConvertSize(600, 500);
        FString CommentYellowConvertText = TEXT("Convert vector 2 to vector 3");
        UMaterialExpressionComment* CommentYellowConvert = MaterialUtils::CreateCommentNode(Material, CommentYellowConvertPos, CommentYellowConvertSize, CommentYellowConvertText);
        Expressions.Add(CommentYellowConvert);


        /*** Deside size of noise pixels ***/

        // Multiply-node
        FVector2D DesideMultiplyNodePos(-2600, 460);
        UMaterialExpressionMultiply* DesideMultiplyNode = MaterialUtils::CreateMultiplyNode(Material, DesideMultiplyNodePos);
        Expressions.Add(DesideMultiplyNode);

        AppendVectorNode->A.Connect(0, DesideMultiplyNode);
        
        // Mask-node
        FVector2D DesideMaskNodePos(-2750, 600);
        UMaterialExpressionComponentMask* DesideMaskNode = MaterialUtils::CreateMaskNode(Material, DesideMaskNodePos, true, true, false);
        Expressions.Add(DesideMaskNode);

        DesideMultiplyNode->B.Connect(0, DesideMaskNode);

        
        // MPC_ThermalSettings node
        FVector2D ThermalSettingsNoiseSizePos(-3000, 640);
        UMaterialExpressionCollectionParameter* ThermalSettingsNoiseSizeNode = MaterialUtils::CreateThermalSettingsCPNode(
            Material, ThermalSettingsNoiseSizePos, TEXT("NoiseSize"), EThermalSettingsParamType::Vector);
        Expressions.Add(ThermalSettingsNoiseSizeNode);

        DesideMaskNode->Input.Connect(0, ThermalSettingsNoiseSizeNode);
        
        // Yellow inside comment node 2 - Deside size of noise pixels
        FVector2D CommentYellowDesidePos(-3030, 400);
        FVector2D CommentYellowDesideSize(600, 500);
        FString CommentYellowDesideText = TEXT("Deside size of noise pixels");
        UMaterialExpressionComment* CommentYellowDeside = MaterialUtils::CreateCommentNode(Material, CommentYellowDesidePos, CommentYellowDesideSize, CommentYellowDesideText);
        Expressions.Add(CommentYellowDeside);


        /*** Get screen coordinates ***/

        // Yellow inside comment node 2 - Deside size of noise pixels
        FVector2D CommentYellowCoordinatesPos(-3650, 400);
        FVector2D CommentYellowCoordinatesSize(600, 500);
        FString CommentYellowCoordinatesText = TEXT("Get screen coordinates");
        UMaterialExpressionComment* CommentYellowCoordinates = MaterialUtils::CreateCommentNode(Material, CommentYellowCoordinatesPos, CommentYellowCoordinatesSize, CommentYellowCoordinatesText);
        Expressions.Add(CommentYellowCoordinates);

        // Floor-node
        FVector2D CoordinatesFloorNodePos(-3200, 460);
        UMaterialExpressionFloor* CoordinatesFloorNode = MaterialUtils::CreateFloorNode(Material, CoordinatesFloorNodePos);
        Expressions.Add(CoordinatesFloorNode);
        
        DesideMultiplyNode->A.Connect(0, CoordinatesFloorNode);
        

        // Multiply-node
        FVector2D CoordinatesMultiplyNodePos(-3400, 460);
        UMaterialExpressionMultiply* CoordinatesMultiplyNode = MaterialUtils::CreateMultiplyNode(Material, CoordinatesMultiplyNodePos);
        Expressions.Add(CoordinatesMultiplyNode);

        CoordinatesFloorNode->Input.Connect(0, CoordinatesMultiplyNode);

        
        // TextureCoordinate-node
        FVector2D CoordinatesTextureCoordinateNodePos(-3620, 460);
        UMaterialExpressionTextureCoordinate* CoordinatesTextureCoordinateNode = MaterialUtils::CreateTextureCoordinateNode(Material, CoordinatesTextureCoordinateNodePos);
        Expressions.Add(CoordinatesTextureCoordinateNode);

        CoordinatesMultiplyNode->A.Connect(0, CoordinatesTextureCoordinateNode);

        
        // MF with ViewSize inside -node
        FVector2D CoordinatesViewSizeNodePos(-3620, 600);
        UMaterialExpressionMaterialFunctionCall* CoordinatesViewSizeNode = MaterialUtils::CreateViewSize(Material, CoordinatesViewSizeNodePos);
        Expressions.Add(CoordinatesViewSizeNode);

        CoordinatesViewSizeNode->UpdateFromFunctionResource();
        CoordinatesMultiplyNode->B.Connect(0, CoordinatesViewSizeNode);

        return Result;
    }

    static UMaterialExpressionAppendVector* CreateNodeArea3(UMaterial* Material, TArray<TObjectPtr<UMaterialExpression>>& Expressions)
    {
        /* 3 - White area - Add back the alpha channel to the image */

        // Append-node
        FVector2D ImageConstructAppendNodePos(-4050, 210);
        UMaterialExpressionAppendVector* ImageConstructAppendNode = MaterialUtils::CreateAppendVectorNode(Material, ImageConstructAppendNodePos);
        Expressions.Add(ImageConstructAppendNode);

        // Constant-node
        FVector2D ImageConstructConstantNodePos(-4250, 300);
        UMaterialExpressionConstant* ImageConstructConstantNode = MaterialUtils::CreateConstantNode(Material, ImageConstructConstantNodePos, 0.0f);
        Expressions.Add(ImageConstructConstantNode);

        ImageConstructAppendNode->B.Connect(0, ImageConstructConstantNode);

        
        // White comment box (3) - Add back the alpha channel to the image
        FVector2D ImageConstructCommentPos(-4350, 75);
        FVector2D ImageConstructCommentSize(500, 450);
        FString ImageConstructCommentText = TEXT("Add back the alpha channel to the image");
        UMaterialExpressionComment* ImageConstructComment = MaterialUtils::CreateCommentNode(Material, ImageConstructCommentPos, ImageConstructCommentSize, ImageConstructCommentText);
        Expressions.Add(ImageConstructComment);
        

        return ImageConstructAppendNode;
    }

    struct FNodeArea4Result
    {
        UMaterialExpressionMaterialFunctionCall* Area4Background3ColorBlendNode = nullptr;
        UMaterialExpressionMultiply* Area4AddSkyMultiplyNode = nullptr;
    };

    static FNodeArea4Result CreateNodeArea4(UMaterial* Material, TArray<TObjectPtr<UMaterialExpression>>& Expressions)
    {
        /* 4 - Blue area */

        /** Blue 4.1 - Background colors **/

        // Blue comment box (4.1) - Background colors
        FVector2D BackgroundCommentPos(-7800, -2600);
        FVector2D BackgroundCommentSize(2200, 1200);
        FString BackgroundCommentText = TEXT("Background colors");
        FColor BackgroundCommentColor =FColor::FromHex(TEXT("00B6FFFF"));
        UMaterialExpressionComment* BackgroundComment = MaterialUtils::CreateCommentNode(Material, BackgroundCommentPos, BackgroundCommentSize, BackgroundCommentText, BackgroundCommentColor);
        Expressions.Add(BackgroundComment);


        // 3ColorBlend-node
        FVector2D Background3ColorBlendNodePos(-5900, -2150);
        UMaterialExpressionMaterialFunctionCall* Background3ColorBlendNode = MaterialUtils::Create3ColorBlendNode(Material, Background3ColorBlendNodePos);
        Expressions.Add(Background3ColorBlendNode);

        Background3ColorBlendNode->UpdateFromFunctionResource();
        


        // MPC_ThermalSettings "Cold" node
        FVector2D ThermalSettingsColdPos(-6400, -2500);
        UMaterialExpressionCollectionParameter* ThermalSettingsColdNode = MaterialUtils::CreateThermalSettingsCPNode(Material, ThermalSettingsColdPos, TEXT("Cold"), EThermalSettingsParamType::Vector);
        Expressions.Add(ThermalSettingsColdNode);
        
        for (FFunctionExpressionInput& Input : Background3ColorBlendNode->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("A"))
            {
                Input.Input.Connect(0, ThermalSettingsColdNode);
            }
        }

        // MPC_ThermalSettings "Mid" node
        FVector2D ThermalSettingsMidPos(-6400, -2300);
        UMaterialExpressionCollectionParameter* ThermalSettingsMidNode = MaterialUtils::CreateThermalSettingsCPNode(Material, ThermalSettingsMidPos, TEXT("Mid"), EThermalSettingsParamType::Vector);
        Expressions.Add(ThermalSettingsMidNode);
        
        for (FFunctionExpressionInput& Input : Background3ColorBlendNode->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("B"))
            {
                Input.Input.Connect(0, ThermalSettingsMidNode);
            }
            
        }

        // MPC_ThermalSettings "Hot" node
        FVector2D ThermalSettingsHotPos(-6400, -2100);
        UMaterialExpressionCollectionParameter* ThermalSettingsHotNode = MaterialUtils::CreateThermalSettingsCPNode(Material, ThermalSettingsHotPos, TEXT("Hot"), EThermalSettingsParamType::Vector);
        Expressions.Add(ThermalSettingsHotNode);
        
        for (FFunctionExpressionInput& Input : Background3ColorBlendNode->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("C"))
            {
                Input.Input.Connect(0, ThermalSettingsHotNode);
            }
            
        }


        // Multiply-node
        FVector2D BackgroundMultiplyNodePos(-6900, -1900);
        UMaterialExpressionMultiply* BackgroundMultiplyNode = MaterialUtils::CreateMultiplyNode(Material, BackgroundMultiplyNodePos);
        Expressions.Add(BackgroundMultiplyNode);


        // MPC Thermal BackgroundTemperature
        FVector2D ThermalSettingsBackgroundTemperaturePos(-7400, -1930);
        UMaterialExpressionCollectionParameter* ThermalSettingsBackgroundTemperatureNode = MaterialUtils::CreateThermalSettingsCPNode(Material, ThermalSettingsBackgroundTemperaturePos, TEXT("BackgroundTemperature"), EThermalSettingsParamType::Scalar);
        Expressions.Add(ThermalSettingsBackgroundTemperatureNode);
        BackgroundMultiplyNode->A.Connect(0, ThermalSettingsBackgroundTemperatureNode);

        // OneMinus-node
        FVector2D BackgroundOneMinusNodePos(-7050, -1700);
        UMaterialExpressionOneMinus* BackgroundOneMinusNode = MaterialUtils::CreateOneMinusNode(Material, BackgroundOneMinusNodePos);
        Expressions.Add(BackgroundOneMinusNode);
        BackgroundMultiplyNode->B.Connect(0, BackgroundOneMinusNode);


        // Fresnel-node
        FVector2D BackgroundFresnelNodePos(-7400, -1700);
        UMaterialExpressionFresnel* BackgroundFresnelNode = MaterialUtils::CreateFresnelNode(Material, BackgroundFresnelNodePos);
        Expressions.Add(BackgroundFresnelNode);

        BackgroundOneMinusNode->Input.Connect(0, BackgroundFresnelNode);


        // Fersnel EXP ScalarParameter-node
        FVector2D BackgroundFresnelExpPos(-7700, -1715);
        UMaterialExpressionScalarParameter* BackgroundFresnelExpNode = MaterialUtils::CreateScalarParameterNode(Material, BackgroundFresnelExpPos, TEXT("Fersnel EXP"), 1.0f);
        Expressions.Add(BackgroundFresnelExpNode);
        BackgroundFresnelNode->ExponentIn.Connect(0, BackgroundFresnelExpNode);


        // Mask-node
        FVector2D BackgroundMaskNodePos(-7700, -1620);
        UMaterialExpressionComponentMask* BackgroundMaskNode = MaterialUtils::CreateMaskNode(Material, BackgroundMaskNodePos, true, true, true);
        Expressions.Add(BackgroundMaskNode);
        BackgroundFresnelNode->Normal.Connect(0, BackgroundMaskNode);

        

        /** Blue 4.2 - Re-add sky in to image background image **/

        // Blue comment box (4,2) - Re-add sky in to image background image
        FVector2D AddSkyCommentPos(-7800, -1300);
        FVector2D AddSkyCommentSize(1500, 900);
        FString AddSkyCommentText = TEXT("Re-add sky in to image background image");
        FColor AddSkyCommentColor =FColor::FromHex(TEXT("00B6FFFF"));
        UMaterialExpressionComment* AddSkyComment = MaterialUtils::CreateCommentNode(Material, AddSkyCommentPos, AddSkyCommentSize, AddSkyCommentText, AddSkyCommentColor);
        Expressions.Add(AddSkyComment);


        // Lerp-node
        FVector2D AddSkyLerpNodePos(-6500, -1000);
        UMaterialExpressionLinearInterpolate* AddSkyLerpNode = MaterialUtils::CreateLerpNode(Material, AddSkyLerpNodePos);
        Expressions.Add(AddSkyLerpNode);

        
        for (FFunctionExpressionInput& Input : Background3ColorBlendNode->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("Alpha"))
            {
                Input.Input.Connect(0, AddSkyLerpNode);
            }
            
        }

        AddSkyLerpNode->B.Connect(0, BackgroundMultiplyNode);

        
        // Multiply-node
        FVector2D AddSkyMultiplyNodePos(-7000, -1050);
        UMaterialExpressionMultiply* AddSkyMultiplyNode = MaterialUtils::CreateMultiplyNode(Material, AddSkyMultiplyNodePos);
        Expressions.Add(AddSkyMultiplyNode);
        AddSkyLerpNode->A.Connect(0, AddSkyMultiplyNode);


        // MPC_ThermalSettings "SkyTemperatur" node
        FVector2D ThermalSettingsSkyTemperaturePos(-7400, -1200);
        UMaterialExpressionCollectionParameter* ThermalSettingsSkyTemperatureNode = MaterialUtils::CreateThermalSettingsCPNode(Material, ThermalSettingsSkyTemperaturePos, TEXT("SkyTemperature"), EThermalSettingsParamType::Scalar);
        Expressions.Add(ThermalSettingsSkyTemperatureNode);

        AddSkyMultiplyNode->A.Connect(0, ThermalSettingsSkyTemperatureNode);


        /*** Blue 4.2 White inside - Is R, G and B channels black? If so re-add sky ***/
        
        // Blue 4.2 inside comment box - Is R, G and B channels black? If so re-add sky
        FVector2D ReAddSkyCommentPos(-7200, -900);
        FVector2D ReAddSkyCommentSize(600, 350);
        FString ReAddSkyCommentText = TEXT("Is R, G and B channels black? If so re-add sky");
        UMaterialExpressionComment* ReAddSkyComment = MaterialUtils::CreateCommentNode(Material, ReAddSkyCommentPos, ReAddSkyCommentSize, ReAddSkyCommentText);
        Expressions.Add(ReAddSkyComment);


        // Step-node
        FVector2D ReAddSkyStepNodePos(-6750, -850);
        UMaterialExpressionStep* ReAddSkyStepNode = MaterialUtils::CreateStepNode(Material, ReAddSkyStepNodePos);
        ReAddSkyStepNode->ConstY = 0.0001f;
        Expressions.Add(ReAddSkyStepNode);
        
        AddSkyLerpNode->Alpha.Connect(0, ReAddSkyStepNode);


        // Max-node
        FVector2D ReAddSkyMaxNodePos(-6900, -860);
        UMaterialExpressionMax* ReAddSkyMaxNode = MaterialUtils::CreateMaxNode(Material, ReAddSkyMaxNodePos);
        Expressions.Add(ReAddSkyMaxNode);

        ReAddSkyStepNode->X.Connect(0, ReAddSkyMaxNode);

        
        // Max-node (connecting Mask G and B)
        FVector2D ReAddSkyMaxGBNodePos(-7010, -700);
        UMaterialExpressionMax* ReAddSkyMaxGBNode = MaterialUtils::CreateMaxNode(Material, ReAddSkyMaxGBNodePos);
        Expressions.Add(ReAddSkyMaxGBNode);

        ReAddSkyMaxNode->B.Connect(0, ReAddSkyMaxGBNode);
        

        // Mask-node
        FVector2D ReAddSkyMaskRNodePos(-7150, -850);
        UMaterialExpressionComponentMask* ReAddSkyMaskRNode = MaterialUtils::CreateMaskNode(Material, ReAddSkyMaskRNodePos, true, false, false);
        Expressions.Add(ReAddSkyMaskRNode);
        ReAddSkyMaxNode->A.Connect(0, ReAddSkyMaskRNode);


        // Mask-node
        FVector2D ReAddSkyMaskGNodePos(-7150, -750);
        UMaterialExpressionComponentMask* ReAddSkyMaskGNode = MaterialUtils::CreateMaskNode(Material, ReAddSkyMaskGNodePos, false, true, false);
        Expressions.Add(ReAddSkyMaskGNode);

        ReAddSkyMaxGBNode->A.Connect(0, ReAddSkyMaskGNode);


        // Mask-node
        FVector2D ReAddSkyMaskBNodePos(-7150, -650);
        UMaterialExpressionComponentMask* ReAddSkyMaskBNode = MaterialUtils::CreateMaskNode(Material, ReAddSkyMaskBNodePos, false, false, true);
        Expressions.Add(ReAddSkyMaskBNode);

        ReAddSkyMaxGBNode->B.Connect(0, ReAddSkyMaskBNode);


        // SceneTexture:BaseColor-node
        FVector2D SceneTextureBaseColorNodePos(-7500, -750);
        UMaterialExpressionMaterialFunctionCall* SceneTextureBaseColorNode = MaterialUtils::CreateSceneTextureBaseColor(Material, SceneTextureBaseColorNodePos);
        Expressions.Add(SceneTextureBaseColorNode);
        SceneTextureBaseColorNode->UpdateFromFunctionResource();

        ReAddSkyMaskRNode->Input.Connect(0, SceneTextureBaseColorNode);
        ReAddSkyMaskGNode->Input.Connect(0, SceneTextureBaseColorNode);
        ReAddSkyMaskBNode->Input.Connect(0, SceneTextureBaseColorNode);


        /** Blue 4.3 - World Normal blur control **/

        // Blue comment box (4.3) - World Normal blur control
        FVector2D BlueBlurCommentPos(-8550, -1980);
        FVector2D BlueBlurCommentSize(700, 550);
        FString BlueBlurCommentText = TEXT("World Normal blur control");
        FColor BlueBlurCommentColor =FColor::FromHex(TEXT("00B6FFFF"));
        UMaterialExpressionComment* BlueBlurComment = MaterialUtils::CreateCommentNode(Material, BlueBlurCommentPos, BlueBlurCommentSize, BlueBlurCommentText, BlueBlurCommentColor);
        Expressions.Add(BlueBlurComment);

        
        // Lerp-node
        FVector2D BlueBlurLerpNodePos(-8000, -1830);
        UMaterialExpressionLinearInterpolate* BlueBlurLerpNode = MaterialUtils::CreateLerpNode(Material, BlueBlurLerpNodePos);
        Expressions.Add(BlueBlurLerpNode);

        BackgroundMaskNode->Input.Connect(0, BlueBlurLerpNode);


        // SceneTexture:WorldNormal-node
        FVector2D SceneTextureWorldNormalNodePos(-8450, -1900);
        UMaterialExpressionMaterialFunctionCall* SceneTextureWorldNormalNode = MaterialUtils::CreateSceneTextureWorldNormal(Material, SceneTextureWorldNormalNodePos);
        Expressions.Add(SceneTextureWorldNormalNode);
        
        SceneTextureWorldNormalNode->UpdateFromFunctionResource();
        BlueBlurLerpNode->A.Connect(0, SceneTextureWorldNormalNode);


        // Clamp-node
        FVector2D BlueBlurClampNodePos(-8250, -1670);
        UMaterialExpressionClamp* BlueBlurClampNode = MaterialUtils::CreateClampNode(Material, BlueBlurClampNodePos, 0.0f, 2.0f);
        Expressions.Add(BlueBlurClampNode);
        
        BlueBlurLerpNode->Alpha.Connect(0, BlueBlurClampNode);

        UE_LOG(LogTemp, Warning, TEXT("Hello from BlueBlurClampNode!"));

        // ThermalSettingsBlur-node
        FVector2D ThermalSettingsBlurNodePos(-8500, -1630);
        UMaterialExpressionCollectionParameter* ThermalSettingsBlurNode = MaterialUtils::CreateThermalSettingsCPNode(Material, ThermalSettingsBlurNodePos, TEXT("Blur"), EThermalSettingsParamType::Scalar);
        Expressions.Add(ThermalSettingsBlurNode);

        BlueBlurClampNode->Input.Connect(0, ThermalSettingsBlurNode);
        
        UE_LOG(LogTemp, Warning, TEXT("Hello from ThermalSettingsBlurNode!"));
        
        /** Blue 4.4 - World Normal blur **/

        // Blue comment box (4.4) - World Normal blur
        FVector2D WorldNormalCommentPos(-11500, -2900);
        FVector2D WorldNormalCommentSize(2900, 1500);
        FString WorldNormalCommentText = TEXT("World Normal blur");
        FColor WorldNormalCommentColor =FColor::FromHex(TEXT("00B6FFFF"));
        UMaterialExpressionComment* WorldNormalComment = MaterialUtils::CreateCommentNode(Material, WorldNormalCommentPos, WorldNormalCommentSize, WorldNormalCommentText, WorldNormalCommentColor);
        Expressions.Add(WorldNormalComment);


        // Add-node
        FVector2D WorldNormalAddNodeStartPos(-8800, -1780);
        UMaterialExpressionAdd* WorldNormalAddNodeStart = MaterialUtils::CreateAddNode(Material, WorldNormalAddNodeStartPos);
        Expressions.Add(WorldNormalAddNodeStart);
        
        BlueBlurLerpNode->B.Connect(0, WorldNormalAddNodeStart);

        //* Groups of Add-nodes
        
        // Add-node 1
        FVector2D WorldNormalAddNode1Pos(-9100, -1940);
        UMaterialExpressionAdd* WorldNormalAddNode1 = MaterialUtils::CreateAddNode(Material, WorldNormalAddNode1Pos);
        Expressions.Add(WorldNormalAddNode1);

        WorldNormalAddNodeStart->A.Connect(0, WorldNormalAddNode1);

        // Add-node 2
        FVector2D WorldNormalAddNode2Pos(-9100, -2100);
        UMaterialExpressionAdd* WorldNormalAddNode2 = MaterialUtils::CreateAddNode(Material, WorldNormalAddNode2Pos);
        Expressions.Add(WorldNormalAddNode2);

        WorldNormalAddNode1->A.Connect(0, WorldNormalAddNode2);

        // Add-node 3
        FVector2D WorldNormalAddNode3Pos(-9100, -2260);
        UMaterialExpressionAdd* WorldNormalAddNode3 = MaterialUtils::CreateAddNode(Material, WorldNormalAddNode3Pos);
        Expressions.Add(WorldNormalAddNode3);

        WorldNormalAddNode2->A.Connect(0, WorldNormalAddNode3);

        // Add-node 4
        FVector2D WorldNormalAddNode4Pos(-9100, -2420);
        UMaterialExpressionAdd* WorldNormalAddNode4 = MaterialUtils::CreateAddNode(Material, WorldNormalAddNode4Pos);
        Expressions.Add(WorldNormalAddNode4);
        
        WorldNormalAddNode3->A.Connect(0, WorldNormalAddNode4);

        // Add-node 5
        FVector2D WorldNormalAddNode5Pos(-9100, -2580);
        UMaterialExpressionAdd* WorldNormalAddNode5 = MaterialUtils::CreateAddNode(Material, WorldNormalAddNode5Pos);
        Expressions.Add(WorldNormalAddNode5);

        WorldNormalAddNode4->A.Connect(0, WorldNormalAddNode5);


        //* Group of Multiply-nodes

        // Multiply-node 1 -1880
        FVector2D WorldNormalMultiplyNode1Pos(-9400, -1690);
        UMaterialExpressionMultiply* WorldNormalMultiplyNode1 = MaterialUtils::CreateMultiplyNode(Material, WorldNormalMultiplyNode1Pos, 0.1167f);
        Expressions.Add(WorldNormalMultiplyNode1);
        WorldNormalAddNodeStart->B.Connect(0, WorldNormalMultiplyNode1);

        // Multiply-node 2
        FVector2D WorldNormalMultiplyNode2Pos(-9400, -1860);
        UMaterialExpressionMultiply* WorldNormalMultiplyNode2 = MaterialUtils::CreateMultiplyNode(Material, WorldNormalMultiplyNode2Pos, 0.1167f);
        Expressions.Add(WorldNormalMultiplyNode2);
        WorldNormalAddNode1->B.Connect(0, WorldNormalMultiplyNode2);

        // Multiply-node 3
        FVector2D WorldNormalMultiplyNode3Pos(-9400, -2020);
        UMaterialExpressionMultiply* WorldNormalMultiplyNode3 = MaterialUtils::CreateMultiplyNode(Material, WorldNormalMultiplyNode3Pos, 0.1167f);
        Expressions.Add(WorldNormalMultiplyNode3);
        WorldNormalAddNode2->B.Connect(0, WorldNormalMultiplyNode3);

        // Multiply-node 4
        FVector2D WorldNormalMultiplyNode4Pos(-9400, -2180);
        UMaterialExpressionMultiply* WorldNormalMultiplyNode4 = MaterialUtils::CreateMultiplyNode(Material, WorldNormalMultiplyNode4Pos, 0.1167f);
        Expressions.Add(WorldNormalMultiplyNode4);
        WorldNormalAddNode3->B.Connect(0, WorldNormalMultiplyNode4);

        // Multiply-node 5
        FVector2D WorldNormalMultiplyNode5Pos(-9400, -2340);
        UMaterialExpressionMultiply* WorldNormalMultiplyNode5 = MaterialUtils::CreateMultiplyNode(Material, WorldNormalMultiplyNode5Pos, 0.1167f);
        Expressions.Add(WorldNormalMultiplyNode5);
        WorldNormalAddNode4->B.Connect(0, WorldNormalMultiplyNode5);

        // Multiply-node 6
        FVector2D WorldNormalMultiplyNode6Pos(-9400, -2500);
        UMaterialExpressionMultiply* WorldNormalMultiplyNode6 = MaterialUtils::CreateMultiplyNode(Material, WorldNormalMultiplyNode6Pos, 0.1167f);
        Expressions.Add(WorldNormalMultiplyNode6);
        WorldNormalAddNode5->B.Connect(0, WorldNormalMultiplyNode6);

        // Multiply-node 7
        FVector2D WorldNormalMultiplyNode7Pos(-9400, -2660);
        UMaterialExpressionMultiply* WorldNormalMultiplyNode7 = MaterialUtils::CreateMultiplyNode(Material, WorldNormalMultiplyNode7Pos, 0.3f);
        Expressions.Add(WorldNormalMultiplyNode7);
        WorldNormalAddNode5->A.Connect(0, WorldNormalMultiplyNode7);


        //* Group of SceneTexture:WorldNormal-nodes

        // SceneTexture:WorldNormal-node 1
        FVector2D SceneTextureWorldNormalNode1Pos(-9750, -1690);
        UMaterialExpressionMaterialFunctionCall* SceneTextureWorldNormalNode1 = MaterialUtils::CreateSceneTextureWorldNormal(Material, SceneTextureWorldNormalNode1Pos);
        Expressions.Add(SceneTextureWorldNormalNode1);
        SceneTextureWorldNormalNode1->UpdateFromFunctionResource();
        WorldNormalMultiplyNode1->A.Connect(0, SceneTextureWorldNormalNode1);
        
        // SceneTexture:WorldNormal-node 2
        FVector2D SceneTextureWorldNormalNode2Pos(-9750, -1860);
        UMaterialExpressionMaterialFunctionCall* SceneTextureWorldNormalNode2 = MaterialUtils::CreateSceneTextureWorldNormal(Material, SceneTextureWorldNormalNode2Pos);
        Expressions.Add(SceneTextureWorldNormalNode2);
        SceneTextureWorldNormalNode2->UpdateFromFunctionResource();
        WorldNormalMultiplyNode2->A.Connect(0, SceneTextureWorldNormalNode2);

        // SceneTexture:WorldNormal-node 3
        FVector2D SceneTextureWorldNormalNode3Pos(-9750, -2020);
        UMaterialExpressionMaterialFunctionCall* SceneTextureWorldNormalNode3 = MaterialUtils::CreateSceneTextureWorldNormal(Material, SceneTextureWorldNormalNode3Pos);
        Expressions.Add(SceneTextureWorldNormalNode3);
        SceneTextureWorldNormalNode3->UpdateFromFunctionResource();
        WorldNormalMultiplyNode3->A.Connect(0, SceneTextureWorldNormalNode3);

        // SceneTexture:WorldNormal-node 4
        FVector2D SceneTextureWorldNormalNode4Pos(-9750, -2180);
        UMaterialExpressionMaterialFunctionCall* SceneTextureWorldNormalNode4 = MaterialUtils::CreateSceneTextureWorldNormal(Material, SceneTextureWorldNormalNode4Pos);
        Expressions.Add(SceneTextureWorldNormalNode4);
        SceneTextureWorldNormalNode4->UpdateFromFunctionResource();
        WorldNormalMultiplyNode4->A.Connect(0, SceneTextureWorldNormalNode4);

        // SceneTexture:WorldNormal-node 5
        FVector2D SceneTextureWorldNormalNode5Pos(-9750, -2340);
        UMaterialExpressionMaterialFunctionCall* SceneTextureWorldNormalNode5 = MaterialUtils::CreateSceneTextureWorldNormal(Material, SceneTextureWorldNormalNode5Pos);
        Expressions.Add(SceneTextureWorldNormalNode5);
        SceneTextureWorldNormalNode5->UpdateFromFunctionResource();
        WorldNormalMultiplyNode5->A.Connect(0, SceneTextureWorldNormalNode5);

        // SceneTexture:WorldNormal-node 6
        FVector2D SceneTextureWorldNormalNode6Pos(-9750, -2500);
        UMaterialExpressionMaterialFunctionCall* SceneTextureWorldNormalNode6 = MaterialUtils::CreateSceneTextureWorldNormal(Material, SceneTextureWorldNormalNode6Pos);
        Expressions.Add(SceneTextureWorldNormalNode6);
        SceneTextureWorldNormalNode6->UpdateFromFunctionResource();
        WorldNormalMultiplyNode6->A.Connect(0, SceneTextureWorldNormalNode6);

        // SceneTexture:WorldNormal-node 7
        FVector2D SceneTextureWorldNormalNode7Pos(-9750, -2660);
        UMaterialExpressionMaterialFunctionCall* SceneTextureWorldNormalNode7 = MaterialUtils::CreateSceneTextureWorldNormal(Material, SceneTextureWorldNormalNode7Pos);
        Expressions.Add(SceneTextureWorldNormalNode7);
        SceneTextureWorldNormalNode7->UpdateFromFunctionResource();
        WorldNormalMultiplyNode7->A.Connect(0, SceneTextureWorldNormalNode7);


        /*** Blue 4.3 White inside - UV Coordinates for color sample ***/
        
        // Blue 4.2 inside comment box - UV Coordinates for color sample
        FVector2D BlueUVCoordCommentPos(-10550, -2800);
        FVector2D BlueUVCoordCCommentSize(700, 1350);
        FString BlueUVCoordCCommentText = TEXT("UV Coordinates for color sample");
        UMaterialExpressionComment* BlueUVCoordComment = MaterialUtils::CreateCommentNode(Material, BlueUVCoordCommentPos, BlueUVCoordCCommentSize, BlueUVCoordCCommentText);
        Expressions.Add(BlueUVCoordComment);


        //* Groups of Add-nodes
        
        // Add-node 1
        FVector2D BlueUVCoordAddNode1Pos(-10020, -1660);
        UMaterialExpressionAdd* BlueUVCoordAddNode1 = MaterialUtils::CreateAddNode(Material, BlueUVCoordAddNode1Pos);
        Expressions.Add(BlueUVCoordAddNode1);
        
        for (FFunctionExpressionInput& Input : SceneTextureWorldNormalNode1->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("UVs"))
            {
                Input.Input.Connect(0, BlueUVCoordAddNode1);
            }
        }

        // Add-node 2 
        FVector2D BlueUVCoordAddNode2Pos(-10020, -1820);
        UMaterialExpressionAdd* BlueUVCoordAddNode2 = MaterialUtils::CreateAddNode(Material, BlueUVCoordAddNode2Pos);
        Expressions.Add(BlueUVCoordAddNode2);

        for (FFunctionExpressionInput& Input : SceneTextureWorldNormalNode2->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("UVs"))
            {
                Input.Input.Connect(0, BlueUVCoordAddNode2);
            }
        }

        // Add-node 3 
        FVector2D BlueUVCoordAddNode3Pos(-10020, -1980);
        UMaterialExpressionAdd* BlueUVCoordAddNode3 = MaterialUtils::CreateAddNode(Material, BlueUVCoordAddNode3Pos);
        Expressions.Add(BlueUVCoordAddNode3);

        for (FFunctionExpressionInput& Input : SceneTextureWorldNormalNode3->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("UVs"))
            {
                Input.Input.Connect(0, BlueUVCoordAddNode3);
            }
        }

        // Add-node 4 
        FVector2D BlueUVCoordAddNode4Pos(-10020, -2140);
        UMaterialExpressionAdd* BlueUVCoordAddNode4 = MaterialUtils::CreateAddNode(Material, BlueUVCoordAddNode4Pos);
        Expressions.Add(BlueUVCoordAddNode4);

        for (FFunctionExpressionInput& Input : SceneTextureWorldNormalNode4->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("UVs"))
            {
                Input.Input.Connect(0, BlueUVCoordAddNode4);
            }
        }

        // Add-node 5 
        FVector2D BlueUVCoordAddNode5Pos(-10020, -2300);
        UMaterialExpressionAdd* BlueUVCoordAddNode5 = MaterialUtils::CreateAddNode(Material, BlueUVCoordAddNode5Pos);
        Expressions.Add(BlueUVCoordAddNode5);

        for (FFunctionExpressionInput& Input : SceneTextureWorldNormalNode5->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("UVs"))
            {
                Input.Input.Connect(0, BlueUVCoordAddNode5);
            }
        }
        
        // Add-node 6 
        FVector2D BlueUVCoordAddNode6Pos(-10020, -2460);
        UMaterialExpressionAdd* BlueUVCoordAddNode6 = MaterialUtils::CreateAddNode(Material, BlueUVCoordAddNode6Pos);
        Expressions.Add(BlueUVCoordAddNode6);

        for (FFunctionExpressionInput& Input : SceneTextureWorldNormalNode6->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("UVs"))
            {
                Input.Input.Connect(0, BlueUVCoordAddNode6);
            }
        }

        // Add-node 7 
        FVector2D BlueUVCoordAddNode7Pos(-10020, -2620);
        UMaterialExpressionAdd* BlueUVCoordAddNode7 = MaterialUtils::CreateAddNode(Material, BlueUVCoordAddNode7Pos);
        Expressions.Add(BlueUVCoordAddNode7);

        for (FFunctionExpressionInput& Input : SceneTextureWorldNormalNode7->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("UVs"))
            {
                Input.Input.Connect(0, BlueUVCoordAddNode7);
            }
        }


        // TextureCoordinate-node
        FVector2D BlueUVCoordTextureCoordinateNodePos(-10300, -2750);
        UMaterialExpressionTextureCoordinate* BlueUVCoordTextureCoordinateNode = MaterialUtils::CreateTextureCoordinateNode(Material, BlueUVCoordTextureCoordinateNodePos);
        Expressions.Add(BlueUVCoordTextureCoordinateNode);

        BlueUVCoordAddNode1->A.Connect(0, BlueUVCoordTextureCoordinateNode);
        BlueUVCoordAddNode2->A.Connect(0, BlueUVCoordTextureCoordinateNode);
        BlueUVCoordAddNode3->A.Connect(0, BlueUVCoordTextureCoordinateNode);
        BlueUVCoordAddNode4->A.Connect(0, BlueUVCoordTextureCoordinateNode);
        BlueUVCoordAddNode5->A.Connect(0, BlueUVCoordTextureCoordinateNode);
        BlueUVCoordAddNode6->A.Connect(0, BlueUVCoordTextureCoordinateNode);
        BlueUVCoordAddNode7->A.Connect(0, BlueUVCoordTextureCoordinateNode);
        

        
        //* Group of Multiply-nodes

        // Multiply-node 1 -1880
        FVector2D BlueUVCoordMultiplyNode1Pos(-10250, -1620);
        UMaterialExpressionMultiply* BlueUVCoordMultiplyNode1 = MaterialUtils::CreateMultiplyNode(Material, BlueUVCoordMultiplyNode1Pos);
        Expressions.Add(BlueUVCoordMultiplyNode1);
        BlueUVCoordAddNode1->B.Connect(0, BlueUVCoordMultiplyNode1);

        // Multiply-node 2
        FVector2D BlueUVCoordMultiplyNode2Pos(-10250, -1780);
        UMaterialExpressionMultiply* BlueUVCoordMultiplyNode2 = MaterialUtils::CreateMultiplyNode(Material, BlueUVCoordMultiplyNode2Pos);
        Expressions.Add(BlueUVCoordMultiplyNode2);
        BlueUVCoordAddNode2->B.Connect(0, BlueUVCoordMultiplyNode2);

        // Multiply-node 3
        FVector2D BlueUVCoordMultiplyNode3Pos(-10250, -1940);
        UMaterialExpressionMultiply* BlueUVCoordMultiplyNode3 = MaterialUtils::CreateMultiplyNode(Material, BlueUVCoordMultiplyNode3Pos);
        Expressions.Add(BlueUVCoordMultiplyNode3);
        BlueUVCoordAddNode3->B.Connect(0, BlueUVCoordMultiplyNode3);

        // Multiply-node 4
        FVector2D BlueUVCoordMultiplyNode4Pos(-10250, -2100);
        UMaterialExpressionMultiply* BlueUVCoordMultiplyNode4 = MaterialUtils::CreateMultiplyNode(Material, BlueUVCoordMultiplyNode4Pos);
        Expressions.Add(BlueUVCoordMultiplyNode4);
        BlueUVCoordAddNode4->B.Connect(0, BlueUVCoordMultiplyNode4);
        
        // Multiply-node 5
        FVector2D BlueUVCoordMultiplyNode5Pos(-10250, -2260);
        UMaterialExpressionMultiply* BlueUVCoordMultiplyNode5 = MaterialUtils::CreateMultiplyNode(Material, BlueUVCoordMultiplyNode5Pos);
        Expressions.Add(BlueUVCoordMultiplyNode5);
        BlueUVCoordAddNode5->B.Connect(0, BlueUVCoordMultiplyNode5);
        
        // Multiply-node 6
        FVector2D BlueUVCoordMultiplyNode6Pos(-10250, -2420);
        UMaterialExpressionMultiply* BlueUVCoordMultiplyNode6 = MaterialUtils::CreateMultiplyNode(Material, BlueUVCoordMultiplyNode6Pos);
        Expressions.Add(BlueUVCoordMultiplyNode6);
        BlueUVCoordAddNode6->B.Connect(0, BlueUVCoordMultiplyNode6);
        
        // Multiply-node 7
        FVector2D BlueUVCoordMultiplyNode7Pos(-10250, -2580);
        UMaterialExpressionMultiply* BlueUVCoordMultiplyNode7 = MaterialUtils::CreateMultiplyNode(Material, BlueUVCoordMultiplyNode7Pos);
        Expressions.Add(BlueUVCoordMultiplyNode7);
        BlueUVCoordAddNode7->B.Connect(0, BlueUVCoordMultiplyNode7);


        //* Group of Constant2Vector -nodes

        // -1660 -1820 -1980 -2140 -2300 -2460 -2620
        
        // Constant 2Vector-node 1
        FVector2D BlueUVCoordConstant2VectorNode1Pos(-10480, -1680);
        UMaterialExpressionConstant2Vector* BlueUVCoordConstant2VectorNode1 = MaterialUtils::CreateConstant2VectorNode(Material, BlueUVCoordConstant2VectorNode1Pos, -1.0f, -2.0f);
        Expressions.Add(BlueUVCoordConstant2VectorNode1);
        BlueUVCoordMultiplyNode1->A.Connect(2, BlueUVCoordConstant2VectorNode1);

        // Constant 2Vector-node 2
        FVector2D BlueUVCoordConstant2VectorNode2Pos(-10480, -1840);
        UMaterialExpressionConstant2Vector* BlueUVCoordConstant2VectorNode2 = MaterialUtils::CreateConstant2VectorNode(Material, BlueUVCoordConstant2VectorNode2Pos, -1.0f, 2.0f);
        Expressions.Add(BlueUVCoordConstant2VectorNode2);
        BlueUVCoordMultiplyNode2->A.Connect(2, BlueUVCoordConstant2VectorNode2);
        
        // Constant 2Vector-node 3
        FVector2D BlueUVCoordConstant2VectorNode3Pos(-10480, -2000);
        UMaterialExpressionConstant2Vector* BlueUVCoordConstant2VectorNode3 = MaterialUtils::CreateConstant2VectorNode(Material, BlueUVCoordConstant2VectorNode3Pos, 1.0f, -2.0f);
        Expressions.Add(BlueUVCoordConstant2VectorNode3);
        BlueUVCoordMultiplyNode3->A.Connect(2, BlueUVCoordConstant2VectorNode3);
        
        // Constant 2Vector-node 4
        FVector2D BlueUVCoordConstant2VectorNode4Pos(-10480, -2160);
        UMaterialExpressionConstant2Vector* BlueUVCoordConstant2VectorNode4 = MaterialUtils::CreateConstant2VectorNode(Material, BlueUVCoordConstant2VectorNode4Pos, 1.0f, 2.0f);
        Expressions.Add(BlueUVCoordConstant2VectorNode4);
        BlueUVCoordMultiplyNode4->A.Connect(2, BlueUVCoordConstant2VectorNode4);
            
        // Constant 2Vector-node 5
        FVector2D BlueUVCoordConstant2VectorNode5Pos(-10480, -2320);
        UMaterialExpressionConstant2Vector* BlueUVCoordConstant2VectorNode5 = MaterialUtils::CreateConstant2VectorNode(Material, BlueUVCoordConstant2VectorNode5Pos, -2.0f, 0.0f);
        Expressions.Add(BlueUVCoordConstant2VectorNode5);
        BlueUVCoordMultiplyNode5->A.Connect(2, BlueUVCoordConstant2VectorNode5);
        
        // Constant 2Vector-node 6
        FVector2D BlueUVCoordConstant2VectorNode6Pos(-10480, -2480);
        UMaterialExpressionConstant2Vector* BlueUVCoordConstant2VectorNode6 = MaterialUtils::CreateConstant2VectorNode(Material, BlueUVCoordConstant2VectorNode6Pos, 2.0f, 0.0f);
        Expressions.Add(BlueUVCoordConstant2VectorNode6);
        BlueUVCoordMultiplyNode6->A.Connect(2, BlueUVCoordConstant2VectorNode6);
        
        // Constant 2Vector-node 7
        FVector2D BlueUVCoordConstant2VectorNode7Pos(-10480, -2640);
        UMaterialExpressionConstant2Vector* BlueUVCoordConstant2VectorNode7 = MaterialUtils::CreateConstant2VectorNode(Material, BlueUVCoordConstant2VectorNode7Pos, 0.0f, 0.0f);
        Expressions.Add(BlueUVCoordConstant2VectorNode7);
        BlueUVCoordMultiplyNode7->A.Connect(2, BlueUVCoordConstant2VectorNode7);

        /*** Blue 4.3 White inside - Get size of a pixel ***/
        
        // Blue 4.3 inside comment box - Get size of a pixel
        FVector2D PixelSizeCommentPos(-11330, -2260);
        FVector2D PixelSizeCommentSize(500, 300);
        FString PixelSizeCommentText = TEXT("Get size of a pixel");
        UMaterialExpressionComment* PixelSizeComment = MaterialUtils::CreateCommentNode(Material, PixelSizeCommentPos, PixelSizeCommentSize, PixelSizeCommentText);
        Expressions.Add(PixelSizeComment);


        // Divide-node
        FVector2D PixelSizeDivideNodePos(-11030, -2140);
        UMaterialExpressionDivide* PixelSizeDivideNode = MaterialUtils::CreateDivideNode(Material, PixelSizeDivideNodePos);
        Expressions.Add(PixelSizeDivideNode);

        BlueUVCoordMultiplyNode1->B.Connect(0, PixelSizeDivideNode);
        BlueUVCoordMultiplyNode2->B.Connect(0, PixelSizeDivideNode);
        BlueUVCoordMultiplyNode3->B.Connect(0, PixelSizeDivideNode);
        BlueUVCoordMultiplyNode4->B.Connect(0, PixelSizeDivideNode);
        BlueUVCoordMultiplyNode5->B.Connect(0, PixelSizeDivideNode);
        BlueUVCoordMultiplyNode6->B.Connect(0, PixelSizeDivideNode);
        BlueUVCoordMultiplyNode7->B.Connect(0, PixelSizeDivideNode);


        // Constant node
        FVector2D PixelSizeConstantNodePos(-11230, -2180);
        UMaterialExpressionConstant* PixelSizeConstantNode = MaterialUtils::CreateConstantNode(Material, PixelSizeConstantNodePos, 1.0f);
        Expressions.Add(PixelSizeConstantNode);
        PixelSizeDivideNode->A.Connect(0, PixelSizeConstantNode);


        // ScreenResolution node
        FVector2D PixelSizeScreenResolutionNodePos(-11230, -2080);
        UMaterialExpressionMaterialFunctionCall* PixelSizeScreenResolutionNode = MaterialUtils::CreateScreenResolution(Material, PixelSizeScreenResolutionNodePos);
        Expressions.Add(PixelSizeScreenResolutionNode);
        PixelSizeScreenResolutionNode->UpdateFromFunctionResource();
        PixelSizeDivideNode->B.Connect(0, PixelSizeScreenResolutionNode);


        FNodeArea4Result Result; 

        Result.Area4Background3ColorBlendNode = Background3ColorBlendNode;
        Result.Area4AddSkyMultiplyNode = AddSkyMultiplyNode;
        
        return Result;
    }

    struct FNodeArea5Result
    {
        UMaterialExpressionMaterialFunctionCall* Area5ThermalActor3ColorBlendNode = nullptr;
        UMaterialExpressionLinearInterpolate* Area5GreenBlurLerpNode = nullptr;
    };

    static FNodeArea5Result CreateNodeArea5(UMaterial* Material, TArray<TObjectPtr<UMaterialExpression>>& Expressions)
    {
        FNodeArea5Result Result;
       
        /* 5 - Green area */

        /** Green 5.1 - Thermal actor color **/
        
        // Green 5.1 comment box - Thermal actor color
        FVector2D ThermalActorCommentPos(-6960, -150);
        FVector2D ThermalActorCommentSize(660, 1050);
        FString ThermalActorCommentText = TEXT("Thermal actor color");
        FColor ThermalActorCommentColor =FColor::FromHex(TEXT("5FFF90FF"));
        UMaterialExpressionComment* ThermalActorComment = MaterialUtils::CreateCommentNode(Material, ThermalActorCommentPos, ThermalActorCommentSize, ThermalActorCommentText, ThermalActorCommentColor);
        Expressions.Add(ThermalActorComment);
        

        // 3ColorBlend-node
        FVector2D ThermalActor3ColorBlendNodePos(-6500, 240);
        UMaterialExpressionMaterialFunctionCall* ThermalActor3ColorBlendNode = MaterialUtils::Create3ColorBlendNode(Material, ThermalActor3ColorBlendNodePos);
        Expressions.Add(ThermalActor3ColorBlendNode);

        ThermalActor3ColorBlendNode->UpdateFromFunctionResource();
        

        // MPC_ThermalSettings "Cold" node -6500, 180
        FVector2D ThermalActorThermalSettingsColdPos(-6800, -80);
        UMaterialExpressionCollectionParameter* ThermalActorThermalSettingsColdNode = MaterialUtils::CreateThermalSettingsCPNode(Material, ThermalActorThermalSettingsColdPos, TEXT("Cold"), EThermalSettingsParamType::Vector);
        Expressions.Add(ThermalActorThermalSettingsColdNode);
        
        for (FFunctionExpressionInput& Input : ThermalActor3ColorBlendNode->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("A"))
            {
                Input.Input.Connect(0, ThermalActorThermalSettingsColdNode);
            }
        }

        // MPC_ThermalSettings "Mid" node
        FVector2D ThermalActorThermalSettingsMidPos(-6800, 120);
        UMaterialExpressionCollectionParameter* ThermalActorThermalSettingsMidNode = MaterialUtils::CreateThermalSettingsCPNode(Material, ThermalActorThermalSettingsMidPos, TEXT("Mid"), EThermalSettingsParamType::Vector);
        Expressions.Add(ThermalActorThermalSettingsMidNode);
        
        for (FFunctionExpressionInput& Input : ThermalActor3ColorBlendNode->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("B"))
            {
                Input.Input.Connect(0, ThermalActorThermalSettingsMidNode);
            }
            
        }

        // MPC_ThermalSettings "Hot" node
        FVector2D ThermalActorThermalSettingsHotPos(-6800, 320);
        UMaterialExpressionCollectionParameter* ThermalActorThermalSettingsHotNode = MaterialUtils::CreateThermalSettingsCPNode(Material, ThermalActorThermalSettingsHotPos, TEXT("Hot"), EThermalSettingsParamType::Vector);
        Expressions.Add(ThermalActorThermalSettingsHotNode);
        
        for (FFunctionExpressionInput& Input : ThermalActor3ColorBlendNode->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("C"))
            {
                Input.Input.Connect(0, ThermalActorThermalSettingsHotNode);
            }
            
        }


        // Power-node
        FVector2D ThermalActorPowerNodePos(-6700, 580);
        UMaterialExpressionPower* ThermalActorPowerNode = MaterialUtils::CreatePowerNode(Material, ThermalActorPowerNodePos);
        Expressions.Add(ThermalActorPowerNode);
        
        for (FFunctionExpressionInput& Input : ThermalActor3ColorBlendNode->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("Alpha"))
            {
                Input.Input.Connect(0, ThermalActorPowerNode);
            }
        }

        // Mask-node
        FVector2D ThermalActorMaskNodePos(-6900, 580);
        UMaterialExpressionComponentMask* ThermalActorMaskNode = MaterialUtils::CreateMaskNode(Material, ThermalActorMaskNodePos, true, false, false);
        Expressions.Add(ThermalActorMaskNode);
        ThermalActorPowerNode->Base.Connect(0, ThermalActorMaskNode);

        // Constant-node
        FVector2D ThermalActorConstantNodePos(-6900, 730);
        UMaterialExpressionConstant* ThermalActorConstantNode = MaterialUtils::CreateConstantNode(Material, ThermalActorConstantNodePos, 1.0f);
        ThermalActorConstantNode->SetParameterName("powerExpActors");
        Expressions.Add(ThermalActorConstantNode);
        ThermalActorPowerNode->Exponent.Connect(0, ThermalActorConstantNode);


        
        /** Green 5.2 - PostProcessInput0 blur control **/

        // Green comment box (5.2) - PostProcessInput0 blur control
        FVector2D GreenBlurCommentPos(-8550, -150);
        FVector2D GreenBlurCommentSize(700, 550);
        FString GreenBlurCommentText = TEXT("PostProcessInput0 blur control");
        FColor GreenBlurCommentColor =FColor::FromHex(TEXT("5FFF90FF"));
        UMaterialExpressionComment* GreenBlurComment = MaterialUtils::CreateCommentNode(Material, GreenBlurCommentPos, GreenBlurCommentSize, GreenBlurCommentText, GreenBlurCommentColor);
        Expressions.Add(GreenBlurComment);


        // Lerp-node
        FVector2D GreenBlurLerpNodePos(-8000, 23);
        UMaterialExpressionLinearInterpolate* GreenBlurLerpNode = MaterialUtils::CreateLerpNode(Material, GreenBlurLerpNodePos);
        Expressions.Add(GreenBlurLerpNode);
        ThermalActorMaskNode->Input.Connect(0, GreenBlurLerpNode);
        

        // SceneTexture:PostProcessInput0-node
        FVector2D SceneTexturePostProcessInput0NodePos(-8400, -50);
        UMaterialExpressionMaterialFunctionCall* SceneTexturePostProcessInput0Node = MaterialUtils::CreateSceneTexturePostProcess(Material, SceneTexturePostProcessInput0NodePos);
        Expressions.Add(SceneTexturePostProcessInput0Node);
        SceneTexturePostProcessInput0Node->UpdateFromFunctionResource();
        GreenBlurLerpNode->A.Connect(0, SceneTexturePostProcessInput0Node);


        // Clamp-node
        FVector2D GreenBlurClampNodePos(-8200, 190);
        UMaterialExpressionClamp* GreenBlurClampNode = MaterialUtils::CreateClampNode(Material, GreenBlurClampNodePos, 0.0f, 2.0f);
        Expressions.Add(GreenBlurClampNode);
        GreenBlurLerpNode->Alpha.Connect(0, GreenBlurClampNode);

        UE_LOG(LogTemp, Warning, TEXT("Hello from GreenBlurClampNode"));

        // ThermalSettingsBlur-node
        FVector2D GreenBlurThermalSettingsBlurNodePos(-8450, 190);
        UMaterialExpressionCollectionParameter* GreenBlurThermalSettingsBlurNode = MaterialUtils::CreateThermalSettingsCPNode(Material, GreenBlurThermalSettingsBlurNodePos, TEXT("Blur"), EThermalSettingsParamType::Scalar);
        Expressions.Add(GreenBlurThermalSettingsBlurNode);
        GreenBlurClampNode->Input.Connect(0, GreenBlurThermalSettingsBlurNode);

        UE_LOG(LogTemp, Warning, TEXT("Hello from ThermalSettingsBlurNode"));

        /** Green 5.3 - PostProcessInput0 blur **/

        // Green comment box (5.3) - PostProcessInput0 blur
        FVector2D PostProcessCommentPos(-11500, -1033);
        FVector2D PostProcessCommentSize(2900, 1500);
        FString PostProcessCommentText = TEXT("PostProcessInput0 blur");
        FColor PostProcessCommentColor =FColor::FromHex(TEXT("5FFF90FF"));
        UMaterialExpressionComment* PostProcessComment = MaterialUtils::CreateCommentNode(Material, PostProcessCommentPos, PostProcessCommentSize, PostProcessCommentText, PostProcessCommentColor);
        Expressions.Add(PostProcessComment);


        // Add-node
        FVector2D PostProcessAddNodeStartPos(-8800, 54);
        UMaterialExpressionAdd* PostProcessAddNodeStart = MaterialUtils::CreateAddNode(Material, PostProcessAddNodeStartPos);
        Expressions.Add(PostProcessAddNodeStart);
        
        GreenBlurLerpNode->B.Connect(0, PostProcessAddNodeStart);

        //* Groups of Add-nodes

        
        // Add-node 1
        FVector2D PostProcessAddNode1Pos(-9100, -73);
        UMaterialExpressionAdd* PostProcessAddNode1 = MaterialUtils::CreateAddNode(Material, PostProcessAddNode1Pos);
        Expressions.Add(PostProcessAddNode1);

        PostProcessAddNodeStart->A.Connect(0, PostProcessAddNode1);

        // Add-node 2
        FVector2D PostProcessAddNode2Pos(-9100, -233);
        UMaterialExpressionAdd* PostProcessAddNode2 = MaterialUtils::CreateAddNode(Material, PostProcessAddNode2Pos);
        Expressions.Add(PostProcessAddNode2);

        PostProcessAddNode1->A.Connect(0, PostProcessAddNode2);

        // Add-node 3
        FVector2D PostProcessAddNode3Pos(-9100, -393);
        UMaterialExpressionAdd* PostProcessAddNode3 = MaterialUtils::CreateAddNode(Material, PostProcessAddNode3Pos);
        Expressions.Add(PostProcessAddNode3);

        PostProcessAddNode2->A.Connect(0, PostProcessAddNode3);

        // Add-node 4
        FVector2D PostProcessAddNode4Pos(-9100, -553);
        UMaterialExpressionAdd* PostProcessAddNode4 = MaterialUtils::CreateAddNode(Material, PostProcessAddNode4Pos);
        Expressions.Add(PostProcessAddNode4);
        
        PostProcessAddNode3->A.Connect(0, PostProcessAddNode4);

        // Add-node 5
        FVector2D PostProcessAddNode5Pos(-9100, -713);
        UMaterialExpressionAdd* PostProcessAddNode5 = MaterialUtils::CreateAddNode(Material, PostProcessAddNode5Pos);
        Expressions.Add(PostProcessAddNode5);

        PostProcessAddNode4->A.Connect(0, PostProcessAddNode5);


        //* Group of Multiply-nodes

        // Multiply-node 1 -1880
        FVector2D PostProcessMultiplyNode1Pos(-9400, 177);
        UMaterialExpressionMultiply* PostProcessMultiplyNode1 = MaterialUtils::CreateMultiplyNode(Material, PostProcessMultiplyNode1Pos, 0.1167f);
        Expressions.Add(PostProcessMultiplyNode1);
        PostProcessAddNodeStart->B.Connect(0, PostProcessMultiplyNode1);

        // Multiply-node 2
        FVector2D PostProcessMultiplyNode2Pos(-9400, 7);
        UMaterialExpressionMultiply* PostProcessMultiplyNode2 = MaterialUtils::CreateMultiplyNode(Material, PostProcessMultiplyNode2Pos, 0.1167f);
        Expressions.Add(PostProcessMultiplyNode2);
        PostProcessAddNode1->B.Connect(0, PostProcessMultiplyNode2);

        // Multiply-node 3
        FVector2D PostProcessMultiplyNode3Pos(-9400, -153);
        UMaterialExpressionMultiply* PostProcessMultiplyNode3 = MaterialUtils::CreateMultiplyNode(Material, PostProcessMultiplyNode3Pos, 0.1167f);
        Expressions.Add(PostProcessMultiplyNode3);
        PostProcessAddNode2->B.Connect(0, PostProcessMultiplyNode3);

        // Multiply-node 4
        FVector2D PostProcessMultiplyNode4Pos(-9400, -313);
        UMaterialExpressionMultiply* PostProcessMultiplyNode4 = MaterialUtils::CreateMultiplyNode(Material, PostProcessMultiplyNode4Pos, 0.1167f);
        Expressions.Add(PostProcessMultiplyNode4);
        PostProcessAddNode3->B.Connect(0, PostProcessMultiplyNode4);

        // Multiply-node 5
        FVector2D PostProcessMultiplyNode5Pos(-9400, -473);
        UMaterialExpressionMultiply* PostProcessMultiplyNode5 = MaterialUtils::CreateMultiplyNode(Material, PostProcessMultiplyNode5Pos, 0.1167f);
        Expressions.Add(PostProcessMultiplyNode5);
        PostProcessAddNode4->B.Connect(0, PostProcessMultiplyNode5);

        // Multiply-node 6
        FVector2D PostProcessMultiplyNode6Pos(-9400, -633);
        UMaterialExpressionMultiply* PostProcessMultiplyNode6 = MaterialUtils::CreateMultiplyNode(Material, PostProcessMultiplyNode6Pos, 0.1167f);
        Expressions.Add(PostProcessMultiplyNode6);
        PostProcessAddNode5->B.Connect(0, PostProcessMultiplyNode6);

        // Multiply-node 7
        FVector2D PostProcessMultiplyNode7Pos(-9400, -793);
        UMaterialExpressionMultiply* PostProcessMultiplyNode7 = MaterialUtils::CreateMultiplyNode(Material, PostProcessMultiplyNode7Pos, 0.3f);
        Expressions.Add(PostProcessMultiplyNode7);
        PostProcessAddNode5->A.Connect(0, PostProcessMultiplyNode7);


        //* Group of SceneTexture:PostProcess-nodes

        // SceneTexture:PostProcess-node 1
        FVector2D SceneTexturePostProcessNode1Pos(-9750, 177);
        UMaterialExpressionMaterialFunctionCall* SceneTexturePostProcessNode1 = MaterialUtils::CreateSceneTexturePostProcess(Material, SceneTexturePostProcessNode1Pos);
        Expressions.Add(SceneTexturePostProcessNode1);
        SceneTexturePostProcessNode1->UpdateFromFunctionResource();
        PostProcessMultiplyNode1->A.Connect(0, SceneTexturePostProcessNode1);
        
        // SceneTexture:PostProcess-node 2
        FVector2D SceneTexturePostProcessNode2Pos(-9750, 7);
        UMaterialExpressionMaterialFunctionCall* SceneTexturePostProcessNode2 = MaterialUtils::CreateSceneTexturePostProcess(Material, SceneTexturePostProcessNode2Pos);
        Expressions.Add(SceneTexturePostProcessNode2);
        SceneTexturePostProcessNode2->UpdateFromFunctionResource();
        PostProcessMultiplyNode2->A.Connect(0, SceneTexturePostProcessNode2);

        // SceneTexture:PostProcess-node 3
        FVector2D SceneTexturePostProcessNode3Pos(-9750, -153);
        UMaterialExpressionMaterialFunctionCall* SceneTexturePostProcessNode3 = MaterialUtils::CreateSceneTexturePostProcess(Material, SceneTexturePostProcessNode3Pos);
        Expressions.Add(SceneTexturePostProcessNode3);
        SceneTexturePostProcessNode3->UpdateFromFunctionResource();
        PostProcessMultiplyNode3->A.Connect(0, SceneTexturePostProcessNode3);

        // SceneTexture:PostProcess-node 4
        FVector2D SceneTexturePostProcessNode4Pos(-9750, -313);
        UMaterialExpressionMaterialFunctionCall* SceneTexturePostProcessNode4 = MaterialUtils::CreateSceneTexturePostProcess(Material, SceneTexturePostProcessNode4Pos);
        Expressions.Add(SceneTexturePostProcessNode4);
        SceneTexturePostProcessNode4->UpdateFromFunctionResource();
        PostProcessMultiplyNode4->A.Connect(0, SceneTexturePostProcessNode4);

        // SceneTexture:PostProcess-node 5
        FVector2D SceneTexturePostProcessNode5Pos(-9750, -473);
        UMaterialExpressionMaterialFunctionCall* SceneTexturePostProcessNode5 = MaterialUtils::CreateSceneTexturePostProcess(Material, SceneTexturePostProcessNode5Pos);
        Expressions.Add(SceneTexturePostProcessNode5);
        SceneTexturePostProcessNode5->UpdateFromFunctionResource();
        PostProcessMultiplyNode5->A.Connect(0, SceneTexturePostProcessNode5);

        // SceneTexture:PostProcess-node 6
        FVector2D SceneTexturePostProcessNode6Pos(-9750, -633);
        UMaterialExpressionMaterialFunctionCall* SceneTexturePostProcessNode6 = MaterialUtils::CreateSceneTexturePostProcess(Material, SceneTexturePostProcessNode6Pos);
        Expressions.Add(SceneTexturePostProcessNode6);
        SceneTexturePostProcessNode6->UpdateFromFunctionResource();
        PostProcessMultiplyNode6->A.Connect(0, SceneTexturePostProcessNode6);

        // SceneTexture:PostProcess-node 7
        FVector2D SceneTexturePostProcessNode7Pos(-9750, -793);
        UMaterialExpressionMaterialFunctionCall* SceneTexturePostProcessNode7 = MaterialUtils::CreateSceneTexturePostProcess(Material, SceneTexturePostProcessNode7Pos);
        Expressions.Add(SceneTexturePostProcessNode7);
        SceneTexturePostProcessNode7->UpdateFromFunctionResource();
        PostProcessMultiplyNode7->A.Connect(0, SceneTexturePostProcessNode7);


        /*** Green 5.3 White inside - UV Coordinates for color sample ***/
        
        // Green 5.3 inside comment box - UV Coordinates for color sample
        FVector2D GreenUVCoordCommentPos(-10550, -933);
        FVector2D GreenUVCoordCCommentSize(700, 1350);
        FString GreenUVCoordCCommentText = TEXT("UV Coordinates for color sample");
        UMaterialExpressionComment* GreenUVCoordComment = MaterialUtils::CreateCommentNode(Material, GreenUVCoordCommentPos, GreenUVCoordCCommentSize, GreenUVCoordCCommentText);
        Expressions.Add(GreenUVCoordComment);


        //* Groups of Add-nodes
        
        // Add-node 1
        FVector2D GreenUVCoordAddNode1Pos(-10020, 207);
        UMaterialExpressionAdd* GreenUVCoordAddNode1 = MaterialUtils::CreateAddNode(Material, GreenUVCoordAddNode1Pos);
        Expressions.Add(GreenUVCoordAddNode1);
        
        for (FFunctionExpressionInput& Input : SceneTexturePostProcessNode1->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("UVs"))
            {
                Input.Input.Connect(0, GreenUVCoordAddNode1);
            }
        }

        // Add-node 2 
        FVector2D GreenUVCoordAddNode2Pos(-10020, 47);
        UMaterialExpressionAdd* GreenUVCoordAddNode2 = MaterialUtils::CreateAddNode(Material, GreenUVCoordAddNode2Pos);
        Expressions.Add(GreenUVCoordAddNode2);

        for (FFunctionExpressionInput& Input : SceneTexturePostProcessNode2->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("UVs"))
            {
                Input.Input.Connect(0, GreenUVCoordAddNode2);
            }
        }

        // Add-node 3 
        FVector2D GreenUVCoordAddNode3Pos(-10020, -113);
        UMaterialExpressionAdd* GreenUVCoordAddNode3 = MaterialUtils::CreateAddNode(Material, GreenUVCoordAddNode3Pos);
        Expressions.Add(GreenUVCoordAddNode3);

        for (FFunctionExpressionInput& Input : SceneTexturePostProcessNode3->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("UVs"))
            {
                Input.Input.Connect(0, GreenUVCoordAddNode3);
            }
        }

        // Add-node 4 
        FVector2D GreenUVCoordAddNode4Pos(-10020, -273);
        UMaterialExpressionAdd* GreenUVCoordAddNode4 = MaterialUtils::CreateAddNode(Material, GreenUVCoordAddNode4Pos);
        Expressions.Add(GreenUVCoordAddNode4);

        for (FFunctionExpressionInput& Input : SceneTexturePostProcessNode4->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("UVs"))
            {
                Input.Input.Connect(0, GreenUVCoordAddNode4);
            }
        }

        // Add-node 5 
        FVector2D GreenUVCoordAddNode5Pos(-10020, -433);
        UMaterialExpressionAdd* GreenUVCoordAddNode5 = MaterialUtils::CreateAddNode(Material, GreenUVCoordAddNode5Pos);
        Expressions.Add(GreenUVCoordAddNode5);

        for (FFunctionExpressionInput& Input : SceneTexturePostProcessNode5->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("UVs"))
            {
                Input.Input.Connect(0, GreenUVCoordAddNode5);
            }
        }
        
        // Add-node 6 
        FVector2D GreenUVCoordAddNode6Pos(-10020, -593);
        UMaterialExpressionAdd* GreenUVCoordAddNode6 = MaterialUtils::CreateAddNode(Material, GreenUVCoordAddNode6Pos);
        Expressions.Add(GreenUVCoordAddNode6);

        for (FFunctionExpressionInput& Input : SceneTexturePostProcessNode6->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("UVs"))
            {
                Input.Input.Connect(0, GreenUVCoordAddNode6);
            }
        }

        // Add-node 7 
        FVector2D GreenUVCoordAddNode7Pos(-10020, -753);
        UMaterialExpressionAdd* GreenUVCoordAddNode7 = MaterialUtils::CreateAddNode(Material, GreenUVCoordAddNode7Pos);
        Expressions.Add(GreenUVCoordAddNode7);

        for (FFunctionExpressionInput& Input : SceneTexturePostProcessNode7->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("UVs"))
            {
                Input.Input.Connect(0, GreenUVCoordAddNode7);
            }
        }


        // TextureCoordinate-node
        FVector2D GreenUVCoordTextureCoordinateNodePos(-10300, -883);
        UMaterialExpressionTextureCoordinate* GreenUVCoordTextureCoordinateNode = MaterialUtils::CreateTextureCoordinateNode(Material, GreenUVCoordTextureCoordinateNodePos);
        Expressions.Add(GreenUVCoordTextureCoordinateNode);

        GreenUVCoordAddNode1->A.Connect(0, GreenUVCoordTextureCoordinateNode);
        GreenUVCoordAddNode2->A.Connect(0, GreenUVCoordTextureCoordinateNode);
        GreenUVCoordAddNode3->A.Connect(0, GreenUVCoordTextureCoordinateNode);
        GreenUVCoordAddNode4->A.Connect(0, GreenUVCoordTextureCoordinateNode);
        GreenUVCoordAddNode5->A.Connect(0, GreenUVCoordTextureCoordinateNode);
        GreenUVCoordAddNode6->A.Connect(0, GreenUVCoordTextureCoordinateNode);
        GreenUVCoordAddNode7->A.Connect(0, GreenUVCoordTextureCoordinateNode);
        

        
        //* Group of Multiply-nodes

        // Multiply-node 1 -1880
        FVector2D GreenUVCoordMultiplyNode1Pos(-10250, 247);
        UMaterialExpressionMultiply* GreenUVCoordMultiplyNode1 = MaterialUtils::CreateMultiplyNode(Material, GreenUVCoordMultiplyNode1Pos);
        Expressions.Add(GreenUVCoordMultiplyNode1);
        GreenUVCoordAddNode1->B.Connect(0, GreenUVCoordMultiplyNode1);

        // Multiply-node 2
        FVector2D GreenUVCoordMultiplyNode2Pos(-10250, 87);
        UMaterialExpressionMultiply* GreenUVCoordMultiplyNode2 = MaterialUtils::CreateMultiplyNode(Material, GreenUVCoordMultiplyNode2Pos);
        Expressions.Add(GreenUVCoordMultiplyNode2);
        GreenUVCoordAddNode2->B.Connect(0, GreenUVCoordMultiplyNode2);

        // Multiply-node 3
        FVector2D GreenUVCoordMultiplyNode3Pos(-10250, -73);
        UMaterialExpressionMultiply* GreenUVCoordMultiplyNode3 = MaterialUtils::CreateMultiplyNode(Material, GreenUVCoordMultiplyNode3Pos);
        Expressions.Add(GreenUVCoordMultiplyNode3);
        GreenUVCoordAddNode3->B.Connect(0, GreenUVCoordMultiplyNode3);

        // Multiply-node 4
        FVector2D GreenUVCoordMultiplyNode4Pos(-10250, -233);
        UMaterialExpressionMultiply* GreenUVCoordMultiplyNode4 = MaterialUtils::CreateMultiplyNode(Material, GreenUVCoordMultiplyNode4Pos);
        Expressions.Add(GreenUVCoordMultiplyNode4);
        GreenUVCoordAddNode4->B.Connect(0, GreenUVCoordMultiplyNode4);
        
        // Multiply-node 5
        FVector2D GreenUVCoordMultiplyNode5Pos(-10250, -393);
        UMaterialExpressionMultiply* GreenUVCoordMultiplyNode5 = MaterialUtils::CreateMultiplyNode(Material, GreenUVCoordMultiplyNode5Pos);
        Expressions.Add(GreenUVCoordMultiplyNode5);
        GreenUVCoordAddNode5->B.Connect(0, GreenUVCoordMultiplyNode5);
        
        // Multiply-node 6
        FVector2D GreenUVCoordMultiplyNode6Pos(-10250, -553);
        UMaterialExpressionMultiply* GreenUVCoordMultiplyNode6 = MaterialUtils::CreateMultiplyNode(Material, GreenUVCoordMultiplyNode6Pos);
        Expressions.Add(GreenUVCoordMultiplyNode6);
        GreenUVCoordAddNode6->B.Connect(0, GreenUVCoordMultiplyNode6);
        
        // Multiply-node 7
        FVector2D GreenUVCoordMultiplyNode7Pos(-10250, -713);
        UMaterialExpressionMultiply* GreenUVCoordMultiplyNode7 = MaterialUtils::CreateMultiplyNode(Material, GreenUVCoordMultiplyNode7Pos);
        Expressions.Add(GreenUVCoordMultiplyNode7);
        GreenUVCoordAddNode7->B.Connect(0, GreenUVCoordMultiplyNode7);


        //* Group of Constant2Vector -nodes
        
        // Constant 2Vector-node 1
        FVector2D GreenUVCoordConstant2VectorNode1Pos(-10480, 187);
        UMaterialExpressionConstant2Vector* GreenUVCoordConstant2VectorNode1 = MaterialUtils::CreateConstant2VectorNode(Material, GreenUVCoordConstant2VectorNode1Pos, -1.0f, -2.0f);
        Expressions.Add(GreenUVCoordConstant2VectorNode1);
        GreenUVCoordMultiplyNode1->A.Connect(2, GreenUVCoordConstant2VectorNode1);

        // Constant 2Vector-node 2
        FVector2D GreenUVCoordConstant2VectorNode2Pos(-10480, 27);
        UMaterialExpressionConstant2Vector* GreenUVCoordConstant2VectorNode2 = MaterialUtils::CreateConstant2VectorNode(Material, GreenUVCoordConstant2VectorNode2Pos, -1.0f, 2.0f);
        Expressions.Add(GreenUVCoordConstant2VectorNode2);
        GreenUVCoordMultiplyNode2->A.Connect(2, GreenUVCoordConstant2VectorNode2);
        
        // Constant 2Vector-node 3
        FVector2D GreenUVCoordConstant2VectorNode3Pos(-10480, -133);
        UMaterialExpressionConstant2Vector* GreenUVCoordConstant2VectorNode3 = MaterialUtils::CreateConstant2VectorNode(Material, GreenUVCoordConstant2VectorNode3Pos, 1.0f, -2.0f);
        Expressions.Add(GreenUVCoordConstant2VectorNode3);
        GreenUVCoordMultiplyNode3->A.Connect(2, GreenUVCoordConstant2VectorNode3);
        
        // Constant 2Vector-node 4
        FVector2D GreenUVCoordConstant2VectorNode4Pos(-10480, -293);
        UMaterialExpressionConstant2Vector* GreenUVCoordConstant2VectorNode4 = MaterialUtils::CreateConstant2VectorNode(Material, GreenUVCoordConstant2VectorNode4Pos, 1.0f, 2.0f);
        Expressions.Add(GreenUVCoordConstant2VectorNode4);
        GreenUVCoordMultiplyNode4->A.Connect(2, GreenUVCoordConstant2VectorNode4);
            
        // Constant 2Vector-node 5
        FVector2D GreenUVCoordConstant2VectorNode5Pos(-10480, -453);
        UMaterialExpressionConstant2Vector* GreenUVCoordConstant2VectorNode5 = MaterialUtils::CreateConstant2VectorNode(Material, GreenUVCoordConstant2VectorNode5Pos, -2.0f, 0.0f);
        Expressions.Add(GreenUVCoordConstant2VectorNode5);
        GreenUVCoordMultiplyNode5->A.Connect(2, GreenUVCoordConstant2VectorNode5);
        
        // Constant 2Vector-node 6
        FVector2D GreenUVCoordConstant2VectorNode6Pos(-10480, -613);
        UMaterialExpressionConstant2Vector* GreenUVCoordConstant2VectorNode6 = MaterialUtils::CreateConstant2VectorNode(Material, GreenUVCoordConstant2VectorNode6Pos, 2.0f, 0.0f);
        Expressions.Add(GreenUVCoordConstant2VectorNode6);
        GreenUVCoordMultiplyNode6->A.Connect(2, GreenUVCoordConstant2VectorNode6);
        
        // Constant 2Vector-node 7
        FVector2D GreenUVCoordConstant2VectorNode7Pos(-10480, -773);
        UMaterialExpressionConstant2Vector* GreenUVCoordConstant2VectorNode7 = MaterialUtils::CreateConstant2VectorNode(Material, GreenUVCoordConstant2VectorNode7Pos, 0.0f, 0.0f);
        Expressions.Add(GreenUVCoordConstant2VectorNode7);
        GreenUVCoordMultiplyNode7->A.Connect(2, GreenUVCoordConstant2VectorNode7);

        /*** Green 5.3 White inside - Get size of a pixel ***/
        
        // Green 5.3 inside comment box - Get size of a pixel
        FVector2D GreenPixelSizeCommentPos(-11330, -393);
        FVector2D GreenPixelSizeCommentSize(500, 300);
        FString GreenPixelSizeCommentText = TEXT("Get size of a pixel");
        UMaterialExpressionComment* GreenPixelSizeComment = MaterialUtils::CreateCommentNode(Material, GreenPixelSizeCommentPos, GreenPixelSizeCommentSize, GreenPixelSizeCommentText);
        Expressions.Add(GreenPixelSizeComment);


        // Divide-node
        FVector2D GreenPixelSizeDivideNodePos(-11030, -273);
        UMaterialExpressionDivide* GreenPixelSizeDivideNode = MaterialUtils::CreateDivideNode(Material, GreenPixelSizeDivideNodePos);
        Expressions.Add(GreenPixelSizeDivideNode);

        GreenUVCoordMultiplyNode1->B.Connect(0, GreenPixelSizeDivideNode);
        GreenUVCoordMultiplyNode2->B.Connect(0, GreenPixelSizeDivideNode);
        GreenUVCoordMultiplyNode3->B.Connect(0, GreenPixelSizeDivideNode);
        GreenUVCoordMultiplyNode4->B.Connect(0, GreenPixelSizeDivideNode);
        GreenUVCoordMultiplyNode5->B.Connect(0, GreenPixelSizeDivideNode);
        GreenUVCoordMultiplyNode6->B.Connect(0, GreenPixelSizeDivideNode);
        GreenUVCoordMultiplyNode7->B.Connect(0, GreenPixelSizeDivideNode);


        // Constant node
        FVector2D GreenPixelSizeConstantNodePos(-11230, -313);
        UMaterialExpressionConstant* GreenPixelSizeConstantNode = MaterialUtils::CreateConstantNode(Material, GreenPixelSizeConstantNodePos, 1.0f);
        Expressions.Add(GreenPixelSizeConstantNode);
        GreenPixelSizeDivideNode->A.Connect(0, GreenPixelSizeConstantNode);


        // ScreenResolution node
        FVector2D GreenPixelSizeScreenResolutionNodePos(-11230, -213);
        UMaterialExpressionMaterialFunctionCall* GreenPixelSizeScreenResolutionNode = MaterialUtils::CreateScreenResolution(Material, GreenPixelSizeScreenResolutionNodePos);
        Expressions.Add(GreenPixelSizeScreenResolutionNode);
        GreenPixelSizeScreenResolutionNode->UpdateFromFunctionResource();
        GreenPixelSizeDivideNode->B.Connect(0, GreenPixelSizeScreenResolutionNode);

        /**/

       

        Result.Area5ThermalActor3ColorBlendNode = ThermalActor3ColorBlendNode;
        Result.Area5GreenBlurLerpNode = GreenBlurLerpNode;

        return Result;
    }

    static UMaterialExpressionIf* CreateNodeArea6(UMaterial* Material, TArray<TObjectPtr<UMaterialExpression>>& Expressions)
    {
        /* 6 - Orange area */

        /** Orange 6.1 - Create heat mask **/
        
        // Orange 6.1 comment box - Create heat mask
        FVector2D HeatMaskCommentPos(-7310, 1150);
        FVector2D HeatMaskCommentSize(1000, 670);
        FString HeatMaskCommentText = TEXT("Create heat mask");
        FColor HeatMaskCommentColor =FColor::FromHex(TEXT("FFD26BFF"));
        UMaterialExpressionComment* HeatMaskComment = MaterialUtils::CreateCommentNode(Material, HeatMaskCommentPos, HeatMaskCommentSize, HeatMaskCommentText, HeatMaskCommentColor);
        Expressions.Add(HeatMaskComment);


        // If-node
        FVector2D HeatMaskIfNodePos(-6510, 1400);
        UMaterialExpressionIf* HeatMaskIfNode = MaterialUtils::CreateIfNode(Material, HeatMaskIfNodePos);
        Expressions.Add(HeatMaskIfNode);
        

        // Mask-node
        FVector2D HeatMaskMaskNode1Pos(-6760, 1250);
        UMaterialExpressionComponentMask* HeatMaskMaskNode1 = MaterialUtils::CreateMaskNode(Material, HeatMaskMaskNode1Pos, true, false, false);
        Expressions.Add(HeatMaskMaskNode1);
        HeatMaskIfNode->A.Connect(0, HeatMaskMaskNode1);

        // Add-node
        FVector2D HeatMaskAddNodePos(-6950, 1250);
        UMaterialExpressionAdd* HeatMaskAddNode = MaterialUtils::CreateAddNode(Material, HeatMaskAddNodePos);
        Expressions.Add(HeatMaskAddNode);
        HeatMaskMaskNode1->Input.Connect(0, HeatMaskAddNode);

        // SceneTexture:SceneDepth-node
        FVector2D HeatMaskSceneTextureSceneDepthNodePos(-7250, 1250);
        UMaterialExpressionMaterialFunctionCall* HeatMaskSceneTextureSceneDepthNode = MaterialUtils::CreateSceneTextureSceneDepth(Material, HeatMaskSceneTextureSceneDepthNodePos);
        Expressions.Add(HeatMaskSceneTextureSceneDepthNode);
        HeatMaskSceneTextureSceneDepthNode->UpdateFromFunctionResource();
        HeatMaskAddNode->A.Connect(0, HeatMaskSceneTextureSceneDepthNode);


        // Mask-node
        FVector2D HeatMaskMaskNode2Pos(-6760, 1434);
        UMaterialExpressionComponentMask* HeatMaskMaskNode2 = MaterialUtils::CreateMaskNode(Material, HeatMaskMaskNode2Pos, true, false, false);
        Expressions.Add(HeatMaskMaskNode2);
        HeatMaskIfNode->B.Connect(0, HeatMaskMaskNode2);

        // Constant-node (Value 1)
        FVector2D HeatMaskConstantNode1Pos(-6760, 1550);
        UMaterialExpressionConstant* HeatMaskConstantNode1 = MaterialUtils::CreateConstantNode(Material, HeatMaskConstantNode1Pos, 1.0f);
        Expressions.Add(HeatMaskConstantNode1);
        HeatMaskIfNode->AGreaterThanB.Connect(0, HeatMaskConstantNode1);

        // Constant-node (Value 0)
        FVector2D HeatMaskConstantNode2Pos(-6760, 1650);
        UMaterialExpressionConstant* HeatMaskConstantNode2 = MaterialUtils::CreateConstantNode(Material, HeatMaskConstantNode2Pos, 0.0f);
        Expressions.Add(HeatMaskConstantNode2);
        HeatMaskIfNode->ALessThanB.Connect(0, HeatMaskConstantNode2);


        /** Orange 5.2 - Custom depth blur control **/

        // Orange comment box (5.2) - Custom depth blur control
        FVector2D OrangeBlurCommentPos(-8550, 1730);
        FVector2D OrangeBlurCommentSize(700, 550);
        FString OrangeBlurCommentText = TEXT("CustomDepth blur control");
        FColor OrangeBlurCommentColor =FColor::FromHex(TEXT("FFD26BFF"));
        UMaterialExpressionComment* OrangeBlurComment = MaterialUtils::CreateCommentNode(Material, OrangeBlurCommentPos, OrangeBlurCommentSize, OrangeBlurCommentText, OrangeBlurCommentColor);
        Expressions.Add(OrangeBlurComment);


        // Lerp-node
        FVector2D OrangeBlurLerpNodePos(-8000, 1903);
        UMaterialExpressionLinearInterpolate* OrangeBlurLerpNode = MaterialUtils::CreateLerpNode(Material, OrangeBlurLerpNodePos);
        Expressions.Add(OrangeBlurLerpNode);
        HeatMaskMaskNode2->Input.Connect(0, OrangeBlurLerpNode);
        

        // SceneTexture:CustomDepth-node
        FVector2D SceneTextureCustomDepthNodePos(-8400, 1830);
        UMaterialExpressionMaterialFunctionCall* SceneTextureCustomDepthNode = MaterialUtils::CreateSceneTextureCustomDepth(Material, SceneTextureCustomDepthNodePos);
        Expressions.Add(SceneTextureCustomDepthNode);
        SceneTextureCustomDepthNode->UpdateFromFunctionResource();
        OrangeBlurLerpNode->A.Connect(0, SceneTextureCustomDepthNode);


        // Clamp-node
        FVector2D OrangeBlurClampNodePos(-8200, 2070);
        UMaterialExpressionClamp* OrangeBlurClampNode = MaterialUtils::CreateClampNode(Material, OrangeBlurClampNodePos, 0.0f, 2.0f);
        Expressions.Add(OrangeBlurClampNode);
        OrangeBlurLerpNode->Alpha.Connect(0, OrangeBlurClampNode);

        UE_LOG(LogTemp, Warning, TEXT("Hello OrangeBlurClampNode:"));

        // ThermalSettingsBlur-node
        FVector2D OrangeBlurThermalSettingsBlurNodePos(-8450, 2070);
        UMaterialExpressionCollectionParameter* OrangeBlurThermalSettingsBlurNode = MaterialUtils::CreateThermalSettingsCPNode(Material, OrangeBlurThermalSettingsBlurNodePos, TEXT("Blur"), EThermalSettingsParamType::Scalar);
        Expressions.Add(OrangeBlurThermalSettingsBlurNode);
        OrangeBlurClampNode->Input.Connect(0, OrangeBlurThermalSettingsBlurNode);


        UE_LOG(LogTemp, Warning, TEXT("HEllo OrangeBlurThermalSettingsBlurNode:"));

        /**/


        /** Orange 5.3 - CustomDepth blur **/

        // Orange comment box (5.3) - CustomDepth blur
        FVector2D CustomDepthCommentPos(-11500, 834);
        FVector2D CustomDepthCommentSize(2900, 1500);
        FString CustomDepthCommentText = TEXT("CustomDepth blur");
        FColor CustomDepthCommentColor =FColor::FromHex(TEXT("FFD26BFF"));
        UMaterialExpressionComment* CustomDepthComment = MaterialUtils::CreateCommentNode(Material, CustomDepthCommentPos, CustomDepthCommentSize, CustomDepthCommentText, CustomDepthCommentColor);
        Expressions.Add(CustomDepthComment);

        

        // Add-node
        FVector2D CustomDepthAddNodeStartPos(-8800, 1921);
        UMaterialExpressionAdd* CustomDepthAddNodeStart = MaterialUtils::CreateAddNode(Material, CustomDepthAddNodeStartPos);
        Expressions.Add(CustomDepthAddNodeStart);
        
        OrangeBlurLerpNode->B.Connect(0, CustomDepthAddNodeStart);

        //* Groups of Add-nodes

        
        // Add-node 1
        FVector2D CustomDepthAddNode1Pos(-9100, 1794);
        UMaterialExpressionAdd* CustomDepthAddNode1 = MaterialUtils::CreateAddNode(Material, CustomDepthAddNode1Pos);
        Expressions.Add(CustomDepthAddNode1);

        CustomDepthAddNodeStart->A.Connect(0, CustomDepthAddNode1);

        // Add-node 2
        FVector2D CustomDepthAddNode2Pos(-9100, 1634);
        UMaterialExpressionAdd* CustomDepthAddNode2 = MaterialUtils::CreateAddNode(Material, CustomDepthAddNode2Pos);
        Expressions.Add(CustomDepthAddNode2);

        CustomDepthAddNode1->A.Connect(0, CustomDepthAddNode2);

        // Add-node 3
        FVector2D CustomDepthAddNode3Pos(-9100, 1474);
        UMaterialExpressionAdd* CustomDepthAddNode3 = MaterialUtils::CreateAddNode(Material, CustomDepthAddNode3Pos);
        Expressions.Add(CustomDepthAddNode3);

        CustomDepthAddNode2->A.Connect(0, CustomDepthAddNode3);

        // Add-node 4
        FVector2D CustomDepthAddNode4Pos(-9100, 1314);
        UMaterialExpressionAdd* CustomDepthAddNode4 = MaterialUtils::CreateAddNode(Material, CustomDepthAddNode4Pos);
        Expressions.Add(CustomDepthAddNode4);
        
        CustomDepthAddNode3->A.Connect(0, CustomDepthAddNode4);

        // Add-node 5
        FVector2D CustomDepthAddNode5Pos(-9100, 1154);
        UMaterialExpressionAdd* CustomDepthAddNode5 = MaterialUtils::CreateAddNode(Material, CustomDepthAddNode5Pos);
        Expressions.Add(CustomDepthAddNode5);

        CustomDepthAddNode4->A.Connect(0, CustomDepthAddNode5);

        UE_LOG(LogTemp, Warning, TEXT("Hei? 1"));

        //* Group of Multiply-nodes

        // Multiply-node 1 -1880
        FVector2D CustomDepthMultiplyNode1Pos(-9400, 2044);
        UMaterialExpressionMultiply* CustomDepthMultiplyNode1 = MaterialUtils::CreateMultiplyNode(Material, CustomDepthMultiplyNode1Pos, 0.1167f);
        Expressions.Add(CustomDepthMultiplyNode1);
        CustomDepthAddNodeStart->B.Connect(0, CustomDepthMultiplyNode1);

        // Multiply-node 2
        FVector2D CustomDepthMultiplyNode2Pos(-9400, 1874);
        UMaterialExpressionMultiply* CustomDepthMultiplyNode2 = MaterialUtils::CreateMultiplyNode(Material, CustomDepthMultiplyNode2Pos, 0.1167f);
        Expressions.Add(CustomDepthMultiplyNode2);
        CustomDepthAddNode1->B.Connect(0, CustomDepthMultiplyNode2);

        // Multiply-node 3
        FVector2D CustomDepthMultiplyNode3Pos(-9400, 1714);
        UMaterialExpressionMultiply* CustomDepthMultiplyNode3 = MaterialUtils::CreateMultiplyNode(Material, CustomDepthMultiplyNode3Pos, 0.1167f);
        Expressions.Add(CustomDepthMultiplyNode3);
        CustomDepthAddNode2->B.Connect(0, CustomDepthMultiplyNode3);

        // Multiply-node 4
        FVector2D CustomDepthMultiplyNode4Pos(-9400, 1554);
        UMaterialExpressionMultiply* CustomDepthMultiplyNode4 = MaterialUtils::CreateMultiplyNode(Material, CustomDepthMultiplyNode4Pos, 0.1167f);
        Expressions.Add(CustomDepthMultiplyNode4);
        CustomDepthAddNode3->B.Connect(0, CustomDepthMultiplyNode4);

        // Multiply-node 5
        FVector2D CustomDepthMultiplyNode5Pos(-9400, 1394);
        UMaterialExpressionMultiply* CustomDepthMultiplyNode5 = MaterialUtils::CreateMultiplyNode(Material, CustomDepthMultiplyNode5Pos, 0.1167f);
        Expressions.Add(CustomDepthMultiplyNode5);
        CustomDepthAddNode4->B.Connect(0, CustomDepthMultiplyNode5);

        // Multiply-node 6
        FVector2D CustomDepthMultiplyNode6Pos(-9400, 1234);
        UMaterialExpressionMultiply* CustomDepthMultiplyNode6 = MaterialUtils::CreateMultiplyNode(Material, CustomDepthMultiplyNode6Pos, 0.1167f);
        Expressions.Add(CustomDepthMultiplyNode6);
        CustomDepthAddNode5->B.Connect(0, CustomDepthMultiplyNode6);

        // Multiply-node 7
        FVector2D CustomDepthMultiplyNode7Pos(-9400, 1094);
        UMaterialExpressionMultiply* CustomDepthMultiplyNode7 = MaterialUtils::CreateMultiplyNode(Material, CustomDepthMultiplyNode7Pos, 0.3f);
        Expressions.Add(CustomDepthMultiplyNode7);
        CustomDepthAddNode5->A.Connect(0, CustomDepthMultiplyNode7);


        UE_LOG(LogTemp, Warning, TEXT("Hei? 2"));
        
        //* Group of SceneTexture:CustomDepth-nodes

        // SceneTexture:CustomDepth-node 1
        FVector2D SceneTextureCustomDepthNode1Pos(-9750, 2044);
        UMaterialExpressionMaterialFunctionCall* SceneTextureCustomDepthNode1 = MaterialUtils::CreateSceneTexturePostProcess(Material, SceneTextureCustomDepthNode1Pos);
        Expressions.Add(SceneTextureCustomDepthNode1);
        SceneTextureCustomDepthNode1->UpdateFromFunctionResource();
        CustomDepthMultiplyNode1->A.Connect(0, SceneTextureCustomDepthNode1);
        
        // SceneTexture:CustomDepth-node 2
        FVector2D SceneTextureCustomDepthNode2Pos(-9750, 1874);
        UMaterialExpressionMaterialFunctionCall* SceneTextureCustomDepthNode2 = MaterialUtils::CreateSceneTexturePostProcess(Material, SceneTextureCustomDepthNode2Pos);
        Expressions.Add(SceneTextureCustomDepthNode2);
        SceneTextureCustomDepthNode2->UpdateFromFunctionResource();
        CustomDepthMultiplyNode2->A.Connect(0, SceneTextureCustomDepthNode2);

        // SceneTexture:CustomDepth-node 3
        FVector2D SceneTextureCustomDepthNode3Pos(-9750, 1714);
        UMaterialExpressionMaterialFunctionCall* SceneTextureCustomDepthNode3 = MaterialUtils::CreateSceneTexturePostProcess(Material, SceneTextureCustomDepthNode3Pos);
        Expressions.Add(SceneTextureCustomDepthNode3);
        SceneTextureCustomDepthNode3->UpdateFromFunctionResource();
        CustomDepthMultiplyNode3->A.Connect(0, SceneTextureCustomDepthNode3);

        // SceneTexture:CustomDepth-node 4
        FVector2D SceneTextureCustomDepthNode4Pos(-9750, 1554);
        UMaterialExpressionMaterialFunctionCall* SceneTextureCustomDepthNode4 = MaterialUtils::CreateSceneTexturePostProcess(Material, SceneTextureCustomDepthNode4Pos);
        Expressions.Add(SceneTextureCustomDepthNode4);
        SceneTextureCustomDepthNode4->UpdateFromFunctionResource();
        CustomDepthMultiplyNode4->A.Connect(0, SceneTextureCustomDepthNode4);

        // SceneTexture:CustomDepth-node 5
        FVector2D SceneTextureCustomDepthNode5Pos(-9750, 1394);
        UMaterialExpressionMaterialFunctionCall* SceneTextureCustomDepthNode5 = MaterialUtils::CreateSceneTexturePostProcess(Material, SceneTextureCustomDepthNode5Pos);
        Expressions.Add(SceneTextureCustomDepthNode5);
        SceneTextureCustomDepthNode5->UpdateFromFunctionResource();
        CustomDepthMultiplyNode5->A.Connect(0, SceneTextureCustomDepthNode5);

        // SceneTexture:CustomDepth-node 6
        FVector2D SceneTextureCustomDepthNode6Pos(-9750, 1234);
        UMaterialExpressionMaterialFunctionCall* SceneTextureCustomDepthNode6 = MaterialUtils::CreateSceneTexturePostProcess(Material, SceneTextureCustomDepthNode6Pos);
        Expressions.Add(SceneTextureCustomDepthNode6);
        SceneTextureCustomDepthNode6->UpdateFromFunctionResource();
        CustomDepthMultiplyNode6->A.Connect(0, SceneTextureCustomDepthNode6);

        // SceneTexture:CustomDepth-node 7
        FVector2D SceneTextureCustomDepthNode7Pos(-9750, 1094);
        UMaterialExpressionMaterialFunctionCall* SceneTextureCustomDepthNode7 = MaterialUtils::CreateSceneTexturePostProcess(Material, SceneTextureCustomDepthNode7Pos);
        Expressions.Add(SceneTextureCustomDepthNode7);
        SceneTextureCustomDepthNode7->UpdateFromFunctionResource();
        CustomDepthMultiplyNode7->A.Connect(0, SceneTextureCustomDepthNode7);

        UE_LOG(LogTemp, Warning, TEXT("Hei? 3"));

        /*** Orange 5.3 White inside - UV Coordinates for color sample ***/
        
        // Orange 5.3 inside comment box - UV Coordinates for color sample
        FVector2D OrangeUVCoordCommentPos(-10550, 934);
        FVector2D OrangeUVCoordCCommentSize(700, 1350);
        FString OrangeUVCoordCCommentText = TEXT("UV Coordinates for color sample");
        UMaterialExpressionComment* OrangeUVCoordComment = MaterialUtils::CreateCommentNode(Material, OrangeUVCoordCommentPos, OrangeUVCoordCCommentSize, OrangeUVCoordCCommentText);
        Expressions.Add(OrangeUVCoordComment);


        //* Groups of Add-nodes
        
        // Add-node 1
        FVector2D OrangeUVCoordAddNode1Pos(-10020, 2074);
        UMaterialExpressionAdd* OrangeUVCoordAddNode1 = MaterialUtils::CreateAddNode(Material, OrangeUVCoordAddNode1Pos);
        Expressions.Add(OrangeUVCoordAddNode1);
        
        for (FFunctionExpressionInput& Input : SceneTextureCustomDepthNode1->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("UVs"))
            {
                Input.Input.Connect(0, OrangeUVCoordAddNode1);
            }
        }

        // Add-node 2 
        FVector2D OrangeUVCoordAddNode2Pos(-10020, 1914);
        UMaterialExpressionAdd* OrangeUVCoordAddNode2 = MaterialUtils::CreateAddNode(Material, OrangeUVCoordAddNode2Pos);
        Expressions.Add(OrangeUVCoordAddNode2);

        for (FFunctionExpressionInput& Input : SceneTextureCustomDepthNode2->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("UVs"))
            {
                Input.Input.Connect(0, OrangeUVCoordAddNode2);
            }
        }

        // Add-node 3 
        FVector2D OrangeUVCoordAddNode3Pos(-10020, 1754);
        UMaterialExpressionAdd* OrangeUVCoordAddNode3 = MaterialUtils::CreateAddNode(Material, OrangeUVCoordAddNode3Pos);
        Expressions.Add(OrangeUVCoordAddNode3);

        for (FFunctionExpressionInput& Input : SceneTextureCustomDepthNode3->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("UVs"))
            {
                Input.Input.Connect(0, OrangeUVCoordAddNode3);
            }
        }

        // Add-node 4 
        FVector2D OrangeUVCoordAddNode4Pos(-10020, 1594);
        UMaterialExpressionAdd* OrangeUVCoordAddNode4 = MaterialUtils::CreateAddNode(Material, OrangeUVCoordAddNode4Pos);
        Expressions.Add(OrangeUVCoordAddNode4);

        for (FFunctionExpressionInput& Input : SceneTextureCustomDepthNode4->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("UVs"))
            {
                Input.Input.Connect(0, OrangeUVCoordAddNode4);
            }
        }

        // Add-node 5 
        FVector2D OrangeUVCoordAddNode5Pos(-10020, 1434);
        UMaterialExpressionAdd* OrangeUVCoordAddNode5 = MaterialUtils::CreateAddNode(Material, OrangeUVCoordAddNode5Pos);
        Expressions.Add(OrangeUVCoordAddNode5);

        for (FFunctionExpressionInput& Input : SceneTextureCustomDepthNode5->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("UVs"))
            {
                Input.Input.Connect(0, OrangeUVCoordAddNode5);
            }
        }
        
        // Add-node 6 
        FVector2D OrangeUVCoordAddNode6Pos(-10020, 1274);
        UMaterialExpressionAdd* OrangeUVCoordAddNode6 = MaterialUtils::CreateAddNode(Material, OrangeUVCoordAddNode6Pos);
        Expressions.Add(OrangeUVCoordAddNode6);

        for (FFunctionExpressionInput& Input : SceneTextureCustomDepthNode6->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("UVs"))
            {
                Input.Input.Connect(0, OrangeUVCoordAddNode6);
            }
        }

        // Add-node 7 
        FVector2D OrangeUVCoordAddNode7Pos(-10020, 1114);
        UMaterialExpressionAdd* OrangeUVCoordAddNode7 = MaterialUtils::CreateAddNode(Material, OrangeUVCoordAddNode7Pos);
        Expressions.Add(OrangeUVCoordAddNode7);

        for (FFunctionExpressionInput& Input : SceneTextureCustomDepthNode7->FunctionInputs)
        {
            if (Input.Input.InputName == TEXT("UVs"))
            {
                Input.Input.Connect(0, OrangeUVCoordAddNode7);
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("Hei? 4"));

        // TextureCoordinate-node
        FVector2D OrangeUVCoordTextureCoordinateNodePos(-10300, 984);
        UMaterialExpressionTextureCoordinate* OrangeUVCoordTextureCoordinateNode = MaterialUtils::CreateTextureCoordinateNode(Material, OrangeUVCoordTextureCoordinateNodePos);
        Expressions.Add(OrangeUVCoordTextureCoordinateNode);

        OrangeUVCoordAddNode1->A.Connect(0, OrangeUVCoordTextureCoordinateNode);
        OrangeUVCoordAddNode2->A.Connect(0, OrangeUVCoordTextureCoordinateNode);
        OrangeUVCoordAddNode3->A.Connect(0, OrangeUVCoordTextureCoordinateNode);
        OrangeUVCoordAddNode4->A.Connect(0, OrangeUVCoordTextureCoordinateNode);
        OrangeUVCoordAddNode5->A.Connect(0, OrangeUVCoordTextureCoordinateNode);
        OrangeUVCoordAddNode6->A.Connect(0, OrangeUVCoordTextureCoordinateNode);
        OrangeUVCoordAddNode7->A.Connect(0, OrangeUVCoordTextureCoordinateNode);
        

        
        //* Group of Multiply-nodes

        // Multiply-node 1 -1880
        FVector2D OrangeUVCoordMultiplyNode1Pos(-10250, 2114);
        UMaterialExpressionMultiply* OrangeUVCoordMultiplyNode1 = MaterialUtils::CreateMultiplyNode(Material, OrangeUVCoordMultiplyNode1Pos);
        Expressions.Add(OrangeUVCoordMultiplyNode1);
        OrangeUVCoordAddNode1->B.Connect(0, OrangeUVCoordMultiplyNode1);

        // Multiply-node 2
        FVector2D OrangeUVCoordMultiplyNode2Pos(-10250, 1954);
        UMaterialExpressionMultiply* OrangeUVCoordMultiplyNode2 = MaterialUtils::CreateMultiplyNode(Material, OrangeUVCoordMultiplyNode2Pos);
        Expressions.Add(OrangeUVCoordMultiplyNode2);
        OrangeUVCoordAddNode2->B.Connect(0, OrangeUVCoordMultiplyNode2);

        // Multiply-node 3
        FVector2D OrangeUVCoordMultiplyNode3Pos(-10250, 1794);
        UMaterialExpressionMultiply* OrangeUVCoordMultiplyNode3 = MaterialUtils::CreateMultiplyNode(Material, OrangeUVCoordMultiplyNode3Pos);
        Expressions.Add(OrangeUVCoordMultiplyNode3);
        OrangeUVCoordAddNode3->B.Connect(0, OrangeUVCoordMultiplyNode3);

        // Multiply-node 4
        FVector2D OrangeUVCoordMultiplyNode4Pos(-10250, 1634);
        UMaterialExpressionMultiply* OrangeUVCoordMultiplyNode4 = MaterialUtils::CreateMultiplyNode(Material, OrangeUVCoordMultiplyNode4Pos);
        Expressions.Add(OrangeUVCoordMultiplyNode4);
        OrangeUVCoordAddNode4->B.Connect(0, OrangeUVCoordMultiplyNode4);
        
        // Multiply-node 5
        FVector2D OrangeUVCoordMultiplyNode5Pos(-10250, 1474);
        UMaterialExpressionMultiply* OrangeUVCoordMultiplyNode5 = MaterialUtils::CreateMultiplyNode(Material, OrangeUVCoordMultiplyNode5Pos);
        Expressions.Add(OrangeUVCoordMultiplyNode5);
        OrangeUVCoordAddNode5->B.Connect(0, OrangeUVCoordMultiplyNode5);
        
        // Multiply-node 6
        FVector2D OrangeUVCoordMultiplyNode6Pos(-10250, 1314);
        UMaterialExpressionMultiply* OrangeUVCoordMultiplyNode6 = MaterialUtils::CreateMultiplyNode(Material, OrangeUVCoordMultiplyNode6Pos);
        Expressions.Add(OrangeUVCoordMultiplyNode6);
        OrangeUVCoordAddNode6->B.Connect(0, OrangeUVCoordMultiplyNode6);
        
        // Multiply-node 7
        FVector2D OrangeUVCoordMultiplyNode7Pos(-10250, 1154);
        UMaterialExpressionMultiply* OrangeUVCoordMultiplyNode7 = MaterialUtils::CreateMultiplyNode(Material, OrangeUVCoordMultiplyNode7Pos);
        Expressions.Add(OrangeUVCoordMultiplyNode7);
        OrangeUVCoordAddNode7->B.Connect(0, OrangeUVCoordMultiplyNode7);


        //* Group of Constant2Vector -nodes
        
        // Constant 2Vector-node 1
        FVector2D OrangeUVCoordConstant2VectorNode1Pos(-10480, 2054);
        UMaterialExpressionConstant2Vector* OrangeUVCoordConstant2VectorNode1 = MaterialUtils::CreateConstant2VectorNode(Material, OrangeUVCoordConstant2VectorNode1Pos, -1.0f, -2.0f);
        Expressions.Add(OrangeUVCoordConstant2VectorNode1);
        OrangeUVCoordMultiplyNode1->A.Connect(2, OrangeUVCoordConstant2VectorNode1);

        // Constant 2Vector-node 2
        FVector2D OrangeUVCoordConstant2VectorNode2Pos(-10480, 1894);
        UMaterialExpressionConstant2Vector* OrangeUVCoordConstant2VectorNode2 = MaterialUtils::CreateConstant2VectorNode(Material, OrangeUVCoordConstant2VectorNode2Pos, -1.0f, 2.0f);
        Expressions.Add(OrangeUVCoordConstant2VectorNode2);
        OrangeUVCoordMultiplyNode2->A.Connect(2, OrangeUVCoordConstant2VectorNode2);
        
        // Constant 2Vector-node 3
        FVector2D OrangeUVCoordConstant2VectorNode3Pos(-10480, 1734);
        UMaterialExpressionConstant2Vector* OrangeUVCoordConstant2VectorNode3 = MaterialUtils::CreateConstant2VectorNode(Material, OrangeUVCoordConstant2VectorNode3Pos, 1.0f, -2.0f);
        Expressions.Add(OrangeUVCoordConstant2VectorNode3);
        OrangeUVCoordMultiplyNode3->A.Connect(2, OrangeUVCoordConstant2VectorNode3);
        
        // Constant 2Vector-node 4
        FVector2D OrangeUVCoordConstant2VectorNode4Pos(-10480, 1574);
        UMaterialExpressionConstant2Vector* OrangeUVCoordConstant2VectorNode4 = MaterialUtils::CreateConstant2VectorNode(Material, OrangeUVCoordConstant2VectorNode4Pos, 1.0f, 2.0f);
        Expressions.Add(OrangeUVCoordConstant2VectorNode4);
        OrangeUVCoordMultiplyNode4->A.Connect(2, OrangeUVCoordConstant2VectorNode4);
            
        // Constant 2Vector-node 5
        FVector2D OrangeUVCoordConstant2VectorNode5Pos(-10480, 1414);
        UMaterialExpressionConstant2Vector* OrangeUVCoordConstant2VectorNode5 = MaterialUtils::CreateConstant2VectorNode(Material, OrangeUVCoordConstant2VectorNode5Pos, -2.0f, 0.0f);
        Expressions.Add(OrangeUVCoordConstant2VectorNode5);
        OrangeUVCoordMultiplyNode5->A.Connect(2, OrangeUVCoordConstant2VectorNode5);
        
        // Constant 2Vector-node 6
        FVector2D OrangeUVCoordConstant2VectorNode6Pos(-10480, 1254);
        UMaterialExpressionConstant2Vector* OrangeUVCoordConstant2VectorNode6 = MaterialUtils::CreateConstant2VectorNode(Material, OrangeUVCoordConstant2VectorNode6Pos, 2.0f, 0.0f);
        Expressions.Add(OrangeUVCoordConstant2VectorNode6);
        OrangeUVCoordMultiplyNode6->A.Connect(2, OrangeUVCoordConstant2VectorNode6);
        
        // Constant 2Vector-node 7
        FVector2D OrangeUVCoordConstant2VectorNode7Pos(-10480, 1094);
        UMaterialExpressionConstant2Vector* OrangeUVCoordConstant2VectorNode7 = MaterialUtils::CreateConstant2VectorNode(Material, OrangeUVCoordConstant2VectorNode7Pos, 0.0f, 0.0f);
        Expressions.Add(OrangeUVCoordConstant2VectorNode7);
        OrangeUVCoordMultiplyNode7->A.Connect(2, OrangeUVCoordConstant2VectorNode7);

        /*** Orange 5.3 White inside - Get size of a pixel ***/

        UE_LOG(LogTemp, Warning, TEXT("Hei? 5"));
        
        // Orange 5.3 inside comment box - Get size of a pixel
        FVector2D OrangePixelSizeCommentPos(-11330, 1474);
        FVector2D OrangePixelSizeCommentSize(500, 300);
        FString OrangePixelSizeCommentText = TEXT("Get size of a pixel");
        UMaterialExpressionComment* OrangePixelSizeComment = MaterialUtils::CreateCommentNode(Material, OrangePixelSizeCommentPos, OrangePixelSizeCommentSize, OrangePixelSizeCommentText);
        Expressions.Add(OrangePixelSizeComment);

        UE_LOG(LogTemp, Warning, TEXT("Hei? 6"));

        // Divide-node
        FVector2D OrangePixelSizeDivideNodePos(-11030, 1594);
        UMaterialExpressionDivide* OrangePixelSizeDivideNode = MaterialUtils::CreateDivideNode(Material, OrangePixelSizeDivideNodePos);
        Expressions.Add(OrangePixelSizeDivideNode);

        OrangeUVCoordMultiplyNode1->B.Connect(0, OrangePixelSizeDivideNode);
        OrangeUVCoordMultiplyNode2->B.Connect(0, OrangePixelSizeDivideNode);
        OrangeUVCoordMultiplyNode3->B.Connect(0, OrangePixelSizeDivideNode);
        OrangeUVCoordMultiplyNode4->B.Connect(0, OrangePixelSizeDivideNode);
        OrangeUVCoordMultiplyNode5->B.Connect(0, OrangePixelSizeDivideNode);
        OrangeUVCoordMultiplyNode6->B.Connect(0, OrangePixelSizeDivideNode);
        OrangeUVCoordMultiplyNode7->B.Connect(0, OrangePixelSizeDivideNode);

        UE_LOG(LogTemp, Warning, TEXT("Hei? 7"));

        // Constant node
        FVector2D OrangePixelSizeConstantNodePos(-11230, 1554);
        UMaterialExpressionConstant* OrangePixelSizeConstantNode = MaterialUtils::CreateConstantNode(Material, OrangePixelSizeConstantNodePos, 1.0f);
        Expressions.Add(OrangePixelSizeConstantNode);
        OrangePixelSizeDivideNode->A.Connect(0, OrangePixelSizeConstantNode);

        UE_LOG(LogTemp, Warning, TEXT("Hei? 8"));

        // ScreenResolution node
        FVector2D OrangePixelSizeScreenResolutionNodePos(-11230, 1654);
        UMaterialExpressionMaterialFunctionCall* OrangePixelSizeScreenResolutionNode = MaterialUtils::CreateScreenResolution(Material, OrangePixelSizeScreenResolutionNodePos);
        Expressions.Add(OrangePixelSizeScreenResolutionNode);
        OrangePixelSizeScreenResolutionNode->UpdateFromFunctionResource();
        OrangePixelSizeDivideNode->B.Connect(0, OrangePixelSizeScreenResolutionNode);

        UE_LOG(LogTemp, Warning, TEXT("Hei? 9"));
        
        /**/

        

        return HeatMaskIfNode;
    }

    void CreateThermalCamera(bool& ppmCreated, FString& statusMessage)
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

        // === Before we start modifying the file... ===
        // Marks material as "about to be changed/modified"
        Material->PreEditChange(nullptr);
        Material->Modify(); 
        
        // The list of nodes in the MaterialFunction - We will be adding the nodes to this list
        TArray<TObjectPtr<UMaterialExpression>>& Expressions = Material->GetExpressionCollection().Expressions;


        // Area 1 - White area - Is Thermal Camera on?
        UMaterialExpressionLinearInterpolate* Area1WhiteLerpNode = CreateNodeArea1(Material, Expressions);

        // Area 2 - Yellow area - Add noise
        FNodeArea2Result Area2 = CreateNodeArea2(Material, Expressions, Area1WhiteLerpNode);
        
        // (Connects Area 1 to Area 2)
        Area1WhiteLerpNode->B.Connect(0, Area2.Area2YellowLerpNode);
        
        // Area 3 - White area - Add back the alpha channel to the image
        UMaterialExpressionAppendVector* Area3AppendNode = CreateNodeArea3(Material, Expressions);

        // (Connects Area 2 to Area 3)
        Area2.Area2YellowLerpNode->A.Connect(0, Area3AppendNode);
        Area2.Area2YellowAddNode->A.Connect(0, Area3AppendNode);

        /* ---- */

        // CombiningLerp-node
        FVector2D CombiningLerpNodePos(-4600, 210);
        UMaterialExpressionLinearInterpolate* CombiningLerpNode = MaterialUtils::CreateLerpNode(Material, CombiningLerpNodePos);
        Expressions.Add(CombiningLerpNode); 
        
        /* ---- */

        // (Connects Area 3 to CombiningLerpNode)
        Area3AppendNode->A.Connect(0, CombiningLerpNode);

        // Area 4 - Blue area
        FNodeArea4Result Area4 = CreateNodeArea4(Material, Expressions);

        // (Connect Area4 to CombiningLerpNode)
        CombiningLerpNode->A.Connect(0, Area4.Area4Background3ColorBlendNode);

        // Area 5 - Green area
        FNodeArea5Result Area5 = CreateNodeArea5(Material, Expressions);

        // (Connects Area5 to CombiningLerpNode)
        CombiningLerpNode->B.Connect(0, Area5.Area5ThermalActor3ColorBlendNode);

        // (Connects Area5's GreenBlurLerpNode node to Area4's AddSkypMultiply)
        Area4.Area4AddSkyMultiplyNode->B.Connect(0,Area5.Area5GreenBlurLerpNode);

        // Area 6 - Orange area
        UMaterialExpressionIf* Area6HeatMaskIfNode = CreateNodeArea6(Material, Expressions);
        
        // (Connects Area6 to CombiningLerpNode)
        CombiningLerpNode->Alpha.Connect(0, Area6HeatMaskIfNode);
        
        /**/
        
        /* Finish */

        
        // === After we are done modifying the file... ===
        // Mark the material as "done being changed/modified"
        Material->PostEditChange();
        // Mark as needing saving ("There's been changes in the package/file")
        Material->MarkPackageDirty();

        // Register the new asset (material) in the AssetRegistry - ensures it appears and is visible in the editor/content browser
        FAssetRegistryModule::AssetCreated(Material);

        /**/
        
        bool bSuccess = LogiUtils::SaveAssetToDisk(Material);

        if (bSuccess)
        {
            statusMessage = FString::Printf(TEXT("Material %s created and successfully saved to: %s"), *Material->GetName(), *FullAssetPath);
            ppmCreated = true;
        }
        else
        {
            statusMessage = FString::Printf(TEXT("Material %s created, but failed to save properly. Manual save required: %s"), *Material->GetName(), *FullAssetPath);
            ppmCreated = true;
        }
        
    }

}



