#include "ATarget.h"

#include "PhysicsManager.h"
#include "SnowBall.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/FObjLoader.h"
#include "SphereTargetComponent.h"
#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "GameFramework/GameMode.h"
#include "World/World.h"
#include "Engine/AssetManager.h"
#include "Engine/Contents/AnimInstance/LuaScriptAnimInstance.h"
#include "Math/MathUtility.h"
#include <random>


ATarget::ATarget()
{
    /*Target = AddComponent<UStaticMeshComponent>(TEXT("Target"));
    Target->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Penguin/Penguin.obj"));
    Target->bSimulate = true;
    SetRootComponent(Target);*/
    Target = AddComponent<USkeletalMeshComponent>(TEXT("Target"));
    Target->SetSkeletalMeshAsset(UAssetManager::Get().GetSkeletalMesh("Contents/SkeletalPenguin/SkeletalPenguin"));
    Target->bSimulate = false;
    Target->SetAnimClass(ULuaScriptAnimInstance::StaticClass());
    Target->StateMachineFileName = TEXT("LuaScripts/Animations/NPCStateMachine.lua");
    SetRootComponent(Target);

    SphereComponent = AddComponent<USphereTargetComponent>(TEXT("Collision"));
    SphereComponent->SetupAttachment(Target);
}

ATarget::~ATarget()
{
}

UObject* ATarget::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewActor->Target = GetComponentByClass<USkeletalMeshComponent>();

    /*AddComponent<USkeletalMeshComponent>(TEXT("Target"));
    NewActor->Target->SetSkeletalMeshAsset(UAssetManager::Get().GetSkeletalMesh("Contents/SkeletalPenguin/SkeletalPenguin"));
    NewActor->Target->bSimulate = false;
    NewActor->Target->SetAnimClass(ULuaScriptAnimInstance::StaticClass());
    NewActor->Target->StateMachineFileName = TEXT("LuaScripts/Animations/NPCStateMachine.lua");
    NewActor->SetRootComponent(NewActor->Target);*/

    NewActor->SphereComponent = GetComponentByClass<USphereTargetComponent>();

    /*AddComponent<USphereTargetComponent>(TEXT("Collision"));
    NewActor->SphereComponent->SetupAttachment(Target);*/

    return NewActor;
}

