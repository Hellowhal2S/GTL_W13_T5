#pragma once
#include "Classes/Engine/StaticMeshActor.h"

class AObstacleBase : public AStaticMeshActor
{
    DECLARE_CLASS(AObstacleBase, AStaticMeshActor)

public:
    AObstacleBase();
    virtual ~AObstacleBase() override = default;

    //virtual void PostSpawnInitialize() override;
    //virtual UObject* Duplicate(UObject* InOuter) override;
    //virtual void BeginPlay() override;
    //virtual void Tick(float DeltaTime) override;
    //virtual void Destroyed() override;
    //virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //virtual bool Destroy() override;
};

