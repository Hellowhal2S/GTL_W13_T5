#include "NPC.h"
#include "Components/SphereTriggerComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Contents/AnimInstance/LuaScriptAnimInstance.h"
#include "Engine/Classes/Lua/LuaScriptComponent.h"
#include "Engine/AssetManager.h"

ANPC::ANPC()
{
}

void ANPC::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();

    // TriggerCompoent 생성 및 설정
    TriggerComponent = AddComponent<USphereTriggerComponent>(TEXT("TriggerComponent"));
    TriggerComponent->SetRadius(10.f); // 테스트용 Trigger 반경 설정
    TriggerComponent->bGenerateOverlapEvents = true;

    // SkeletalMeshComponent 생성 및 설정
    SkeletalMeshComponent = AddComponent<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
    SetRootComponent(SkeletalMeshComponent);
    SkeletalMeshComponent->SetSkeletalMeshAsset(UAssetManager::Get().GetSkeletalMesh("Contents/Human/Human"));

    // 애니메이션 인스턴스 생성 및 연결
    SkeletalMeshComponent->SetAnimClass(ULuaScriptAnimInstance::StaticClass());
    AnimInstance = Cast<ULuaScriptAnimInstance>(SkeletalMeshComponent->GetAnimInstance());

    SetActorScale(FVector(0.03f, 0.03f, 0.03f));

}

void ANPC::BeginPlay()
{
    Super::BeginPlay();

    OnActorBeginOverlap.AddLambda([this](AActor* OverlappedActor, AActor* OtherActor)
        {
            if (ULuaScriptComponent* LuaComp = GetComponentByClass<ULuaScriptComponent>())
            {
                LuaComp->ActivateFunction("OnOverlap", OtherActor);
            }
        });
}
