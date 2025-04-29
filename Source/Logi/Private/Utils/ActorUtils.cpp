#include "Utils/ActorUtils.h"

///
/// Utils related to Actor blueprints - adding, editing, manipluation, changes, etc.
/// 
namespace Logi::ActorUtils
{

	TArray<FName> FindAllMeshComponentsInBlueprint(UBlueprint* blueprint) {
	TArray<FName> meshComponentNames;
	//Iterate over all nodes in the blueprint
	for (USCS_Node* node : blueprint->SimpleConstructionScript->GetAllNodes()) {
		//Check if the node is a nullptr if so, skip
		if (!node) continue;
		//Check if the node has a component class
		UClass* ComponentClass = node->ComponentClass;
		//Check if the component class is a child of the skeletal mesh component or static mesh component
		if (ComponentClass->IsChildOf(USkeletalMeshComponent::StaticClass()) || ComponentClass->IsChildOf(UStaticMeshComponent::StaticClass())) {
			//Add the variable name to the mesh component names list
			meshComponentNames.Add(node->GetVariableName());
		}
	}

	return meshComponentNames;
}

	TArray<UMaterialInterface*> FindAllMaterialsFromActorScsNode(USCS_Node* scsNode) {
		TArray<UMaterialInterface*> materials;

		//Validate the scs node
		if (!scsNode) {
			UE_LOG(LogTemp, Error, TEXT("SCS node is null"));
			return materials;
		}

		//Get the mesh component from the SCS node
		UMeshComponent* meshComponent = Cast<UMeshComponent>(scsNode->ComponentTemplate);

		//Validate the mesh component
		if (!meshComponent) {
			UE_LOG(LogTemp, Error, TEXT("Mesh component is null, can't find materials"));
			return materials;
		}

		materials = meshComponent->GetMaterials();

		return materials;
	}

	TArray<USCS_Node*> FindUscsNodesForMeshComponentsFromABlueprint(UBlueprint* blueprint) {

		TArray<USCS_Node*> nodes;

		//validate blueprint
		if (!blueprint) {
			UE_LOG(LogTemp, Error, TEXT("Actor default object is null, can't find materials"));
			return nodes;
		}

		for (USCS_Node* node : blueprint->SimpleConstructionScript->GetAllNodes()) {
			if (!node) continue;

			UClass* componentClass = node->ComponentClass;

			if (componentClass->IsChildOf(USkeletalMeshComponent::StaticClass()) ||
				componentClass->IsChildOf(UStaticMeshComponent::StaticClass()))
			{
				nodes.Add(node);
			}
		}

		return nodes;

	}

	UMeshComponent* FindActorMeshComponentFromName(UBlueprint* blueprint, FName meshComponentName) {

		//Find the actor default object in the blueprint
		AActor* actorDefaultObject = Cast<AActor>(blueprint->GeneratedClass->GetDefaultObject());

		//Validate actor default object
		if (!actorDefaultObject) {
			UE_LOG(LogTemp, Error, TEXT("Actor default object is null, can't find materials"));
			return nullptr;
		}

		//Find the meshComponent in the blueprint by name
		for (UActorComponent* component : actorDefaultObject->GetComponents())
		{
			if (component && component->GetFName() == meshComponentName)
			{
				return Cast<UMeshComponent>(component);
			}
		}

		return nullptr;
	}

	auto CreateMaterialSwitchersForLogiMaterialsInActorBlueprint(UBlueprint* blueprint) {
		//validate blueprint
		if (!blueprint) {
			UE_LOG(LogTemp, Error, TEXT("Blueprint is null, can't replace materials"));
			return;
		}

		TArray<USCS_Node*> meshComponentUscsNodes = FindUscsNodesForMeshComponentsFromABlueprint(blueprint);

		//Validate meshComponentUscsNodes
		if (meshComponentUscsNodes.Num() == 0) {
			UE_LOG(LogTemp, Error, TEXT("No mesh component nodes found in the blueprint"));
			return;
		}


		//Replace materials in all actor mesh components with Logi materials
		for (USCS_Node* node : meshComponentUscsNodes) {
			TArray<UMaterialInterface*> materials = FindAllMaterialsFromActorScsNode(node);

			//Validate materials
			if (materials.Num() == 0) {
				UE_LOG(LogTemp, Error, TEXT("No materials found in the mesh component"));
				continue;
			}

			//Replace materials in the mesh component with Logi materials
			for (UMaterialInterface* material : materials) {

				if (!material->GetName().Contains(TEXT("Logi"), ESearchCase::IgnoreCase)) {

				}
				

			}


		}


	}

}