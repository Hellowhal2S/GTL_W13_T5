#pragma once
#include "Actors/Player.h"

class ASnowBall;
class UParticleSystemComponent;

class AMyPlayer : public APlayer
{
    DECLARE_CLASS(AMyPlayer, APlayer)
public:
    AMyPlayer();
    virtual void RegisterLuaType(sol::state& Lua) override;
    virtual void BeginPlay() override;

    virtual void Tick(float DeltaTime) override;
    static FVector InitialVector;
    static FRotator InitialRotator;
    void UpdateCameraRotation();
    void CameraInitialize();
    POINT    PrevCursorPos = { 0, 0 };
    bool     CursorInit    = false;
    bool     bSkipFirstFrame = false;  // 마우스 위치 초기화 후 첫 프레임 스킵용
    float    Yaw          = 180.0f;        // 누적 Yaw 값 (라디안 혹은 도 단위)
    float    Pitch        = 0.0f;        // 누적 Pitch 값 (상하 회전)
    float    Sensitivity   = 0.05f;      // 감도 (필요에 맞게 조절)
    
    // 마우스 모드 관련 변수들
    bool     bCameraMode   = true;        // true: 카메라 회전 모드, false: 마우스 커서 모드
    bool     bPrevAltState = false;       // 이전 Alt 키 상태 (키 눌림 감지용)
    
    UCameraComponent* Camera = nullptr;;
    USceneComponent* DefaultSceneComponent = nullptr;
    UParticleSystemComponent* ParticleSystemComp = nullptr;
    ASnowBall* Target;
    bool FirstCT =false;
    bool SecondCT =false;
    bool bInitiatlize =false;
    float AccTime = 0.0f;
    int32 Counter = 5;
    bool EndGame = false;
};
