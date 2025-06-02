#pragma once
#include "GameFramework/Actor.h"
#include "GameFramework/SpringArmComponent.h"

class USphereComponent;
class UStaticMeshComponent;
class USkeletalMeshComponent;

class ASnowBall : public AActor
{
    DECLARE_CLASS(ASnowBall, AActor)

public:
    ASnowBall();
    virtual void Tick(float DeltaTime) override;

    
    USceneComponent* DefaultSceneComponent = nullptr;
    UStaticMeshComponent* SnowBallComponent = nullptr;
    USphereComponent* SphereCollision = nullptr;
    USpringArmComponent* SpringArmComponent = nullptr;
    UCameraComponent* CameraComponent = nullptr;

    FVector SpawnLocation = FVector(100, 0, 70);
    bool bIsRespawned = true;
};
