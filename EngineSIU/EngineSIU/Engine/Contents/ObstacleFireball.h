#pragma once
#include "ObstacleBase.h"
#include "Distribution/DistributionFloat.h"

class USphereTargetComponent;
class UParticleSystemComponent;
class UParticleSystem;

class AObstacleFireball : public AObstacleBase
{
    DECLARE_CLASS(AObstacleFireball, AObstacleBase);

public:
    AObstacleFireball();
    virtual ~AObstacleFireball() override;

    virtual void PostSpawnInitialize() override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    void InitializeParticle();
    void Fire(FVector InDirection);
    void CreateExplosion();

public:
    UParticleSystemComponent* ParticleSystemComponent;
    UParticleSystem* FireballParticleSystem;
    USphereTargetComponent* Collision = nullptr;

    FDelegateHandle OvelapHandler;
    FDelegateHandle OvelapEndHandler;
    
private:
    FDistributionFloat SpeedGenerator;
    float Speed;
    bool bIsFired = false;
    bool bIsCreatedExplosion = false;
    FVector Direction;

    float AccumulatedTime = 0.0f;
    float DestroyTime = 0.0f;
};

