#pragma once
#include "Actors/Player.h"

class ASnowBall;

class AMyPlayer : public APlayer
{
    DECLARE_CLASS(AMyPlayer, APlayer)
public:
    AMyPlayer();

    virtual void Tick(float DeltaTime) override;
    UCameraComponent* Camera = nullptr;;
    USceneComponent* DefaultSceneComponent = nullptr;
    ASnowBall* Target;
};
