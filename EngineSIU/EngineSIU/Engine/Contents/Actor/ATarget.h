#pragma once
#include "GameFramework/Actor.h"

class UStaticMeshComponent;

class ATarget : public AActor
{
    DECLARE_CLASS(ATarget, AActor)
public:
    ATarget();

    UStaticMeshComponent* Target = nullptr;
    
    virtual void Tick(float DeltaTime) override;
};
