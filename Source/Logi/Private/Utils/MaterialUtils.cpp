#include "Utils/MaterialUtils.h"

#include <optional>

#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionAppendVector.h"
#include "Materials/MaterialExpressionClamp.h"
#include "Materials/MaterialExpressionCollectionParameter.h"
#include "Materials/MaterialExpressionComment.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant2Vector.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
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
#include "Materials/MaterialExpressionFunctionOutput.h"

#include "Utils/EThermalSettingsParamType.h"


namespace Logi::MaterialUtils
{

    bool IsOuterAMaterialOrFunction(const UObject* Outer)
    {
        return Outer && (Outer->IsA<UMaterial>() || Outer->IsA<UMaterialFunctionInterface>());
    }

    // === General Utility Nodes ===

    UMaterialExpressionLinearInterpolate* CreateLerpNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateLerpNode"));
            return nullptr;
        }
        
        UMaterialExpressionLinearInterpolate* LerpNode = NewObject<UMaterialExpressionLinearInterpolate>(Outer);
        LerpNode->MaterialExpressionEditorX = EditorPos.X;
        LerpNode->MaterialExpressionEditorY = EditorPos.Y;
        
        return LerpNode;
    }

    UMaterialExpressionAdd* CreateAddNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateAddNode"));
            return nullptr;
        }
        
        UMaterialExpressionAdd* AddNode = NewObject<UMaterialExpressionAdd>(Outer);

        // Posititon in editor
        AddNode->MaterialExpressionEditorX = EditorPos.X;
        AddNode->MaterialExpressionEditorY = EditorPos.Y;
        
        return AddNode;
    }

    // BValue first of the optionals, because it is most likely to be the one that has a set value, if not both does.
    UMaterialExpressionMultiply* CreateMultiplyNode(UObject* Outer, const FVector2D& EditorPos, std::optional<float> BValue, std::optional<float> AValue)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateMultiplyNode"));
            return nullptr;
        }
        
        UMaterialExpressionMultiply* MultiplyNode = NewObject<UMaterialExpressionMultiply>(Outer);
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

    UMaterialExpressionOneMinus* CreateOneMinusNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateOneMinusNode"));
            return nullptr;
        }
        
        UMaterialExpressionOneMinus* OneMinusNode = NewObject<UMaterialExpressionOneMinus>(Outer);
        OneMinusNode->MaterialExpressionEditorX = EditorPos.X;
        OneMinusNode->MaterialExpressionEditorY = EditorPos.Y;

        return OneMinusNode;
    }

    UMaterialExpressionMax* CreateMaxNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateMaxNode"));
            return nullptr;
        }
        
        UMaterialExpressionMax* MaxNode = NewObject<UMaterialExpressionMax>(Outer);
        MaxNode->MaterialExpressionEditorX = EditorPos.X;
        MaxNode->MaterialExpressionEditorY = EditorPos.Y;

        return MaxNode;
    }
    
    UMaterialExpressionStep* CreateStepNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateStepNode"));
            return nullptr;
        }
        
        UMaterialExpressionStep* StepNode = NewObject<UMaterialExpressionStep>(Outer);
        StepNode->MaterialExpressionEditorX = EditorPos.X;
        StepNode->MaterialExpressionEditorY = EditorPos.Y;

        return StepNode;
    }

    UMaterialExpressionPower* CreatePowerNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreatePowerNode"));
            return nullptr;
        }
        
        UMaterialExpressionPower* PowerNode = NewObject<UMaterialExpressionPower>(Outer);
        PowerNode->MaterialExpressionEditorX = EditorPos.X;
        PowerNode->MaterialExpressionEditorY = EditorPos.Y;

        return PowerNode;
    }

    UMaterialExpressionDivide* CreateDivideNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateDivideNode"));
            return nullptr;
        }
        
        UMaterialExpressionDivide* DivideNode = NewObject<UMaterialExpressionDivide>(Outer);
        DivideNode->MaterialExpressionEditorX = EditorPos.X;
        DivideNode->MaterialExpressionEditorY = EditorPos.Y;

        return DivideNode;
    }
    
    UMaterialExpressionIf* CreateIfNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateLerpNode"));
            return nullptr;
        }
        
        UMaterialExpressionIf* IfNode = NewObject<UMaterialExpressionIf>(Outer);
        IfNode->MaterialExpressionEditorX = EditorPos.X;
        IfNode->MaterialExpressionEditorY = EditorPos.Y;

        return IfNode;
    }

    UMaterialExpressionClamp* CreateClampNode(UObject* Outer, const FVector2D& EditorPos, const std::optional<float> MinValue, const std::optional<float> MaxValue)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateClampNode"));
            return nullptr;
        }
        
        UMaterialExpressionClamp* ClampNode = NewObject<UMaterialExpressionClamp>(Outer);
        ClampNode->MaterialExpressionEditorX = EditorPos.X;
        ClampNode->MaterialExpressionEditorY = EditorPos.Y;

        // Set MinDefault and MaxDefault if values are provided
        if (MinValue.has_value())
        {
            ClampNode->MinDefault = MinValue.value();
        }

        if (MaxValue.has_value())
        {
            ClampNode->MaxDefault = MaxValue.value();
        }

        return ClampNode;
    }

    UMaterialExpressionFloor* CreateFloorNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateFloorNode"));
            return nullptr;
        }
        
        UMaterialExpressionFloor* FloorNode = NewObject<UMaterialExpressionFloor>(Outer);
        FloorNode->MaterialExpressionEditorX = EditorPos.X;
        FloorNode->MaterialExpressionEditorY = EditorPos.Y;

        return FloorNode;
    }

    UMaterialExpressionConstant* CreateConstantNode(UObject* Outer, const FVector2D& EditorPos, const float Value)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateConstantNode"));
            return nullptr;
        }
        
        UMaterialExpressionConstant* ConstantNode = NewObject<UMaterialExpressionConstant>(Outer);
        ConstantNode->MaterialExpressionEditorX = EditorPos.X;
        ConstantNode->MaterialExpressionEditorY = EditorPos.Y;
        ConstantNode->R = Value; 

        return ConstantNode;
    }

    UMaterialExpressionConstant2Vector* CreateConstant2VectorNode(UObject* Outer, const FVector2D& EditorPos, const float XValue, const float YValue)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateConstant2VectorNode"));
            return nullptr;
        }
        
        UMaterialExpressionConstant2Vector* Constant2VectorNode = NewObject<UMaterialExpressionConstant2Vector>(Outer);
        Constant2VectorNode->MaterialExpressionEditorX = EditorPos.X;
        Constant2VectorNode->MaterialExpressionEditorY = EditorPos.Y;
        Constant2VectorNode->R = XValue; 
        Constant2VectorNode->G = YValue;

        return Constant2VectorNode;
    }

    UMaterialExpressionConstant3Vector* CreateConstant3VectorNode(UObject* Outer, const FVector2D& EditorPos, const FLinearColor& Constant)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateConstant3VectorNode"));
            return nullptr;
        }
       
        UMaterialExpressionConstant3Vector* Constant3VectorNode = NewObject<UMaterialExpressionConstant3Vector>(Outer);
        Constant3VectorNode->MaterialExpressionEditorX = EditorPos.X;
        Constant3VectorNode->MaterialExpressionEditorY = EditorPos.Y;

        Constant3VectorNode->Constant = Constant; 

        return Constant3VectorNode;
    }

    UMaterialExpressionPixelNormalWS* CreatePixelNormalWSNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreatePixelNormalWSNode"));
            return nullptr;
        }

        UMaterialExpressionPixelNormalWS* PixelNormalWSNode = NewObject<UMaterialExpressionPixelNormalWS>(Outer);
        PixelNormalWSNode->MaterialExpressionEditorX = EditorPos.X;
        PixelNormalWSNode->MaterialExpressionEditorY = EditorPos.Y;

        return PixelNormalWSNode;
    }
    
    UMaterialExpressionTime* CreateTimeNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateTimeNode"));
            return nullptr;
        }

        UMaterialExpressionTime* TimeNode = NewObject<UMaterialExpressionTime>(Outer);
        TimeNode->MaterialExpressionEditorX = EditorPos.X;
        TimeNode->MaterialExpressionEditorY = EditorPos.Y;

        return TimeNode;
    }
    
    UMaterialExpressionTextureCoordinate* CreateTextureCoordinateNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateTextureCoordinateNode"));
            return nullptr;
        }
        
        UMaterialExpressionTextureCoordinate* TextureCoordinateNode = NewObject<UMaterialExpressionTextureCoordinate>(Outer);
        TextureCoordinateNode->MaterialExpressionEditorX = EditorPos.X;
        TextureCoordinateNode->MaterialExpressionEditorY = EditorPos.Y;

        return TextureCoordinateNode;
    }
    
    UMaterialExpressionVectorNoise* CreateVectorNoiseNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateVectorNoiseNode"));
            return nullptr;
        }

        UMaterialExpressionVectorNoise* VectorNoiseNode = NewObject<UMaterialExpressionVectorNoise>(Outer);
        VectorNoiseNode->MaterialExpressionEditorX = EditorPos.X;
        VectorNoiseNode->MaterialExpressionEditorY = EditorPos.Y;

        return VectorNoiseNode;
    }

    UMaterialExpressionAppendVector* CreateAppendVectorNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateAppendVectorNode"));
            return nullptr;
        }

        UMaterialExpressionAppendVector* AppendVectorNode = NewObject<UMaterialExpressionAppendVector>(Outer);
        AppendVectorNode->MaterialExpressionEditorX = EditorPos.X;
        AppendVectorNode->MaterialExpressionEditorY = EditorPos.Y;

        return AppendVectorNode;
    }

    UMaterialExpressionComponentMask* CreateMaskNode(UObject* Outer, const FVector2D& EditorPos, const bool R, const bool G, const bool B)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateMaskNode"));
            return nullptr;
        }

        UMaterialExpressionComponentMask* MaskNode = NewObject<UMaterialExpressionComponentMask>(Outer);
        MaskNode->MaterialExpressionEditorX = EditorPos.X;
        MaskNode->MaterialExpressionEditorY = EditorPos.Y;

        MaskNode->R = R; // Activate/Deactivate R-channel
        MaskNode->G = G; // Activate/Deactivate G-channel
        MaskNode->B = B; // Activate/Deactivate B-channel

        return MaskNode;
    }


    // === Material Function Nodes ===
    
    UMaterialExpressionMaterialFunctionCall* CreateScreenResolutionNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateScreenResolutionNode"));
            return nullptr;
        }
        
        // Opprett en MaterialExpressionMaterialFunctionCall-node
        UMaterialExpressionMaterialFunctionCall* MaterialFunctionNode = NewObject<UMaterialExpressionMaterialFunctionCall>(Outer);
        
        // ScreenResolution MaterialFunction
        const FString FilePath = TEXT("MaterialFunction'/Engine/Functions/Engine_MaterialFunctions02/ScreenResolution.ScreenResolution'");
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
            UE_LOG(LogTemp, Warning, TEXT("Failed to load 3ColorBlend MaterialFunction in CreateScreenResolutionNode"));
            return nullptr;
            
        }

        return MaterialFunctionNode;
    }

    UMaterialExpressionMaterialFunctionCall* CreateViewSizeNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateViewSizeNode"));
            return nullptr;
        }

        // 1) Load the MF asset with SceneTexturePP functionality inside
        FString FilePath = TEXT("/Logi/MF_Logi_ViewSize.MF_Logi_ViewSize");
        UMaterialFunction* MaterialFunction = LoadObject<UMaterialFunction>(nullptr, *FilePath);

        if (!MaterialFunction)
        {
            UE_LOG(LogTemp, Warning, TEXT("MaterialFunction failed to load in CreateViewSizeNode!"));
            return nullptr;
        }
        
        // 2) Create MaterialExpressionFunctionCall node, and assign the MF asset to it
        UMaterialExpressionMaterialFunctionCall* MFViewSizeNode = NewObject<UMaterialExpressionMaterialFunctionCall >(Outer);
        MFViewSizeNode->MaterialFunction = MaterialFunction;  // Set the Function to the MF-asset from 1)

        // Nodes posititon in editor
        MFViewSizeNode->MaterialExpressionEditorX = EditorPos.X;
        MFViewSizeNode->MaterialExpressionEditorY = EditorPos.Y;

        return MFViewSizeNode;
    }
    
    UMaterialExpressionMaterialFunctionCall* Create3ColorBlendNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to Create3ColorBlendNode"));
            return nullptr;
        }
        
        // Opprett en MaterialExpressionMaterialFunctionCall-node
        UMaterialExpressionMaterialFunctionCall* MaterialFunctionNode = NewObject<UMaterialExpressionMaterialFunctionCall>(Outer);
        
        // 3ColorBlend MaterialFunction
        const FString FilePath = TEXT("MaterialFunction'/Engine/Functions/Engine_MaterialFunctions01/ImageAdjustment/3ColorBlend.3ColorBlend'");
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
            UE_LOG(LogTemp, Warning, TEXT("Failed to load 3ColorBlend MaterialFunction in Create3ColorBlendNode"));
            return nullptr;
        }

        return MaterialFunctionNode;
    }

    UMaterialExpressionFunctionOutput* CreateOutputResultNode(UObject* Outer, const FVector2D& EditorPos, const FName& OutputName)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateOutputResultNode"));
            return nullptr;
        }

        UMaterialExpressionFunctionOutput* OutputResultNode = NewObject<UMaterialExpressionFunctionOutput>(Outer);

        OutputResultNode->MaterialExpressionEditorX = EditorPos.X;
        OutputResultNode->MaterialExpressionEditorY = EditorPos.Y;

        OutputResultNode->OutputName = OutputName;
        
        return OutputResultNode;
    }


    // === Parameter & Collection Nodes ===

    UMaterialExpressionScalarParameter* CreateScalarParameterNode(UObject* Outer, const FVector2D& EditorPos, const FName& ParameterName, const float DefaultValue)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateScalarParameterNode"));
            return nullptr;
        }
        
        UMaterialExpressionScalarParameter* ScalarParameterNode = NewObject<UMaterialExpressionScalarParameter>(Outer);
        ScalarParameterNode->MaterialExpressionEditorX = EditorPos.X;
        ScalarParameterNode->MaterialExpressionEditorY = EditorPos.Y;
        ScalarParameterNode->ParameterName = ParameterName;
        ScalarParameterNode->DefaultValue = DefaultValue;

        return ScalarParameterNode;
    }

    UMaterialExpressionCollectionParameter* CreateThermalSettingsCPNode(UObject* Outer, const FVector2D& EditorPos, const FName& ParameterName, const EThermalSettingsParamType ParamType)
    {

        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateThermalSettingsCPNode"));
            return nullptr;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Starting to create Collection Parameter Node"));

        /* Load MPC_ThermalSettings */
        const FString MPCPath = TEXT("/Game/Logi_ThermalCamera/Materials/MPC_Logi_ThermalSettings");
        UMaterialParameterCollection* MPC_ThermalSettings = LoadObject<UMaterialParameterCollection>(nullptr, *MPCPath);
        
        if (!MPC_ThermalSettings)
        {
            // If MPC_ThermalSettings was not loaded correctly, log the error and return nullptr (end)
            UE_LOG(LogTemp, Error, TEXT("Could not load Material Parameter Collection: %s"), *MPCPath);
            return nullptr; 
        }

        /* Create Collection Param node with MPC_ThermalSettings as the collection */
        UMaterialExpressionCollectionParameter* CollectionParamNode = NewObject<UMaterialExpressionCollectionParameter>(Outer);
        
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


    // === Other Nodes ===

    UMaterialExpressionComment* CreateCommentNode(UObject* Outer, const FVector2D& EditorPos, const FVector2D& Size, const FString& CommentText, const FLinearColor& BoxColor)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateCommentNode"));
            return nullptr;
        }
        
        UMaterialExpressionComment* Comment = NewObject<UMaterialExpressionComment>(Outer);

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

        return Comment;
    }
    
    UMaterialExpressionFresnel* CreateFresnelNode(UObject* Outer, const FVector2D& EditorPos, const std::optional<float> BaseReflectFractionValue, const std::optional<float> ExponentValue)
    {

        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateFresnelNode"));
            return nullptr;
        }
        
        UMaterialExpressionFresnel* FresnelNode = NewObject<UMaterialExpressionFresnel>(Outer);
        FresnelNode->MaterialExpressionEditorX = EditorPos.X;
        FresnelNode->MaterialExpressionEditorY = EditorPos.Y;

        // Set BaseReflectFraction and Exponent if values are provided
        if (BaseReflectFractionValue.has_value())
        {
            FresnelNode->BaseReflectFraction = BaseReflectFractionValue.value();
        }
        if (ExponentValue.has_value())
        {
            FresnelNode->Exponent = ExponentValue.value();
        }

        return FresnelNode;
    }

    UMaterialExpressionMakeMaterialAttributes* CreateMaterialAttributesNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateMaterialAttributesNode"));
            return nullptr;
        }

        UMaterialExpressionMakeMaterialAttributes* MaterialAttrNode = NewObject<UMaterialExpressionMakeMaterialAttributes>(Outer);

        MaterialAttrNode->MaterialExpressionEditorX = EditorPos.X;
        MaterialAttrNode->MaterialExpressionEditorY = EditorPos.Y;

        return MaterialAttrNode;
    }


    // === !!! 5.3.2 Workaround node creations - replace with proper node creation for UE version 5.4+ (See documentation) ===
    
    UMaterialExpressionMaterialFunctionCall* CreateSceneTexturePostProcessNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateSceneTexturePostProcessNode"));
            return nullptr;
        }

        // 1) Load the MF asset with SceneTexturePP functionality inside
        const FString FilePath = TEXT("/Logi/MF_Logi_SceneTexturePostProcess.MF_Logi_SceneTexturePostProcess");
        UMaterialFunction* MaterialFunction = LoadObject<UMaterialFunction>(nullptr, *FilePath);

        if (!MaterialFunction)
        {
            UE_LOG(LogTemp, Warning, TEXT("MaterialFunction failed to load in CreateSceneTexturePostProcessNode!"));
            return nullptr;
        }
        
        // 2) Create MaterialExpressionFunctionCall node, and assign the MF asset to it
        UMaterialExpressionMaterialFunctionCall* MFSceneTexturePostProcessNode = NewObject<UMaterialExpressionMaterialFunctionCall >(Outer);
        MFSceneTexturePostProcessNode->MaterialFunction = MaterialFunction;  // Set the Function to the MF-asset from 1)

        // Nodes posititon in editor
        MFSceneTexturePostProcessNode->MaterialExpressionEditorX = EditorPos.X;
        MFSceneTexturePostProcessNode->MaterialExpressionEditorY = EditorPos.Y;

        return MFSceneTexturePostProcessNode;
    }
    
    UMaterialExpressionMaterialFunctionCall* CreateSceneTextureBaseColorNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateSceneTextureBaseColor"));
            return nullptr;
        }

        // 1) Load the MF asset with SceneTexturePP functionality inside
        const FString FilePath = TEXT("/Logi/MF_Logi_SceneTextureBaseColor.MF_Logi_SceneTextureBaseColor");
        UMaterialFunction* MaterialFunction = LoadObject<UMaterialFunction>(nullptr, *FilePath);

        if (!MaterialFunction)
        {
            UE_LOG(LogTemp, Warning, TEXT("MaterialFunction failed to load in CreateSceneTextureBaseColorNode!"));
            return nullptr;
        }
        
        // 2) Create MaterialExpressionFunctionCall node, and assign the MF asset to it
        UMaterialExpressionMaterialFunctionCall* MFSceneTextureBaseColorNode = NewObject<UMaterialExpressionMaterialFunctionCall >(Outer);
        MFSceneTextureBaseColorNode->MaterialFunction = MaterialFunction;  // Set the Function to the MF-asset from 1)

        // Nodes posititon in editor
        MFSceneTextureBaseColorNode->MaterialExpressionEditorX = EditorPos.X;
        MFSceneTextureBaseColorNode->MaterialExpressionEditorY = EditorPos.Y;

        return MFSceneTextureBaseColorNode;
    }

    UMaterialExpressionMaterialFunctionCall* CreateSceneTextureWorldNormalNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateSceneTextureWorldNormalNode"));
            return nullptr;
        }

        // 1) Load the MF asset with SceneTexturePP functionality inside
        const FString FilePath = TEXT("/Logi/MF_Logi_SceneTextureWorldNormal.MF_Logi_SceneTextureWorldNormal");
        UMaterialFunction* MaterialFunction = LoadObject<UMaterialFunction>(nullptr, *FilePath);

        if (!MaterialFunction)
        {
            UE_LOG(LogTemp, Warning, TEXT("MaterialFunction failed to load in CreateSceneTextureWorldNormalNode!"));
            return nullptr;
        }

        // 2) Create MaterialExpressionFunctionCall node, and assign the MF asset to it
        UMaterialExpressionMaterialFunctionCall* MFSceneTextureWorldNormalNode = NewObject<UMaterialExpressionMaterialFunctionCall>(Outer);
        MFSceneTextureWorldNormalNode->MaterialFunction = MaterialFunction;  // Set the Function to the MF-asset from 1)

        // Nodes posititon in editor
        MFSceneTextureWorldNormalNode->MaterialExpressionEditorX = EditorPos.X;
        MFSceneTextureWorldNormalNode->MaterialExpressionEditorY = EditorPos.Y;

        return MFSceneTextureWorldNormalNode;
    }
    
    UMaterialExpressionMaterialFunctionCall* CreateSceneTextureSceneDepthNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateSceneTextureSceneDepthNode"));
            return nullptr;
        }

        // 1) Load the MF asset with SceneTexturePP functionality inside
        const FString FilePath = TEXT("/Logi/MF_Logi_SceneTextureSceneDepth.MF_Logi_SceneTextureSceneDepth");
        UMaterialFunction* MaterialFunction = LoadObject<UMaterialFunction>(nullptr, *FilePath);

        if (!MaterialFunction)
        {
            UE_LOG(LogTemp, Warning, TEXT("MaterialFunction failed to load in CreateSceneTextureSceneDepthNode!"));
            return nullptr;
        }
        
        // 2) Create MaterialExpressionFunctionCall node, and assign the MF asset to it
        UMaterialExpressionMaterialFunctionCall* MFSceneTextureSceneDepthNode = NewObject<UMaterialExpressionMaterialFunctionCall >(Outer);
        MFSceneTextureSceneDepthNode->MaterialFunction = MaterialFunction;  // Set the Function to the MF-asset from 1)

        // Nodes posititon in editor
        MFSceneTextureSceneDepthNode->MaterialExpressionEditorX = EditorPos.X;
        MFSceneTextureSceneDepthNode->MaterialExpressionEditorY = EditorPos.Y;

        return MFSceneTextureSceneDepthNode;
    }
    
    UMaterialExpressionMaterialFunctionCall* CreateSceneTextureCustomDepthNode(UObject* Outer, const FVector2D& EditorPos)
    {
        // If Outer is not a UMaterial or UMaterialFunctionInterface(UMaterialFunction + others)
        if (!IsOuterAMaterialOrFunction(Outer))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid Outer passed to CreateSceneTextureCustomDepthNode"));
            return nullptr;
        }

        // 1) Load the MF asset with SceneTexturePP functionality inside
        const FString FilePath = TEXT("/Logi/MF_Logi_SceneTextureCustomDepth.MF_Logi_SceneTextureCustomDepth");
        UMaterialFunction* MaterialFunction = LoadObject<UMaterialFunction>(nullptr, *FilePath);

        if (!MaterialFunction)
        {
            UE_LOG(LogTemp, Warning, TEXT("MaterialFunction failed to load in CreateSceneTextureCustomDepthNode!"));
            return nullptr;
        }
        
        // 2) Create MaterialExpressionFunctionCall node, and assign the MF asset to it
        UMaterialExpressionMaterialFunctionCall* MFSceneTextureCustomDepthNode = NewObject<UMaterialExpressionMaterialFunctionCall >(Outer);
        MFSceneTextureCustomDepthNode->MaterialFunction = MaterialFunction;  // Set the Function to the MF-asset from 1)

        // Nodes posititon in editor
        MFSceneTextureCustomDepthNode->MaterialExpressionEditorX = EditorPos.X;
        MFSceneTextureCustomDepthNode->MaterialExpressionEditorY = EditorPos.Y;

        return MFSceneTextureCustomDepthNode;
    }
  
    
}