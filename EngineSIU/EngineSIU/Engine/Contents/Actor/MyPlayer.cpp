#include "MyPlayer.h"

#include "SnowBall.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "World/World.h"

AMyPlayer::AMyPlayer()
{
    DefaultSceneComponent = AddComponent<USceneComponent>(TEXT("DefaultSceneComponent"));
    SetRootComponent(DefaultSceneComponent);
    Camera = AddComponent<UCameraComponent>("Camera");
    Camera->SetupAttachment(DefaultSceneComponent);
}

void AMyPlayer::Tick(float DeltaTime)
{
    APlayer::Tick(DeltaTime);
    for (auto iter : GetWorld()->GetActiveLevel()->Actors)
    {
        if (Cast<ASnowBall>(iter))
        {
            Target = Cast<ASnowBall>(iter);
        }
    }

    SetActorLocation(Target->SnowBallComponent->GetRelativeLocation());

}
