#include "SnowBall.h"

#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FObjLoader.h"

ASnowBall::ASnowBall()
{
    
    SnowBallComponent = AddComponent<UStaticMeshComponent>("SnowBall");
    SnowBallComponent->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Reference/Reference.obj"));
    SetRootComponent(SnowBallComponent);
    
    SphereCollision = AddComponent<USphereComponent>("Collision");
    SphereCollision->SetupAttachment(SnowBallComponent);

    SpringArmComponent = AddComponent<USpringArmComponent>("SpringArm");
    SpringArmComponent->SetupAttachment(SnowBallComponent);
    CameraComponent = AddComponent<UCameraComponent>("Camera");
    CameraComponent->SetupAttachment(SpringArmComponent);
    CameraComponent->SetRelativeRotation(FRotator(-30, 0, 0));
}

void ASnowBall::Tick(float DeltaTime)
{
    APlayer::Tick(DeltaTime);
    //
    // SetActorLocation(SnowBallComponent->GetRelativeLocation());
    // SnowBallComponent->SetRelativeLocation(FVector(0, 0, 0));
}

