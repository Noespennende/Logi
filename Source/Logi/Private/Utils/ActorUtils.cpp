#include "Utils/ActorUtils.h"

///
/// Utils related to Actor blueprints - adding, editing, manipluation, changes, etc.
/// 
namespace Logi::ActorUtils
{

	TArray<FName> FindAllMeshComponentsInBlueprint(const UBlueprint* Blueprint) {
		TArray<FName> MeshComponentNames;

		//Iterate over all nodes in the blueprint
		for (const USCS_Node* Node : Blueprint->SimpleConstructionScript->GetAllNodes()) {
			//Check if the node is a nullptr if so, skip
			if (!Node) continue;

			//Check if the node has a component class
			const UClass* ComponentClass = Node->ComponentClass;

			//Check if the component class is a child of the skeletal mesh component or static mesh component
			if (ComponentClass->IsChildOf(USkeletalMeshComponent::StaticClass()) || ComponentClass->IsChildOf(UStaticMeshComponent::StaticClass())) {
				//Add the variable name to the mesh component names list
				MeshComponentNames.Add(Node->GetVariableName());
			}
		}

		return MeshComponentNames;
	}

	TArray<UMaterialInterface*> FindAllMaterialsFromActorScsNode(const USCS_Node* ScsNode) {
		TArray<UMaterialInterface*> Materials;

		//Validate the scs node
		if (!ScsNode) {
			UE_LOG(LogTemp, Error, TEXT("SCS node is null"));
			return Materials;
		}

		//Get the mesh component from the SCS node
		const UMeshComponent* MeshComponent = Cast<UMeshComponent>(ScsNode->ComponentTemplate);

		//Validate the mesh component
		if (!MeshComponent) {
			UE_LOG(LogTemp, Error, TEXT("Mesh component is null, can't find materials"));
			return Materials;
		}

		Materials = MeshComponent->GetMaterials();

		return Materials;
	}

	TArray<USCS_Node*> FindUscsNodesForMeshComponentsFromBlueprint(const UBlueprint* Blueprint) {

		TArray<USCS_Node*> Nodes;

		//validate blueprint
		if (!Blueprint) {
			UE_LOG(LogTemp, Error, TEXT("Actor default object is null, can't find materials"));
			return Nodes;
		}

		for (USCS_Node* Node : Blueprint->SimpleConstructionScript->GetAllNodes()) {
			if (!Node) continue;

			const UClass* ComponentClass = Node->ComponentClass;

			if (ComponentClass->IsChildOf(USkeletalMeshComponent::StaticClass()) ||
				ComponentClass->IsChildOf(UStaticMeshComponent::StaticClass()))
			{
				Nodes.Add(Node);
			}
		}

		return Nodes;

	}

	UMeshComponent* FindActorMeshComponentFromName(const UBlueprint* Blueprint, const FName& MeshComponentName) {

		//Find the actor default object in the blueprint
		const AActor* ActorDefaultObject = Cast<AActor>(Blueprint->GeneratedClass->GetDefaultObject());

		//Validate actor default object
		if (!ActorDefaultObject) {
			UE_LOG(LogTemp, Error, TEXT("Actor default object is null, can't find materials"));
			return nullptr;
		}

		//Find the meshComponent in the blueprint by name
		for (UActorComponent* Component : ActorDefaultObject->GetComponents())
		{
			if (Component && Component->GetFName() == MeshComponentName)
			{
				return Cast<UMeshComponent>(Component);
			}
		}

		return nullptr;
	}

}