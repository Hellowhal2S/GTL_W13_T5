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
    virtual UObject* Duplicate(UObject* InOuter) override;    //UStaticMeshComponent* Target = nullptr;
    USkeletalMeshComponent* Target = nullptr;
    USphereTargetComponent* SphereComponent = nullptr;    virtual void Tick(float DeltaTime) override;
    bool bDead = false;
    float AccTime = 0.0f;

    // 펭귄 움직임 관련 변수들
    FVector InitialLocation;    // 초기 배치 위치
    float MovementRadius = 100.0f;    // 움직임 반경
    float MovementSpeed = 10.0f;    // 이동 속도
    FVector TargetLocation;    // 목표 위치
    bool bIsMovingToTarget = false;    // 목표 지점으로 이동 중인지
    float TimeSinceLastTargetChange = 0.0f;    // 마지막 목표 변경 후 시간
    float TargetChangeInterval = 3.0f;    // 목표 변경 간격 (초)

    bool bIsDangerNear = false;    // 위험(공) 감지 상태
    float DangerEscapeSpeed = 30.0f;    // 위험 회피 시 이동 속도
    float IdleAnimationTimer = 0.0f;    // Idle/Bray 애니메이션 지속 시간
    float IdleAnimationDuration = 0.0f;    // 현재 Idle 애니메이션 목표 지속 시간
    FString CurrentAnimationState = "Idle";    // 현재 애니메이션 상태
    AActor* DangerousActor = nullptr;    // 위험한 액터 참조

    // 펭귄 움직임 관련 함수들
    void HandlePenguinMovement(float DeltaTime);
    void SetNewTargetLocation();
    void SetAnimationState(const FString& StateName);
    
    void HandleDangerEscape(float DeltaTime);    // 위험 회피 행동
    void HandleIdleAnimation(float DeltaTime);    // Idle 애니메이션 관리
    void StartIdleAnimation();    // 새로운 Idle 애니메이션 시작
    void HandleDeathByBall(AActor* Ball);    // 공에 맞아 죽는 처리

private:
    bool RandomBool();
    float RandomRange(float Min, float Max);
    int32 RandomRange(int32 Min, int32 Max);
};
