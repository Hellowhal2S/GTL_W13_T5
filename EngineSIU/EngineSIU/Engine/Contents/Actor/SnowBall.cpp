#include "SnowBall.h"

#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FObjLoader.h"

ASnowBall::ASnowBall()
{
    // DefaultSceneComponent = AddComponent<USceneComponent>("DefaultSceneComponent");
    // SetRootComponent(DefaultSceneComponent);
    
    SnowBallComponent = AddComponent<UStaticMeshComponent>("SnowBall");
    SnowBallComponent->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Apple/apple_mid.obj"));
    SetRootComponent(SnowBallComponent);
    
}

void ASnowBall::Tick(float DeltaTime)
{
    AActor::Tick(DeltaTime);
    SnowBallComponent = GetComponentByClass<UStaticMeshComponent>();

    if (GetActorLocation().Z < 10.0)
        Destroy();
}

