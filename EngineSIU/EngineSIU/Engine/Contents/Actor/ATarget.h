#pragma once
#include "GameFramework/Actor.h"

class USphereTargetComponent;
class UStaticMeshComponent;

class ATarget : public AActor
{
    DECLARE_CLASS(ATarget, AActor)
public:
    ATarget();

    virtual void BeginPlay() override;

    
    UStaticMeshComponent* Target = nullptr;
    USphereTargetComponent* SphereComponent = nullptr;
    virtual void Tick(float DeltaTime) override;
    bool bDead = false;
    float AccTime = 0.0f;
};