void ATarget::BeginPlay()
{
    AActor::BeginPlay();

    // 초기 위치 저장 (펭귄 움직임의 중심점)
    InitialLocation = GetActorLocation();

    // AnimInstance 초기화 확인 및 설정
    ULuaScriptAnimInstance* AnimInstance = Cast<ULuaScriptAnimInstance>(GetComponentByClass<USkeletalMeshComponent>()->GetAnimInstance());
    if (AnimInstance)
    {
        //UE_LOGFMT(ELogLevel::Display, "ATarget::BeginPlay - AnimInstance found: {}", AnimInstance->GetClass()->GetName());


    }

    // 초기 Idle 애니메이션 상태 설정
    StartIdleAnimation();

    //// StateMachine 파일 경로 명시적으로 지정 (필요시)
    //Target->StateMachineFileName = TEXT("LuaScripts/Animations/NPCStateMachine.lua");
    //Target->ClearAnimScriptInstance();
    //Target->InitAnim();

    //ULuaScriptAnimInstance* AnimInstance = Cast<ULuaScriptAnimInstance>(Target->GetAnimInstance());
    //if (AnimInstance)
    //{
    //    AnimInstance->InitializeAnimation();
    //}

    // Trigger 이벤트에 StateMachine 연동

    GetComponentByClass<USphereTargetComponent>()->OnComponentBeginOverlap.AddLambda(
        [this](UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OhterComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
        {
            ASnowBall* Ball = Cast<ASnowBall>(OtherActor);
            if (Ball != nullptr)
            {
                // 공과의 거리 계산
                float DistanceToActor = FVector::Dist(GetActorLocation(), OtherActor->GetActorLocation());

                UE_LOGFMT(ELogLevel::Display, "Penguin detected ball at distance: {}", DistanceToActor);


                UE_LOG(ELogLevel::Warning, TEXT("Penguin hit by ball - Death!"));

                HandleDeathByBall(Ball);
            }
        }
    );    
    
    //GetComponentByClass<USphereTargetComponent>()->OnComponentEndOverlap.AddLambda(
    //    [this](UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OhterComponent, int32 OtherBodyIndex)
    //    {
    //        ASnowBall* Ball = Cast<ASnowBall>(OtherActor);
    //        if (Ball != nullptr && DangerousActor == OtherActor)
    //        {
    //            UE_LOG(ELogLevel::Display, TEXT("Penguin - Danger passed, returning to normal behavior"));

    //            // 정상 행동으로 복귀 - Idle 애니메이션 시작
    //            StartIdleAnimation();
    //        }
    //    }
    //);

    // Hit 반경 설정 
    GetComponentByClass<USphereTargetComponent>()->SetRadius(10.f);

}

void ATarget::Tick(float DeltaTime)
{
    AActor::Tick(DeltaTime);
    // 사망 처리
    if (bDead)
    {
        AccTime += DeltaTime;
        if (AccTime > 2.0f)
        {
            UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine);
            //GetComponentByClass<USkeletalMeshComponent>()->bSimulate = false; // TODO: Ragdoll 완성되면 해제 
            SetActorLocation(FVector(-200, -200, -200));
            if (GEngine->ActiveWorld->GetGameMode())
                GEngine->ActiveWorld->GetGameMode()->Score += 100;

            bDead = false;

        }
    }

    // 넉백 처리
    if (bIsKnockback)
    {
        KnockbackElapsed += DeltaTime;
        float Alpha = FMath::Clamp(KnockbackElapsed / KnockbackDuration, 0.0f, 1.0f);
        FVector NewLocation = FMath::Lerp(KnockbackStartLocation, KnockbackTargetLocation, Alpha);
        SetActorLocation(NewLocation);

        if (Alpha >= 1.0f)
        {
            bIsKnockback = false;
        }
        return; // 넉백 중에는 다른 동작 생략
    }

    // 펭귄 행동 처리 (죽지 않은 상태에서만)
    if (!bDead)
    {
            // 정상적인 움직임과 Idle 애니메이션 처리
            if (bIsMovingToTarget)
            {
                HandlePenguinMovement(DeltaTime);
            }
            else
            {
                HandleIdleAnimation(DeltaTime);
            }
    }


}

void ATarget::HandlePenguinMovement(float DeltaTime)
{
    // 시간 업데이트
    TimeSinceLastTargetChange += DeltaTime;

    // 목표 위치가 설정되어 있다면 그쪽으로 이동
    if (bIsMovingToTarget)
    {
        FVector CurrentLocation = GetActorLocation();
        FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
        float DistanceToTarget = FVector::Dist(CurrentLocation, TargetLocation);

        // 목표에 가까우면 도착으로 처리
        if (DistanceToTarget < 10.0f)
        {
            bIsMovingToTarget = false;
            TimeSinceLastTargetChange = 0.0f;

            // Idle 상태로 변경
            StartIdleAnimation();
        }
        else
        {
            // 목표 방향으로 이동
            FVector NewLocation = CurrentLocation + (Direction * MovementSpeed * DeltaTime);

            // 초기 위치에서 반경을 벗어나지 않도록 제한
            if (FVector::Dist(NewLocation, InitialLocation) <= MovementRadius)
            {
                SetActorLocation(NewLocation);

                // 이동 중이므로 Walk 애니메이션 상태 유지
                if (CurrentAnimationState != "Walk")
                {
                    SetAnimationState("Walk");
                }

                // 이동 방향으로 회전
                if (!Direction.IsZero())
                {
                    float YawRotation = FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X));
                    FRotator NewRotation(0.0f, YawRotation, 0.0f); // Pitch=0, Yaw=계산된 각도, Roll=0
                    SetActorRotation(NewRotation);
                }
            }
            else
            {
                // 반경을 벗어나려고 하면 새로운 목표 설정
                SetNewTargetLocation();
            }
        }
    }
}

