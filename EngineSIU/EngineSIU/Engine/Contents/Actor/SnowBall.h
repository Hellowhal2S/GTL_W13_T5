#pragma once
#include "GameFramework/Actor.h"
#include "GameFramework/SpringArmComponent.h"

class USphereTargetComponent;
class USphereComponent;
class UStaticMeshComponent;
class USkeletalMeshComponent;

class ASnowBall : public AActor
{
    DECLARE_CLASS(ASnowBall, AActor)

public:
    ASnowBall();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    
    UStaticMeshComponent* SnowBallComponent = nullptr;
    USphereTargetComponent* SphereCollision = nullptr;
    
    FVector SpawnLocation = FVector(100, 0, 70);
    FVector InitialScale = FVector(5.f);
    float InitialRadius = 2.f;
    float InitialMass = 10.f;
    bool bIsRespawned = true;
};
