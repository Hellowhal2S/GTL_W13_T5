#include "NPC.h"
#include "Components/SphereTriggerComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Contents/AnimInstance/LuaScriptAnimInstance.h"
#include "Engine/Classes/Lua/LuaScriptComponent.h"
#include "Engine/AssetManager.h"

ANPC::ANPC()
{
}

UObject* ANPC::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewComponent->PostSpawnInitialize();
    return NewComponent;
}

void ANPC::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();

    // TriggerCompoent 생성 및 설정
    TriggerComponent = AddComponent<USphereTriggerComponent>(TEXT("SphereTriggerComponent"));
    TriggerComponent->SetRadius(10.f); // 테스트용 Trigger 반경 설정
    TriggerComponent->bGenerateOverlapEvents = true;

    // SkeletalMeshComponent 생성 및 설정aa
    SkeletalMeshComponent = AddComponent<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
    SetRootComponent(SkeletalMeshComponent);
    SkeletalMeshComponent->SetSkeletalMeshAsset(UAssetManager::Get().GetSkeletalMesh("Contents/SkeletalPenguin/SkeletalPenguin"));
    SkeletalMeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    SkeletalMeshComponent->SetAnimation(UAssetManager::Get().GetAnimation("Contents/Armature|Runaway"));

    // 애니메이션 인스턴스 생성 및 연결
    SkeletalMeshComponent->SetAnimClass(ULuaScriptAnimInstance::StaticClass());
    AnimInstance = Cast<ULuaScriptAnimInstance>(SkeletalMeshComponent->GetAnimInstance());

    SetActorScale(FVector(0.01f, 0.01f, 0.01f));
    SetActorLocation(FVector(0.0f, 0.0f, 5.0f));

    // SkeletalMeshComponent의 StateMachine 설정
    //SkeletalMeshComponent->StateMachineFileName = TEXT("LuaScripts/Animations/NPCStateMachine.lua");
    //SkeletalMeshComponent->ClearAnimScriptInstance();
    //SkeletalMeshComponent->InitAnim();
    //if (AnimInstance)
    //{
    //    AnimInstance->InitializeAnimation();
    //}

    // LuaScriptComponent 생성 및 Lua 파일 경로 지정
    ULuaScriptComponent* LuaComp = GetComponentByClass<ULuaScriptComponent>();
    if (LuaComp)
    {
        // Lua 파일 경로를 ScriptName에 지정 (상대경로 또는 프로젝트 내 경로)
        //LuaComp->SetScriptName(TEXT("LuaScripts/Animations/NPCStateMachine.lua"));
        //LuaComp->LoadScript();
    }

}

void ANPC::BeginPlay()
{
    Super::BeginPlay();

    TriggerComponent->OnComponentBeginOverlap.AddLambda(
        [this](UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp
            , int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
        {
            if (AnimInstance && AnimInstance->StateMachine && AnimInstance->StateMachine->LuaTable.valid())
            {
                sol::function OnOverlap = AnimInstance->StateMachine->LuaTable["OnOverlap"];
                if (OnOverlap.valid())
                    OnOverlap(AnimInstance->StateMachine->LuaTable, OtherActor);
            }
        });

    TriggerComponent->OnComponentEndOverlap.AddLambda(
        [this](UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp
            , int32 OtherBodyIndex)
        {
            if (AnimInstance && AnimInstance->StateMachine && AnimInstance->StateMachine->LuaTable.valid())
            {
                sol::function OnEndOverlap = AnimInstance->StateMachine->LuaTable["OnEndOverlap"];
                if (OnEndOverlap.valid())
                    OnEndOverlap(AnimInstance->StateMachine->LuaTable, OtherActor);
            }
        });
}
