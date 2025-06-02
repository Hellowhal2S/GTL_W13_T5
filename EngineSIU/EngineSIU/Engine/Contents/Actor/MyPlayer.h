#pragma once
#include "Actors/Player.h"

class ASnowBall;

class AMyPlayer : public APlayer
{
    DECLARE_CLASS(AMyPlayer, APlayer)
public:
    AMyPlayer();
    
    virtual void BeginPlay() override;

    virtual void Tick(float DeltaTime) override;

    void UpdateCameraRotation();

    POINT    PrevCursorPos = { 0, 0 };
    bool     CursorInit    = false;
    float    Yaw          = 180.0f;        // 누적 Yaw 값 (라디안 혹은 도 단위)
    float    Sensitivity   = 0.05f;      // 감도 (필요에 맞게 조절)
    
    // 마우스 모드 관련 변수들
    bool     bCameraMode   = true;        // true: 카메라 회전 모드, false: 마우스 커서 모드
    bool     bPrevAltState = false;       // 이전 Alt 키 상태 (키 눌림 감지용)
    
    UCameraComponent* Camera = nullptr;;
    USceneComponent* DefaultSceneComponent = nullptr;
    ASnowBall* Target;
    bool FirstCT =false;
    bool SecondCT =false;

    float AccTime = 0.0f;
};
