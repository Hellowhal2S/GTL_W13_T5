#include "ATarget.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FObjLoader.h"

ATarget::ATarget()
{
    Target = AddComponent<UStaticMeshComponent>(TEXT("Target"));
    Target->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Penguin/Penguin.obj"));
    Target->bSimulate = true;
    SetRootComponent(Target);
    
}

void ATarget::Tick(float DeltaTime)
{
    AActor::Tick(DeltaTime);
    
}
