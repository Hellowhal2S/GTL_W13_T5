#pragma once
#include "Actors/Player.h"

class ASnowBall;

class AMyPlayer : public APlayer
{
    DECLARE_CLASS(AMyPlayer, APlayer)
public:
    AMyPlayer();

    virtual void Tick(float DeltaTime) override;

    void UpdateCameraRotation();

    POINT    PrevCursorPos = { 0, 0 };
    bool     CursorInit    = false;
    float    Yaw          = 180.0f;        // 누적 Yaw 값 (라디안 혹은 도 단위)
    float    Sensitivity   = 0.05f;      // 감도 (필요에 맞게 조절)
    UCameraComponent* Camera = nullptr;;
    USceneComponent* DefaultSceneComponent = nullptr;
    ASnowBall* Target;
};
