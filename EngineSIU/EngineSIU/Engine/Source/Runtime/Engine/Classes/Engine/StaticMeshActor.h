#pragma once
#include "GameFramework/Actor.h"

class UStaticMeshComponent;


class AStaticMeshActor : public AActor
{
    DECLARE_CLASS(AStaticMeshActor, AActor)

public:
    AStaticMeshActor();

    UStaticMeshComponent* GetStaticMeshComponent() const;

protected:
    UPROPERTY
    (UStaticMeshComponent*, StaticMeshComponent, = nullptr);
};
 
