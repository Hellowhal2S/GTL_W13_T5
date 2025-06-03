#pragma once
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"

class ADeerCanon : public AActor
{
    DECLARE_CLASS(ADeerCanon,AActor)
public:
    ADeerCanon();
    virtual ~ADeerCanon();

public:
    UStaticMeshComponent* Deer = nullptr;
    USceneComponent* SpawnPoint = nullptr;
    // UBoxComponent* Trigger = nullptr;
    virtual void Tick(float DeltaTime) override;

    float CoolTime = 5.0;
    float CurCoolTime = 0.0; 
};
