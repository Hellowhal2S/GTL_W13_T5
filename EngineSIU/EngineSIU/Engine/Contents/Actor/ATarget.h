#pragma once
#include "GameFramework/Actor.h"

class USphereTargetComponent;
class USkeletalMeshComponent;
class UStaticMeshComponent;

class ATarget : public AActor
{
    DECLARE_CLASS(ATarget, AActor)
public:
    ATarget();
    ~ATarget();
    virtual void BeginPlay() override;
    //virtual UObject* Duplicate(UObject* InOuter) override;

    //UStaticMeshComponent* Target = nullptr;
    USkeletalMeshComponent* Target = nullptr;
    USphereTargetComponent* SphereComponent = nullptr;
    virtual void Tick(float DeltaTime) override;
    bool bDead = false;
    float AccTime = 0.0f;
};
