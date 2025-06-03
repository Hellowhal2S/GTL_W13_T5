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


ATarget::ATarget()
{
    /*Target = AddComponent<UStaticMeshComponent>(TEXT("Target"));
    Target->SetStaticMesh(FObjManager::GetStaticMesh(L"Contents/Penguin/Penguin.obj"));
    Target->bSimulate = true;
    SetRootComponent(Target);*/    Target = AddComponent<USkeletalMeshComponent>(TEXT("Target"));
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

//UObject* ATarget::Duplicate(UObject* InOuter)
//{
//    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
//
//    NewActor->Target = AddComponent<USkeletalMeshComponent>(TEXT("Target"));
//    NewActor->Target->SetSkeletalMeshAsset(UAssetManager::Get().GetSkeletalMesh("Contents/SkeletalPenguin/SkeletalPenguin"));
//    NewActor->Target->bSimulate = false;
//    NewActor->Target->SetAnimClass(ULuaScriptAnimInstance::StaticClass());
//    NewActor->SetRootComponent(Target);
//
//    NewActor->SphereComponent = AddComponent<USphereTargetComponent>(TEXT("Collision"));
//    NewActor->SphereComponent->SetupAttachment(Target);
//    
//    return NewActor;
//}

void ATarget::BeginPlay()
{
    AActor::BeginPlay();

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

    SphereComponent->SetRadius(100.f);

    GetComponentByClass<USphereTargetComponent>()->OnComponentBeginOverlap.AddLambda(
        [this](UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OhterComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
        {
            ULuaScriptAnimInstance* AnimInstance = Cast<ULuaScriptAnimInstance>(Target->GetAnimInstance());

            if (Cast<ASnowBall>(OtherActor) != nullptr)
            {
                UE_LOG(ELogLevel::Error, TEXT("Penguin Killed"));
            } 

            if (AnimInstance && AnimInstance->StateMachine && AnimInstance->StateMachine->LuaTable.valid())
            {
                sol::function OnOverlap = AnimInstance->StateMachine->LuaTable["OnOverlap"];
                if (OnOverlap.valid())
                    OnOverlap(AnimInstance->StateMachine->LuaTable, OtherActor);
            }
        }

    );

    GetComponentByClass<USphereTargetComponent>()->OnComponentEndOverlap.AddLambda(
        [this](UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OhterComponent, int32 OtherBodyIndex)
        {
            ULuaScriptAnimInstance* AnimInstance = Cast<ULuaScriptAnimInstance>(Target->GetAnimInstance());

            if (AnimInstance && AnimInstance->StateMachine && AnimInstance->StateMachine->LuaTable.valid())
            {
                sol::function OnOverlap = AnimInstance->StateMachine->LuaTable["OnEndOverlap"];
                if (OnOverlap.valid())
                    OnOverlap(AnimInstance->StateMachine->LuaTable, OtherActor);
            }
        }

    );
   
}

void ATarget::Tick(float DeltaTime)
{
    AActor::Tick(DeltaTime);

    if (bDead)
    {
        AccTime += DeltaTime;
        if (AccTime >2.0f)
        {
            //UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine);
            //GetComponentByClass<USkeletalMeshComponent>()->bSimulate = false;
            //EditorEngine->PhysicsManager->DestroyGameObject(GetComponentByClass<USkeletalMeshComponent>()->BodyInstance->BIGameObject);
            //SetActorLocation(FVector(-200,-200,-200));
            //// GetComponentByClass<UStaticMeshComponent>()->CreatePhysXGameObject();
            //if (GEngine->ActiveWorld->GetGameMode())
            //    GEngine->ActiveWorld->GetGameMode()->Score+= 100;
            //bDead = false;

        }
    }
}
