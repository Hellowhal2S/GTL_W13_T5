#include "ObstacleWall.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/FObjLoader.h"


AObstacleWall::AObstacleWall()
{
    StaticMeshComponent->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Primitives/Cube.obj"));
    float height = 4;
    StaticMeshComponent->SetRelativeScale3D(FVector(2.0, 7.0f, height));
    StaticMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, height / 2.0f));
    StaticMeshComponent->bSimulate = true;
    StaticMeshComponent->bIsKinematic = true;

    auto* CollisionComp = AddComponent<UBoxComponent>("BoxCollision");
    CollisionComp->AttachToComponent(RootComponent);
    CollisionComp->SetBoxExtent(FVector::OneVector * 0.5f);
}
