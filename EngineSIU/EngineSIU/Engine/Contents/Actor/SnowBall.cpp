#include "SnowBall.h"

#include "PhysicsManager.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Engine/EditorEngine.h"
#include "GameFramework/GameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FObjLoader.h"

ASnowBall::ASnowBall()
{
    // DefaultSceneComponent = AddComponent<USceneComponent>("DefaultSceneComponent");
    // SetRootComponent(DefaultSceneComponent);
    
    SnowBallComponent = AddComponent<UStaticMeshComponent>("SnowBall");
    SnowBallComponent->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/FreshSnow/SnowBall.obj"));
    SetRootComponent(SnowBallComponent);
    
}

void ASnowBall::Tick(float DeltaTime)
{
    AActor::Tick(DeltaTime);
    SnowBallComponent = GetComponentByClass<UStaticMeshComponent>();
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);

    if (!Engine->PIEWorld->GetGameMode()->bGameOver && GetActorLocation().Z < 0.f)
    {
        if (Engine)
        {
            Engine->PIEWorld->GetGameMode()->Life--;
            if (Engine->PIEWorld->GetGameMode()->Life > 0)
            {
                Engine->PhysicsManager->DestroyGameObject(SnowBallComponent->BodyInstance->BIGameObject);
                SetActorLocation(SpawnLocation);
                SnowBallComponent->CreatePhysXGameObject();
            }
        }
    }
}