void ATarget::SetNewTargetLocation()
{
    // 초기 위치 주변의 랜덤한 위치를 목표로 설정
    float RandomAngle = RandomRange(0.0f, 2.0f * PI);
    float RandomRadius = RandomRange(20.0f, MovementRadius - 10.0f); // 약간 여유를 둠

    FVector RandomOffset;
    RandomOffset.X = FMath::Cos(RandomAngle) * RandomRadius;
    RandomOffset.Y = FMath::Sin(RandomAngle) * RandomRadius;
    RandomOffset.Z = 0.0f; // 높이는 변경하지 않음

    TargetLocation = InitialLocation + RandomOffset;
    bIsMovingToTarget = true;

    //UE_LOGFMT(ELogLevel::Display, "Penguin new target set: ({}, {}, {})", TargetLocation.X, TargetLocation.Y, TargetLocation.Z);
}

void ATarget::SetAnimationState(const FString& StateName)
{
    // 같은 애니메이션이면 변경하지 않음
    if (CurrentAnimationState == StateName)
    {
        return;
    }

    CurrentAnimationState = StateName;

    ULuaScriptAnimInstance* AnimInstance = Cast<ULuaScriptAnimInstance>(GetComponentByClass<USkeletalMeshComponent>()->GetAnimInstance());

    if (AnimInstance && AnimInstance->StateMachine && AnimInstance->StateMachine->LuaTable.valid())
    {
        // Lua 테이블의 state 직접 변경
        AnimInstance->StateMachine->LuaTable["state"] =*StateName;

    }
}


void ATarget::HandleIdleAnimation(float DeltaTime)
{
    IdleAnimationTimer += DeltaTime;

    // Idle 애니메이션 지속 시간이 끝나면
    if (IdleAnimationTimer >= IdleAnimationDuration)
    {
        // 다음 행동 결정: 50% 확률로 이동 또는 다른 Idle 애니메이션
        if (RandomBool())
        {
            // 새로운 목적지로 이동 시작
            SetNewTargetLocation();
        }
        else
        {
            // 다른 Idle 애니메이션 시작
            StartIdleAnimation();
        }
    }
}

void ATarget::StartIdleAnimation()
{
    // Idle 또는 Bray 중 랜덤 선택
    FString IdleAnimations[] = { "Idle", "Bray" };
    FString SelectedAnimation = IdleAnimations[RandomRange(0, 1)];

    SetAnimationState(SelectedAnimation);

    // 3-5초 지속 시간 설정
    IdleAnimationDuration = RandomRange(3.0f, 5.0f);
    IdleAnimationTimer = 0.0f;

}

void ATarget::HandleDeathByBall(AActor* Ball)
{
    bDead = true;
    AccTime = 0.0f;

    // 넉백 시작/목표 위치 계산
    FVector BallLocation = Ball->GetActorLocation();
    FVector CurrentLocation = GetActorLocation();
    FVector KnockbackDirection = (CurrentLocation - BallLocation).GetSafeNormal();

    KnockbackStartLocation = CurrentLocation;
    KnockbackTargetLocation = CurrentLocation + (KnockbackDirection * 50.0f); // 50.0f는 넉백 거리
    KnockbackElapsed = 0.0f;
    bIsKnockback = true;

    // 사망 애니메이션 (Sleep)
    SetAnimationState("Sleep");
    UE_LOG(ELogLevel::Warning, TEXT("================================================="));
    UE_LOG(ELogLevel::Warning, TEXT("================================================="));
    UE_LOG(ELogLevel::Warning, TEXT("Penguin killed by ball - Playing death animation"));
}

// 랜덤 함수 헬퍼 구현
bool ATarget::RandomBool()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 1);
    return dis(gen) == 1;
}

float ATarget::RandomRange(float Min, float Max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(Min, Max);
    return dis(gen);
}

int32 ATarget::RandomRange(int32 Min, int32 Max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int32> dis(Min, Max);
    return dis(gen);
}
